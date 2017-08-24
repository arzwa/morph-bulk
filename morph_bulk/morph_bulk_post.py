#!/usr/bin/python3.5
"""
Arthur Zwaenepoel - 2017

Part of morph_bulk_run project
"""

# IMPORTS
import pandas as pd
import os
import glob
import re
import logging
import scipy.stats as ss


def summary(input_dir, output_dir, p_values, set_descriptions, gene_descriptions,
            go, supplementary, fdr_level, score_cut_off, full):
    """
    Generates various summary files from a MORPH bulk run
    :param input_dir: MORPH bulk results
    :param output_dir: output files directory
    :param p_values: data to estimate p-values provided (see random_baits.py)
    :param set_descriptions: gene set descriptions (tab delimited)
    :param go: boolean flag, use GO data from godb?
    :param supplementary: boolean flag, generate supplementary tables with TFs, kinases, etc.?
    :param fdr_level: level to control the FDR
    :param score_cut_off: z-score cut off for inclusion of candidates
    :param full: boolean flag, full supplementary tables?
    :return: summary.csv and supplementary tables if desired
    """
    # make output directory
    if not os.path.isdir(output_dir):
        os.mkdir(output_dir)
    else:
        logging.warning('Directory {} already exists, files may be overwritten!'.format(output_dir))

    columns = ['group', 'AUSR', 'genes_in_data', 'genes_missing', 'candidates']
    if p_values is not None:
        logging.info("Found p-values")
        p_values = pd.read_csv(p_values, index_col=0)
        p_values['size'] = [int(p) for p in p_values['size']]
        columns.append('p-value')

    desc_dict = {}

    if set_descriptions:
        desc_dict = get_descriptions(set_descriptions)

    if go:
        go_df, desc_dict = get_go_info()

    if gene_descriptions:
        gene_descs = get_descriptions(gene_descriptions)
    else:
        gene_descs = None

    results_summary = {}
    extended_annot = {}
    i = 0
    for morph_file in glob.glob(os.path.join(input_dir, '*')):

        # basic summary
        morph_parsed, candidates = parse_morph_report(morph_file, gene_descs)
        p_value = get_p_value(morph_parsed['genes_in_data'], morph_parsed['ausr'], p_values)
        morph_parsed['p-value'] = p_value

        if not go and set_descriptions:
            morph_parsed['set description'] = desc_dict[morph_parsed['group']]

        results_summary[morph_parsed['group']] = morph_parsed

        # extended annotation
        if p_value < fdr_level:

            for candidate in candidates.keys():
                if not score_cut_off or float(candidates[candidate]['score']) > score_cut_off:
                    extended_annot[i] = candidates[candidate]
                    extended_annot[i]['gene_set'] = morph_parsed['group']
                    extended_annot[i]['ausr'] = morph_parsed['ausr']
                    extended_annot[i]['p-value'] = morph_parsed['p-value']
                    extended_annot[i]['genes_in_data'] = morph_parsed['genes_in_data']

                    if morph_parsed['group'] in desc_dict.keys():
                        extended_annot[i]['set_description'] = desc_dict[morph_parsed['group']]

                    elif not go:
                        logging.warning("No set description found for {0}".format(morph_parsed['group']))

                    i += 1

    summary_df = pd.DataFrame.from_dict(results_summary, orient='index')
    extended_annot_df = pd.DataFrame.from_dict(extended_annot, orient='index')

    # FDR correction
    logging.info('Applying FDR correction at gene set level (Benjamini & Hochberg method, FDR-level = {})'.format(
        fdr_level))
    summary_df = benjamini_hochberg(summary_df, alpha=fdr_level)

    significant_sets = []
    for gene_set in summary_df[summary_df['BH-corrected']<fdr_level].index:
        logging.info("Evaluating {}".format(gene_set))
        candidates_df = extended_annot_df[extended_annot_df['gene_set'] == gene_set]
        significant_sets.append(candidates_df)

    if significant_sets:
        corrected_extended = pd.concat(significant_sets)

    else:
        logging.warning('No significant sets found: EXIT')
        corrected_extended = pd.DataFrame()

    # supplementary data tables
    if supplementary:
        regexps = compile_regexps()
        supplementary_dfs = get_supplementary(corrected_extended.to_dict(orient='index'), regexps)

    if go:
        summary_df = pd.concat([summary_df, go_df], axis=1)
        summary_df = summary_df[summary_df['p-value'] <= 1]

    # Generate csv's
    summary_df = summary_df.sort_values(['p-value'], ascending=True)
    summary_df.to_csv(os.path.join(output_dir, 'summary.csv'))
    corrected_extended.to_csv(os.path.join(output_dir, 'extended_annotation.csv'))

    # Print some data to screen
    logging.info("~"*80)
    logging.info("{:^80}".format("Total number of gene sets: {}".format(len(list(summary_df.index)))))
    logging.info("{:^80}".format("Number of significant scoring gene sets (FDR controlled): {}".format(len(
        list(summary_df[summary_df['BH-corrected'] < fdr_level].index)))))
    logging.info("{:^80}".format("Number of genes with new annotation: {}".format(len(
        set(list(corrected_extended['gene']))))))
    logging.info("{:^80}".format("Number of annotations: {}".format(len(
        set(list(corrected_extended.index))))))
    logging.info("~"*80)

    if supplementary:
        for sup in supplementary_dfs.keys():
            if full:
                supplementary_dfs[sup].to_csv(os.path.join(output_dir, sup + '.csv'))
            elif len(supplementary_dfs[sup])>0:
                if 'set_description' not in supplementary_dfs[sup].columns:
                    supplementary_dfs[sup]['set_description'] = ['']*len(supplementary_dfs[sup])
                df = pd.DataFrame({
                    'genes': supplementary_dfs[sup].groupby([
                        'set_description', 'ausr', 'p-value', 'genes_in_data'
                    ])['gene'].apply(list).apply(" ".join)}).reset_index()
                df.to_csv(os.path.join(output_dir, sup + '.csv'))


