#!/bin/sh
/bin/rm -rf output
mkdir output
. ../command.sh
gdb --args ${command[@]}
