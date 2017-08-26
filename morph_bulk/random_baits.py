#!/home/arthur/anaconda3/bin/python3.5
"""
Arthur Zwaenepoel - 2016

Part of morph_bulk_run project
"""
import pandas as pd
import random
import ruamel.yaml as yaml
import os
import uuid
import glob
import logging


def random_bulk_run(morph_config_file, data_sets_dir, output_file, morph_path, range_start,
                    range_end, number_total, chunk_size, background_set=None):
    """
    Perform MORPH on n random sets of genes in a defined size range [r1,r2]
    :param morph_config_file: MORPH configuration file
    :param data_sets_dir: path to data sets for MORPH
    :param output_file: output file name
    :param morph_path: path to MORPH executable (tested with MORPH C++ version 1.0.6)
    :param range_start: minimum gene set size (>= 5)
    :param range_end: maximum gene set size
    :param number_total: total number of gene sets to analyze
    :param chunk_size: chunk size for MORPH run (prevent aggressive memory usage)
    :return: CSV file with results from random bulk run
    """
    with open(morph_config_file,'r') as m:
        morph_config = m.read()
    morph_config = yaml.load(morph_config, yaml.RoundTripLoader)

    # Check directory
    dir = os.listdir('.')
    for i in ['output', 'sets']:
        if i in dir:
            raise AssertionError("{} already exists in working directory!".format(i))

    # Run random runs
    logging.info("Started random run")
    results_dict = {}

    if background_set:
        background_set = fetch_background_set(background_set)

    count = 0
    for i in range(int(range_start),int(range_end)):
        done = 0
        logging.info("Started {0} random runs for gene sets of size {1}".format(number_total, i))
        while done < number_total:
            job_list_name = str(uuid.uuid1())
            output_dir = job_list_name + '.output_dir'
            os.mkdir(output_dir)
            random_sets(data_sets_dir, i, chunk_size, morph_config, job_list_name, background_set)

            # Run MORPH
            os.system("{0} {1} {2} {3} {4}".format(morph_path, morph_config_file, job_list_name, output_dir, '1'))
            os.system('rm -r {}.sets'.format(job_list_name))

            # Get results
            results_dict, count = process_output(results_dict, output_dir, count)
            os.system('rm -r {}*'.format(job_list_name))

            done += chunk_size
        logging.info("Done with {0} random runs for gene sets of size {1}".format(number_total, i))

    logging.info("Making data frame")
    df = pd.DataFrame(index=list(range(count)), columns=['size', 'AUSR'])
    i = 0
    for size in results_dict.keys():
        for ausr in results_dict[size]:
            df.set_value(i, 'size', size)
            df.set_value(i, 'AUSR', ausr)
            i+=1
    df.to_csv(output_file)

    return output_file


def process_output(results_dict, output_dir, count):
    """
    Process MORPH output for random run
    :param results_dict: dictionary with MORPH results
    :param output_dir: directory with MORPH results
    :return: updated dictionary with MORPH results
    """
    for result in glob.glob(os.path.join(output_dir, '*')):
        with open(result, 'r') as f:
            count += 1

            for line in f:
                line = line.strip()
                if line.startswith('Best'):
                    ausr = float(line.split()[-1])

                elif line.startswith('Genes of interest present in data set'):
                    n_genes = len(line.split(': ')[-1].split())

            if n_genes in results_dict.keys():
                results_dict[n_genes].append(ausr)

            else:
                results_dict[n_genes] = [ausr]

    return results_dict, count


def random_sets(data_sets_dir, i, n, config, job_list_name, background_set=None):
    """
    Generate random gene sets and job_list file
    :param data_sets_dir: path to data sets for MORPH
    :param i: gene set size
    :param n: number of gene sets
    :param config: MORPH configuration file
    :param job_list_name: job list name (unique ID)
    :return: nothing (written file)
    """
    sets = []
    sp = config['species'][0]['name']
    sets_dir = job_list_name + '.sets'
    os.mkdir(sets_dir)
    data_sets = os.listdir(data_sets_dir)

    # Perform random gene sampling from data sets
    for j in range(int(n)+1):
        sample = pick_random_genes_from_random_data_set(data_sets, data_sets_dir, i, background_set)
        open("./{0}/{1}".format(sets_dir, str(j)), 'w').write(" ".join(sample))
        sets.append({"name":  str(j), "path": os.path.join(os.path.abspath('.'), sets_dir, str(j))})

    # Write the job lists
    job_info = [{"species_name": sp, "data_path": sp, "genes_of_interest": sets}]
    job_list_dict = {"data_path": config['species_data_path'], "jobs": job_info}
    open(job_list_name, 'w').write(yaml.dump(job_list_dict, default_flow_style=False, default_style="'"))


def pick_random_genes_from_random_data_set(data_sets, data_sets_dir, n, background_set=None):
    """
    Picks a random set of n genes (without replacement) from a random data_set
    :param data_sets: list of data sets
    :param n: number of genes to pick
    :return: list of genes
    """
    data_set = random.choice(data_sets)
    data = pd.read_csv(os.path.join(data_sets_dir, data_set), index_col=0, header=0, sep="\t")

    if background_set:
        sample_set = background_set & set(data.index)
    else:
        sample_set = data.index

    genes = random.sample(list(sample_set), n)
    return genes


def fetch_background_set(background_set):
    """
    Make a set from the input background set

    :param background_set: file with in first column genes
    :return: set with genes.
    """
    with open(background_set, 'r') as f:
        lines = f.readlines()
    lines = [line.split()[0].strip() for line in lines]

    return set(lines)
