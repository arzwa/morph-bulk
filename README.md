# MORPH bulk run command line interface
Arthur Zwaenepoel 2016 - 2017
(arzwa@psb.vib-ugent.be)

This Command line interface (CLI) bundles several command line utilities
to perform bulk runs with MORPH v1.0.6 (C++ version). The CLI is
written using click in Python3.5 and runs on UNIX based systems. Non
standard Python packages used are the usual Anaconda packages (pandas,
numpy, etc.). For parsing and writing `.yaml` files, the ruamel package
is used.

To install `cd` to the cloned repository and run:

    $ pip install .

Different command line utilities are available, to see them run

    $ morphbulk

Which should, if installation went correctly, render the following
response:

    Usage: morphbulk [OPTIONS] COMMAND [ARGS]...

      Welcome to the MORPH bulk run command line interface!

                                   _     _           _ _
                                  | |   | |         | | |
        _ __ ___   ___  _ __ _ __ | |__ | |__  _   _| | | __
       | '_ ` _ \ / _ \| '__| '_ \| '_ \| '_ \| | | | | |/ /
       | | | | | | (_) | |  | |_) | | | | |_) | |_| | |   <
       |_| |_| |_|\___/|_|  | .__/|_| |_|_.__/ \__,_|_|_|\_\
                            | |
                            |_|


    Options:
      --verbose [silent|info|debug]  Verbosity level, default = info.
      --help                         Show this message and exit.

    Commands:
      add       Add a species to Morph.
      morph     Run MORPH in bulk (chunked).
      pipeline  MORPH bulk run pipeline.
      post      Post-process MORPH bulk results.
      pre       Preprocess expression matrices.
      random    Random MORPH bulk run.
      rdf       Generate RDF graph (for MorphDB).


The available commands are all steps in the MORPH bulk analysis, however
also a full pipeline can be run using the ``pipeline`` command.
You can adjust the logging vebosity for any command with the `--verbose`
option.

    $ morphbulk --verbose [silent|info|debug] [COMMAND]


### Input data:

It is important that your data is structured as follows:
```
.
├── clusterings
│   ├── click
│   │   ├── caros_all.click.clustering
│   │   ├── caros_hairy_roots.click.clustering
│   │   ├── caros_organs.click.clustering
│   │   ├── caros_smartcell.click.clustering
│   │   └── caros_suspension_culture.click.clustering
│   ├── IsDpOrtholog
│   │   ├── caros_all.IsDpOrtholog.clustering
│   │   ├── caros_hairy_roots.IsDpOrtholog.clustering
│   │   ├── caros_organs.IsDpOrtholog.clustering
│   │   ├── caros_smartcell.IsDpOrtholog.clustering
│   │   └── caros_suspension_culture.IsDpOrtholog.clustering
│   ├── IsEnzymeKEGG
│   │   ├── caros_all.IsEnzymeKEGG.clustering
│   │   ├── caros_hairy_roots.IsEnzymeKEGG.clustering
│   │   ├── caros_organs.IsEnzymeKEGG.clustering
│   │   ├── caros_smartcell.IsEnzymeKEGG.clustering
│   │   └── caros_suspension_culture.IsEnzymeKEGG.clustering
│   └── IsOrtholog_final
│       ├── caros_all.IsOrtholog_final.clustering
│       ├── caros_hairy_roots.IsOrtholog_final.clustering
│       ├── caros_organs.IsOrtholog_final.clustering
│       ├── caros_smartcell.IsOrtholog_final.clustering
│       └── caros_suspension_culture.IsOrtholog_final.clustering
├── datasets
│   ├── caros_all.expression_matrix
│   ├── caros_hairy_roots.expression_matrix
│   ├── caros_organs.expression_matrix
│   ├── caros_smartcell.expression_matrix
│   └── caros_suspension_culture.expression_matrix
└── gene_descriptions.tsv
```

Note that each clustering type should have it's own directory (here e.g.
click) and that the names of the clusterings should correspond to the
respective data set it is a clustering of, with as filename
`<dataset>.<clustering type>.<clustering>`.

The data sets should be structured as plain matrices with header:
```
gene	Heinz_bud	Heinz_flower	Heinz_leaf
Solyc02g085950	12710.51	10259.24	122316.7
Solyc10g075130	22209.16	46884	9.78
Solyc04g071610	4880.03	2966.38	310.43
```

And the clusterings should be just a tab delimited file that looks as
follows:
```
Solyc02g085950	1
Solyc10g075130	2
Solyc04g071610	1
```
structured as `<gene> TAB <cluster_ID>`. The cluster ID can be any
literal.

Gene descriptions should be provided as a tab delimited file 
`gene_descriptions.tsv`, _e.g_:

```
Caros024876.1   50S ribosomal protein L27, partial
Caros024876.2   No hits found
Caros028421.1   unnamed product
Caros005129.1   TPS1
Caros007111.1   No hits found
Caros005129.2   probable alpha,alpha-trehalose-phosphate synthase, TPS1
Caros007111.2   ATP/ADP carrier protein, ADP,ATP carrier protein 3, mitochondrial-like
Caros023877.1   unnamed product
Caros005129.3   unnamed product
```

### Running as a pipeline

    $ morphbulk pipeline
Command line utility that uses combines all steps (except the RDF graph
generation). Runs a MORPH bulk run for the above outlined input data.
Run `morphbulk pipeline --help` for usage instructions.

Basically this will perform the following steps:

1. Add species to MORPH (generate config files _etc._)
2. Perform random MORPH bulk runs (for empirical _p_-value estimation.
3. Perform MORPH in bulk on a given functional annotation
4. Summarize results

Optionally, an RDF graph can than be constructed from the output, which 
allows easy addition to MorphDB or personal exploration in an RDF database
server (_e.g._ Apache Jena Fuseki).

### Adding a species to MORPH

    $ morphbulk add
Script for adding a new species to MORPH (makes configuration and
job list files) Usage: open a terminal and run: `morphbulk add
--help` and follow the instructions there.


### Running MORPH in bulk

    $ morphbulk morph
Python command line utility for running MORPH bulk runs in chunks. Type
`morphbulk morph --help` at the command line for usage instructions.


### Running MORPH in bulk on random gene sets for p-value estimation

    $ morphbulk random
Python command line utility for preparing and running MORPH random bulk
runs for p-value estimation. Type `morphbulk random --help` at the
command line for usage instructions.


### Post processing MORPH bulk run

    $ morphbulk post
Python command line utility for post processing MORPH bulk run results.
Enables generation of a summary file, extended annotation and
supplementary tables highlighting special genes. Type
`morphbulk post --help` at the command line for usage instructions.


### Pre processing data sets

    $ morphbulk pre
script for expression data set pre-processing (filtering and
normalization). Uses `TMMnormalization.R` for normalization of RNA-Seq
data sets. Type `morphbulk pre --help` at the command line for
usage instructions.


### Making an RDF (turtle) graph from a bulk run

    $ morphbulk rdf
script for making an RDF graph (`turtle` format) Type `morphbulk rdf
 --help` at the command line for usage instructions.
To add a `.ttl` file to a database, it is best to rebuild the entire
database. To this end run: `<path to apache jena>/bin/tdbloader2 --loc
<path to database> <.ttl files>`




