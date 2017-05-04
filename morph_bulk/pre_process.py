#!/usr/bin/python3.5
"""
Arthur Zwaenepoel - 2016

Part of morph_bulk_run project
"""
# TODO: Normalization currently requires R script, should be ported to Python.

import pandas as pd
import os
import click


def normalize_matrix(matrix_file, normalization_script):
    """
    Normalize expression matrix using external R script
    """
    os.system('Rscript ' + normalization_script + " " + matrix_file + " " +
              matrix_file + ".normalized")


def filter_matrix(matrix_file, p):
    """
    Filter an expression matrix by standard deviation
    Choose standard deviation as such that at least 70% of dataset survives filter
    """
    matrix = pd.read_table(matrix_file, sep = "\t", index_col=0)
    for sd in range(10,1,-1):
        matrix_copy = matrix
        matrix_copy = matrix_copy[matrix_copy.std(axis = 1) > sd]
        if len(matrix_copy.index) > (float(p) * len(matrix.index)):
            print("Standard deviation used for filtering: {}".format(sd))
            return matrix_copy


def cluster_matrix(cluster_script, matrix_file):
    """
    Cluster (CLICK) expression matrix and move to clustering directory.
    Requires Tim's cluster.sh script
    """
    os.system(cluster_script + " " + matrix_file)
    os.system('mv *.clustering *.anl ' './clusterings/click/')
