#!/home/arthur/anaconda3/bin/python3.5
"""
Arthur Zwaenepoel - 2016

Part of morph_bulk_run project
"""

# IMPORTS
import pandas as pd
import os
import glob
import logging
from rdflib import Graph, Literal, RDF, URIRef, RDFS
from rdflib.namespace import Namespace
from .morph_bulk_post import get_go_info


def morph_to_rdf(input_dir, output_file, p_values, gene_descriptions, bait_descriptions, go,
                 gene_families, bait_type, species, p_val_cut_off, score_cut_off, max_candidates):
    # check if gene descriptions provided
    gene_description_dict = {}
    gd = False
    if gene_descriptions:
        gene_description_dict = gd_dict(gene_descriptions)
        gd = True

    # check if bait descriptions provided
    bait_description_dict = {}
    if bait_descriptions and not go:
        bait_description_dict = gd_dict(bait_descriptions)

    if go:
        go_df, bait_description_dict = get_go_info()

    # check if gene families provided
    gf_df = None
    if gene_families is not None:
        gf_df = pd.read_csv(gene_families, index_col=2, header=None, sep=';', quotechar="\"")

    # check if p-values provided
    if p_values is not None:
        p_values = pd.read_csv(p_values, index_col=0)
        p_values['size'] = [int(x) for x in p_values['size']]

    # initialize graph
    graph = Graph()

    # make ontology object
    onto = MorphOntology()

    # add RDFS predicates for MORPH predicates
    # graph = rdfs_for_morph(graph, onto) NOT IMPLEMENTED

    # get number of groups to process
    total = int(os.popen('ls ' + input_dir + ' | wc -l').read().strip())
    count = 0
    count2 = 0

    # fancy progress printing
    strlist = ["\---/", " \-/ ", "  X  ", " /-\ ", "/---\\"]
    logging.info("{0}  Start processing  MORPH output  {0}".format(strlist[-1]))

    # start iterating over files in inputdir
    for morph_file in glob.glob(os.path.join(input_dir, '*')):
        # print progress
        count += 1
        if count % 10 == 0:
            str1 = strlist[count2]
            logging.info("{0} processed {1:4d} of {2:4d} gene sets {0}".format(str1, count, total))
            count2 += 1
            count2 %= len(strlist)

        morph_parsed = parse_morph_report(morph_file, bait_description_dict)

        # add a group and all it's content to the graph
        graph = add_group_to_graph(graph, onto, morph_parsed, bait_type, species,
                                   p_values, p_val_cut_off, score_cut_off, max_candidates, gd)

    count2 -= 1

    # add gene family information
    if gf_df is not None:
        count2 +=1
        logging.info("{0}   Adding orthology information   {0}".format(strlist[(count2) % 5]))
        graph = add_orthology_to_graph(graph, onto, gf_df, species)

    # add gene descriptions
    if gene_descriptions or go:
        logging.info("{0}     Adding gene descriptions     {0}".format(strlist[(count2 + 1) % 5]))
        graph = add_descriptions_to_graph(graph, onto, gene_description_dict)

    # serialize graph
    logging.info("{0}        Serializing  graph        {0}".format(strlist[(count2 + 2) % 5]))
    graph.serialize("./" + output_file, format = 'turtle')


class MorphOntology():
    """
    A class representing the MorphDB ontology.
    Attempting to follow W3C standards as much as possible.
    """
    def __init__(self):
        self.namespace = Namespace("http://morph.org/")
        self.gene_set = "http://morph.org/gene_set#"
        self.gene_set_uri = URIRef("http://morph.org/gene_set")
        self.species_gene_set = "http://morph.org/gene_set_sp#"
        self.species_gene_set_uri = URIRef("http://morph.org/gene_set_sp")
        self.gene = "http://morph.org/gene#"
        self.gene_uri = URIRef("http://morph.org/gene")
        self.score = "http://morph.org/score#"
        self.score_uri = URIRef("http://morph.org/score#")
        self.gene_set_type = "http://morph.org/gene_set_type#"
        self.gene_set_type_uri = URIRef("http://morph.org/gene_set_type")
        self.gene_family = "http://morph.org/gene_family#"
        self.gene_family_uri = URIRef("http://morph.org/gene_family")


