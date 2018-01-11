# MORPH bulk run command line interface

<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [MORPH bulk run command line interface](#morph-bulk-run-command-line-interface)
	- [1. Install morph-bulk using Singularity (recommended)](#1-install-morph-bulk-using-singularity-recommended)
		- [1.1 Install Singularity](#11-install-singularity)
		- [1.2 Build the morph-bulk image](#12-build-the-morph-bulk-image)
	- [2. Install morph-bulk the classical way](#2-install-morph-bulk-the-classical-way)
	- [3. Usage](#3-usage)

<!-- /TOC -->

Arthur Zwaenepoel 2017 (arzwa@psb.vib-ugent.be)

This Command line interface (CLI) bundles several command line utilities to
perform bulk runs with MORPH v1.0.6 (C++ version), which is included in this
repository (refer to the <a href='https://gitlab.psb.ugent.be/arzwa/morph-bulk/wikis/home'>wiki
</a> for further installation instructions). The CLI is written using click in
Python3.5 and runs on GNU/Linux based systems (tested on Ubuntu 16.04).

## 1. Install morph-bulk using Singularity (recommended)

### 1.1 Install Singularity
Singularity uses a container concept similar to Docker. Singularity
containers can be used to package entire scientific workflows, software
and libraries, and even data. Install Singularity following the
instructions here:

- Linux: http://singularity.lbl.gov/install-linux
- Mac: http://singularity.lbl.gov/install-mac
- Windows: http://singularity.lbl.gov/install-windows

If you're on Mac or Windows, do not forget to actually activate the virtual machine:

```
vagrant init singularityware/singularity-2.4
vagrant up
vagrant ssh
```

### 1.2 Build the morph-bulk image

From within th virtual machine, clone the repository and enter it:

    git clone https://gitlab.psb.ugent.be/arzwa/morph-bulk.git
    cd morph-bulk

If you have singularity installed, you can create the `morph-bulk` image
using the following command (*assuming you are inside the cloned repository*):

    sudo singularity build morph-bulk.simg morph-bulk.shub

This will create the `morph-bulk.simg` image file. Now you can use
`morph-bulk` like you would do normally but bear in mind that instead of
calling the `morph-bulk` executable directly, you now have to run

    singularity exec morph-bulk.simg morph-bulk <command>

You can test you're installation by running an analysis on the example
directory inside the morph-bulk repository.

    singularity exec morph-bulk.simg morph-bulk pipeline -r1 5 -r2 10 -nt 100 ./example/ zosma ./example/zosma.go.tsv ./pipeline_out/

If this runs smoothly, everything is installed correctly.

## 2. Install morph-bulk the classical way

To install `cd` to the cloned repository and run:

    $ pip install .

Different command line utilities are available, to see them run

    $ morph-bulk

Which should, if installation went correctly, render the following
response:

    Usage: morph-bulk [OPTIONS] COMMAND [ARGS]...

      Welcome to the MORPH bulk run command line interface!

                                  _           _           _ _
                                 | |         | |         | | |
       _ __ ___   ___  _ __ _ __ | |__ ______| |__  _   _| | | __
      | '_ ` _ \ / _ \| '__| '_ \| '_ \______| '_ \| | | | | |/ /
      | | | | | | (_) | |  | |_) | | | |     | |_) | |_| | |   <
      |_| |_| |_|\___/|_|  | .__/|_| |_|     |_.__/ \__,_|_|_|\_\
                           | |
                           |_|



    Options:
      --verbose [silent|info|debug]  Verbosity level, default = info.
      --help                         Show this message and exit.

    Commands:
      add       Add a species to MORPH.
      pipeline  MORPH bulk run pipeline.
      post      Post-process MORPH bulk results.
      random    Random MORPH bulk run.
      rdf       Generate RDF graph (for MorphDB).
      run       Run MORPH in bulk (chunked).


The available commands are all steps in the MORPH bulk analysis, however
also a full pipeline can be run using the ``pipeline`` command.
You can adjust the logging vebosity for any command with the `--verbose`
option.

    $ morph-bulk --verbose [silent|info|debug] [COMMAND]

## 3. Usage
Please refer to the <a href='https://gitlab.psb.ugent.be/arzwa/morph-bulk/wikis/home'>wiki</a>
for detailed install instructions, data format requirements and a tutorial
on how to run MORPH bulk runs.
