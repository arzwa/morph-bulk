#!/usr/bin/python3.5
"""
Arthur Zwaenepoel - 2016

Part of morph_bulk_run project
"""

# IMPORTS
import click
import uuid
import ruamel.yaml as yaml
import os
import logging


def morph_chunked_run(morph_config_file, jobs_dir, output_dir, chunk_size, number_of_candidates, morph_path):
    """
    Chunked MORPH bulk run wrapper.
    :param morph_config_file: MORPH configuration file
    :param jobs_dir: directory to store jobs
    :param output_dir: output directory
    :param chunk_size: number of jobs to run at once
    :param number_of_candidates: number of candidate genes to include in MORPH output
    :param morph_path: path to MORPH executable (tested with MORPH C++ version 1.0.6)
    :return: executed MORPH bulk run
    """
    # Check directories and files
    if not os.path.exists(jobs_dir):
        logging.error("Directory {} not found!".format(jobs_dir))
    if not os.path.exists(output_dir):
        logging.info("Creating output directory")
        os.system('mkdir {}'.format(output_dir))
    if not os.path.exists(morph_config_file):
        logging.error("Config file {} not found!".format(morph_config_file))

    # Read in the config file
    with open(morph_config_file,'r') as m:
        morph_config = m.read()
    morph_config = yaml.load(morph_config, yaml.RoundTripLoader)



    # Get jobs
    jobs = os.listdir(jobs_dir)
    count = 0
    i = 0
    c = int(chunk_size)

    while i < len(jobs):
        if i + c >= len(jobs):
            end = len(jobs) - 1
        else:
            end = i + c
        count += 1
        logging.info("Started chunk {}".format(count))
        jobs_chunk = jobs[i:end]
        job_list_name = str(uuid.uuid1())
        make_tmp_job_list(jobs_chunk, jobs_dir, morph_config, job_list_name)

        # Run MORPH
        run_morph(morph_path, morph_config_file, job_list_name, output_dir, number_of_candidates)
        i += c

    logging.info("Started last chunk")
    job_list_name = str(uuid.uuid1())
    make_tmp_job_list(jobs[i:], jobs_dir, morph_config, job_list_name)
    run_morph(morph_path, morph_config_file, job_list_name, output_dir, number_of_candidates)


def make_tmp_job_list(jobs, jobs_dir, config, job_list_name):
    """
    Make a temporary job list
    :param jobs: jobs to include in job list
    :param jobs_dir: directory where jobs are stored
    :param config: MORPH configuration file
    :param job_list_name: unique job list ID
    :return: job list yaml file
    """
    sp = config['species'][0]['name']
    sets = []

    # Perform random gene sampling from data sets
    for j in jobs:
        sets.append({"name": j, "path": os.path.join(os.path.abspath(jobs_dir), j)})

    # Write the job list
    job_info = [{"species_name": sp, "data_path": sp, "genes_of_interest": sets}]
    job_list_dict = {"data_path": config['species_data_path'], "jobs": job_info}
    open(job_list_name, 'w').write(yaml.dump(job_list_dict, default_flow_style=False, default_style="'"))


def run_morph(morph_path, morph_config, job_list, output_dir, number):
    """
    Runs MORPH CLI (v1.0.6)
    :param morph_path: path to MORPH executable (tested with MORPH C++ version 1.0.6)
    :param morph_config: MORPH configuration file
    :param job_list: job list file
    :param output_dir: output directory for MORPH results
    :return: MORPH results in output_dir
    """
    os.system("{0} {1} {2} {3} {4}".format(morph_path, morph_config, job_list, output_dir, number))
    os.remove(job_list)