def rdfs_for_morph(graph, onto):
    """
    Add RDFS predicates for MORPH predicates
    NOT IMPLEMENTED
    """
    graph.add(onto.gene_set_type_uri, RDFS.domain, onto.gene_set_uri)
    graph.add(onto.gene_set_type_uri, RDFS.range, onto.gene_set_type)
    graph.add(onto.species_gene_set_uri, RDFS.subClassOf, onto.gene_set_uri)
    graph.add(onto.species_gene_set_uri, RDFS.domain, onto.gene_set_uri)
    graph.add(onto.species_gene_set_uri, RDFS.range, onto.gene_set_uri)
    pass


def add_group_to_graph(graph, onto, morph_parsed, bait_type, species, p_values,
                       p_val_cut_off, score_cut_off, max_candidates, gd):
    """
    Add a group (bait set) to the graph. Uses the ontology defined in class MorphOntology. Uses the RDF-schema.
    Include all candidates that score > score_cut_off and all bait sets with p < p_val_cut_off.
    :param graph: RDF graph
    :param onto: ontology object
    :param morph_parsed: parsed MORPH results file
    :param bait_type: type of bait set (e.g. go, mapman)
    :param species: species ID
    :param p_values: file with random MORPH bulk results
    :param p_val_cut_off: p-value cut off for set inclusion
    :param score_cut_off: z-score cut off for candidate gene inclusion
    :param max_candidates: maximum number of candidates to include
    :param gd: gene descriptions (optional, othrwise fetched fro MORPH results)
    :return: updated RDF graph
    """
    group, AUSR, description, genes_in_data, genes_missing, body, candidates = morph_parsed
    MORPH = onto.namespace
    group_uri = URIRef(onto.gene_set + group)
    group_type_uri = URIRef(onto.gene_set_type + bait_type)
    species_group_uri = URIRef(onto.species_gene_set  + species + "_" + group)

    if p_values is not None:
        p_val = get_p_value(len(genes_in_data), AUSR, p_values)
        if p_val > p_val_cut_off:
            return graph
        graph.add((species_group_uri, MORPH.has_p_value, Literal(p_val)))

    # add node for group
    graph.add((group_uri, RDF.type, onto.gene_set_uri))
    graph.add((group_uri, onto.gene_set_type_uri, group_type_uri))
    graph.add((group_uri, RDFS.label, Literal(description)))

    # add subentry for species
    graph.add((group_uri, MORPH.has_species, species_group_uri))
    graph.add((species_group_uri, RDF.type, onto.species_gene_set_uri))
    graph.add((species_group_uri, RDFS.label, Literal(description)))
    graph.add((species_group_uri, MORPH.has_ausr, Literal(AUSR)))
    graph.add((species_group_uri, MORPH.no_genes_in_dataset, Literal(len(genes_in_data))))
    graph.add((species_group_uri, MORPH.no_genes_missing, Literal(len(genes_missing))))

    # add genes and gene-group predicates
    for gene in genes_in_data:
        gene_uri = URIRef(onto.gene + gene)
        graph.add((gene_uri, RDF.type, onto.gene_uri))
        graph.add((species_group_uri, MORPH.has_bait_in_dataset, gene_uri))
        graph.add((gene_uri, MORPH.is_bait_of, species_group_uri))

    for gene in genes_missing:
        gene_uri = URIRef(onto.gene + gene)
        graph.add((gene_uri, RDF.type, onto.gene_uri))
        graph.add((species_group_uri, MORPH.has_bait_missing, gene_uri))
        graph.add((gene_uri, MORPH.is_missing_bait_of, species_group_uri))

    for gene in candidates.keys():
        rank = candidates[gene][0]
        score = candidates[gene][1]
        gene_description = candidates[gene][2]

        if int(rank) <= max_candidates and float(score) > score_cut_off:
            score_uri = URIRef(onto.score + gene + "_" + group)
            gene_uri = URIRef(onto.gene + gene)
            graph.add((gene_uri, RDF.type, onto.gene_uri))
            graph.add((species_group_uri, MORPH.has_candidate, gene_uri))
            graph.add((gene_uri, MORPH.is_candidate_of, species_group_uri))
            graph.add((gene_uri, MORPH.has_score, score_uri))
            graph.add((score_uri, MORPH.score_for_gene_set, group_uri))
            graph.add((score_uri, MORPH.score_for_sp_gene_set, species_group_uri))
            graph.add((score_uri, MORPH.score_value, Literal(float(score))))
            graph.add((score_uri, MORPH.has_rank, Literal(int(rank))))
            if not gd:
                graph.add((gene_uri, RDFS.label, Literal(gene_description)))

    return graph


