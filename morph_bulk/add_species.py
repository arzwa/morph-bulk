#!/usr/bin/python3.5
"""
Arthur Zwaenepoel - 2016

Part of morph_bulk_run project
"""

import ruamel.yaml as yaml
import os
import glob


class MorphConfig(object):
    """
    Morph config class, carries all information on directories etc.
    The directory structure enforced in this version for automated
    addition of species to MORPH is (e.g.):
    base_path/
        ./datasets
            ./dataset1
            ./dataset2
        ./clusterings
            ./click
                ./dataset1.click.clustering
                ./dataset2.click.clustering
            ./ppi
                ./dataset1.ppi.clustering
                ./dataset2.ppi.clustering
        ./gene_descriptions
            ./gene_descriptions.tsv
        README
    """
    def __init__(self, base_path, species, gene_sets, output_dir, morph_path, cache_path, gene_sets_type):
        self.base_path = base_path
        self.species = species
        self.gene_sets = gene_sets
        self.annotation = gene_sets
        self.morph = morph_path
        self.output_dir = output_dir
        self.bulk_out = os.path.join(output_dir, gene_sets_type)
        self.cache = cache_path
        self.gene_sets_type = gene_sets_type
        self.config_file = None
        self.jobs = None
        self.data_sets = os.path.join(self.base_path, 'datasets')


def check_directory(morph_config):
    """
    Check if directory has correct structure.
    """
    for i in ['clusterings', 'datasets']:
        if not os.path.isdir(os.path.join(morph_config.base_path, i)):
            raise NotADirectoryError("{} not found".format(i))

    if not os.path.exists('./gene_descriptions.tsv'):
        raise FileNotFoundError('gene_descriptions.tsv not found!')


def format_groups(morph_config, plaza=False):
    """
    Format a plain annotation file to a grouped one (gene set)
    Input: PLAZA style csv
    """
    # TODO: use maybe pandas groupby?
    gene_sets_file = morph_config.gene_sets
    gene_sets_dict = {}

    with open(gene_sets_file, 'r') as g:
        for line in g:
            if plaza:
                line = line.strip().split(";")
                if morph_config.gene_sets_type == 'go':
                    gene = line[2][1:-1]
                    gs = line[3][1:-1]

                else:
                    gene = line[1][1:-1]
                    gs = line[2][1:-1]

            else:
                line = line.strip().split("\t")
                gene = line[0]
                gs = line[1]

            if gs not in gene_sets_dict.keys():
                gene_sets_dict[gs] = {gene}

            else:
                gene_sets_dict[gs].add(gene)

    gene_sets_file_out = os.path.join(morph_config.base_path, "gene_sets",
                                      "gene_sets_{}.tsv".format(morph_config.gene_sets_type))

    if 'gene_sets' not in os.listdir(morph_config.base_path):
        os.system('mkdir {0}/{1}'.format(morph_config.base_path, 'gene_sets'))

    with open(gene_sets_file_out, 'w') as o:
        for key in gene_sets_dict.keys():
            o.write(key + "\t" + ",".join(list(gene_sets_dict[key])) + "\n")

    morph_config.gene_sets = gene_sets_file_out
    return morph_config


def write_config(morph_config):
    """
    Writes out config.yaml file for morph
    """
    expression_matrices = []

    for exp_mat in glob.glob(os.path.join(morph_config.base_path, 'datasets','*')):
        exp_mat_name = exp_mat.split("/")[-1]
        clusterings = []

        for clustering_type in glob.glob(os.path.join(morph_config.base_path, 'clusterings', '*')):
            for clustering in glob.glob(os.path.join(clustering_type, '*')):
                clustering_name = clustering.split("/")[-1]
                if len(clustering_name.split('.')) == 3:

                    # expression matrix specific clustering e.g. click
                    if clustering_name.split('.')[0] == exp_mat_name.split('.')[0]:
                        name = clustering_name.split('.')[-2]
                        clusterings.append({"name": name,
                                            "path": os.path.join('clusterings', name, os.path.basename(clustering))})

                else:
                    # expression matrix indepedent clusterings e.g. is_enzyme
                    name = clustering_name.split('.')[-2]
                    clusterings.append({"name": name,
                                        "path": os.path.join('clusterings', name, os.path.basename(clustering))})

        expression_matrices.append({"name": exp_mat_name,
                                    "path": os.path.join("datasets", exp_mat_name),
                                    "clusterings": clusterings})

    species_info = [{"name": morph_config.species,
                     "data_path": os.path.basename(morph_config.base_path),
                     "example_goi": "none specified",
                     "gene_descriptions": "gene_descriptions.tsv",
                     "gene_web_page": "http://www.arabidopsis.org/servlets/TairObject?name=$name&type=locus",
                     "gene_pattern": ".+",
                     "expression_matrices": expression_matrices}]

    config = {"cache_path": morph_config.cache,
              "species": species_info,
              "species_data_path": os.path.join(morph_config.base_path, "..")}

    with open(os.path.join(morph_config.base_path, "morph_config.yaml"), 'w') as o:
        o.write(yaml.dump(config, default_flow_style=False, default_style="'"))

    return os.path.join(morph_config.base_path, "morph_config.yaml")


def write_jobs(morph_config):
    """
    Writes out job files from groups (pathways/GO terms)
    """
    jobs_path = os.path.join(morph_config.base_path, "gene_sets", morph_config.gene_sets_type)

    if not os.path.isdir(os.path.join(morph_config.base_path, "gene_sets")):
        print('\tmkdir ' + os.path.join(morph_config.base_path, "gene_sets"))
        os.system('mkdir ' + os.path.join(morph_config.base_path, "gene_sets"))

    if not os.path.isdir(jobs_path):
        print('\tmkdir '+ jobs_path)
        os.system('mkdir '+ jobs_path)

    gene_sets = morph_config.gene_sets

    with open(gene_sets, 'r') as gss:
        for line in gss:
            line = line.strip().split("\t")
            gs = line[0]
            genes = line[1].split(",")
            genes = [gene.replace(" ", "") for gene in genes]  # remove whitespace
            genes = list(set(genes))  # remove duplicates

            with open(jobs_path + "/" + gs, 'w') as o:
                o.write(" ".join(genes))

    return jobs_path
