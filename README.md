# MORPH bulk run command line interface
Arthur Zwaenepoel 2016 - 2017
(arzwa@psb.vib-ugent.be)

This Command line interface (CLI) bundles several command line utilities to
perform bulk runs with MORPH v1.0.6 (C++ version), which is included in this
repository (refer to the <ahref='https://github.com/arzwa/morphbulk/wiki'>wiki
</a> for further installation instructions. The CLI is written using click in
Python3.5 and runs on UNIX based systems.

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
      bulk      Run MORPH in bulk (chunked).
      pipeline  MORPH bulk run pipeline.
      post      Post-process MORPH bulk results.
      random    Random MORPH bulk run.
      rdf       Generate RDF graph (for MorphDB).


The available commands are all steps in the MORPH bulk analysis, however
also a full pipeline can be run using the ``pipeline`` command.
You can adjust the logging vebosity for any command with the `--verbose`
option.

    $ morphbulk --verbose [silent|info|debug] [COMMAND]

Please refer to the <a href='https://github.com/arzwa/morphbulk/wiki'>wiki</a> 
for detailed install instructions, data format requirements and a tutorial 
on how to run MORPH bulk runs.