def get_p_value(n, ausr, p_values):
    """
    Get the p-value for a particular number of genes with a particular AUSR
    :param n: number of genes
    :param ausr: AUSR value
    :param p_values: data frame with results from random MORPH run
    :return: p-value: P(AUSR | n random genes)
    """
    if n > max(list(p_values['size'])):
        n = max(list(p_values['size']))
    l = sorted(p_values[p_values['size'] == int(n)]['AUSR'])
    p_value = 0
    for i in range(len(l)):
        if float(l[i]) > float(ausr):
            p_value = len(l[i:]) / len(l)
            break
    return p_value


def add_orthology_to_graph(graph, onto, gene_families, sp):
    """
    Add genefamily information all in one loop over genefamily file.
    :param graph: RDF graph
    :param onto: ontology object
    :param gene_families: gene families data frame
    :param sp: species ID
    :return: update RDF graph
    """
    MORPH = onto.namespace
    for row in gene_families.itertuples():
        gene_uri = URIRef(onto.gene + row[0])
        gf_uri = URIRef(onto.gene_family + row[1])
        species = row[2]
        if sp == species:
            graph.add((gene_uri, RDF.type, onto.gene_uri))
            graph.add((gf_uri, RDF.type, onto.gene_family_uri))
            graph.add((gene_uri, MORPH.member_of, gf_uri))
            graph.add((gf_uri, MORPH.has_member, gene_uri))
            graph.add((gf_uri, MORPH.has_species_member, Literal(species)))
            graph.add((gene_uri, MORPH.species, Literal(species)))
    return graph


def add_descriptions_to_graph(graph, onto, gene_descriptions):
    """
    Add gene descriptions as RDFS.label objects. Note that genes with no description will have no RDFS.label "
    :param graph: RDF graph
    :param onto: ontology object
    :param gene_descriptions: gene descriptions dictionary
    :return: updated RDF graph
    """
    for key in gene_descriptions.keys():
        gene_uri = URIRef((onto.gene + key))
        graph.add((gene_uri, RDFS.label, Literal(gene_descriptions[key])))
    return graph


def parse_morph_report(morph_file, bait_desc):
    """
    Parses a Morph output file for a single bait group
    :param morph_file: MORPH results file
    :param bait_desc: description of gene set (e.g. GO, mapman)
    :return: MORPH results in list
    """
    with open(morph_file, 'r') as f:
        content = f.read().split("\n\n")
        header = content[0].split("\n")
        body = content[1].split("\n")[:-1]

        group = morph_file.split("__")[-1][:-4]
        group = group.replace(':', '_')
        AUSR = float(header[0].split(": ")[1])
        description = ''
        if group in bait_desc:
            description = bait_desc[group]
        genes_in_data = [gene.upper() for gene in header[4].split(": ")[1].split(" ")[:-1]]
        genes_missing = []
        if len(header) > 5:
            genes_missing = [gene.upper() for gene in header[5].split(": ")[1].split(" ")[:-1]]

        # get candidates and there ranks and scores
        candidates = {}
        for line in body[2:]:
            rank, gene, score, gene_description = line.split("\t")[0:4]
            gene = gene.upper()
            candidates[gene] = [rank, score, gene_description]

    return [group, AUSR, description, genes_in_data, genes_missing, body, candidates]


def gd_dict(gene_descriptions):
    """
    Get dictionary with gene descriptions from tab delimited file
    :param gene_descriptions: tab delimited file
    :return: dictionary with gene to description mapping
    """
    desc_dict = {}
    with open(gene_descriptions, 'r') as f:
        for line in f:
            line = line.strip().split("\t")
            if len(line) > 1:
                desc_dict[line[0].upper()] = line[1]
            else:
                desc_dict[line[0].upper()] = 'NA'
    return desc_dict