def parse_morph_report(morph_file, gene_descriptions=None):
    """
    Parses one morph output file (for one bait-gene set)
    :param morph_file: input file (one morph report file)
    :return: information from morph report in python dict
    """
    with open(morph_file, 'r') as f:
        content = f.read().split("\n\n")
        header = content[0].split("\n")
        body = content[1].split("\n")[:-1]
        group = morph_file.split("__")[-1][:-4]
        group = group.replace(':', '_')
        ausr = float(header[0].split(": ")[1])
        genes_in_data = [gene.upper() for gene in header[4].split(": ")[1].split(" ")[:-1]]
        genes_missing = []

        if len(header) > 5:
            genes_missing = [gene.upper() for gene in header[5].split(": ")[1].split(" ")[:-1]]

        # get candidates and there ranks and scores
        candidates = {}

        for line in body[2:]:
            rank, gene, score, desc = line.split("\t")[0:4]
            gene = gene.upper()

            if gene_descriptions is not None:
                if gene in gene_descriptions.keys():
                    desc = gene_descriptions[gene]

            candidates[gene] = {'gene': gene, 'rank': rank, 'score': score, 'desc': desc}

    return {'group': group, 'ausr': ausr, 'genes_in_data': len(genes_in_data),
            'genes_missing': len(genes_missing)}, candidates


def get_supplementary(results_dict, regexps):
    """
    Compile supplementary tables
    :param results_dict: MORPH results dictionary
    :param regexps: regular expressions for description matching
    :return: supplementary tables in dictionary
    """
    results = {'unknown': [], 'tf': [], 'signal': [], 'transporter': []}
    for key in results_dict:
        entry = results_dict[key]
        match = regex_matcher(regexps, entry['desc'])
        if match != -1:
            results[match].append(entry)

    for key in results.keys():
        results[key] = {k: results[key][k] for k in range(len(results[key]))}
        results[key] = pd.DataFrame.from_dict(results[key], orient='index')

    return results


def compile_regexps():
    """
    Compile regular expression for description matching (supplementary tables) once.
    :return: compiled regular expressions
    """
    regexps = {'unknown': [re.compile('unknown', re.IGNORECASE),
                           re.compile('n/a', re.IGNORECASE),
                           re.compile('hypothetical', re.IGNORECASE)],
               'tf': [re.compile('transcription factor', re.IGNORECASE),
                      re.compile('transcriptional activator', re.IGNORECASE),
                      re.compile('transcriptional coactivator', re.IGNORECASE),
                      re.compile('transcriptional effector', re.IGNORECASE),
                      re.compile('transcriptional regulator', re.IGNORECASE)],
               'signal': [re.compile('kinase', re.IGNORECASE),
                          re.compile('receptor', re.IGNORECASE),
                          re.compile('calmodulin', re.IGNORECASE),
                          re.compile('phosphatase', re.IGNORECASE),
                          re.compile('ubiquitin', re.IGNORECASE)],
               'transporter': [re.compile('transporter', re.IGNORECASE),
                               re.compile('exchanger', re.IGNORECASE),
                               re.compile('antiporter', re.IGNORECASE),
                               re.compile('symporter', re.IGNORECASE)]
               }
    return regexps


def regex_matcher(regexps, description):
    """
    Match regular expressions with gene description.
    :param regexps: compiled regular expressions
    :param description: gene description string
    :return: type of gene or -1 if nothing
    """
    for key in regexps.keys():
        for pattern in regexps[key]:
            if pattern.search(description) is not None:
                return key
    return -1


def get_go_info():
    """
    Get GO information from godb
    :return: go descriptions data frame and dictionary
    """
    import godb
    go = godb.get_annotations()
    go.index = go['GO id']
    go.index = [x.replace(':', '_') for x in go.index]
    desc_dict = {g: go.ix[g]['Term'] for g in go.index}
    return go, desc_dict


def get_descriptions(set_descriptions):
    """
    Get gene descriptions from a tab delimited file
    :param set_descriptions: tab delimited file
    :return: dictionary with gene to description mapping
    """
    desc_dict = {}
    with open(set_descriptions, 'r') as f:
        for line in f:
            id, desc = line.strip().split("\t")
            desc_dict[id] = desc
    return desc_dict


def get_p_value(n, ausr, p_values):
    """
    Get the p-value for a particular number of genes with a particular AUSR.
    Note that when the gene set size exceeds the maximum size in the random run results, a pessimistic estimate
    is made using the maximum gene set size in the random run.
    :param n: number of genes
    :param ausr: AUSR value
    :param p_values: data frame with results from random MORPH run
    :return: p-value: P(AUSR | n random genes)
    """
    if int(n) > max(p_values['size']):
        n = max(p_values['size'])

    l = sorted(p_values[p_values['size'] == int(n)]['AUSR'])
    p_value = 0
    for i in range(len(l)):
        if float(l[i]) > float(ausr):
            p_value = len(l[i:]) / len(l)
            break

    return p_value


def benjamini_hochberg(df, alpha=0.05, column='p-value'):
    m = len(df.index)
    df = df.sort_values(column, ascending=True)
    for i in range(m):
        if ((i +1)/ m) * alpha < df.ix[df.index[i]][column]:
            df['BH-corrected'] = (m/(i+1)) * df[column]
            return df


def holm(df, alpha, column='score'):
    m = len(df.index)
    df['p-score'] = ss.norm.sf([float(x) for x in list(df[column])])
    df = df.sort_values('p-score', ascending=True)
    for i in range(m):
        if alpha/(m+1-i) < df.ix[df.index[i]]['p-score']:
            logging.info('holm: {}'.format(i))
            return df[:i-1]
    return df
