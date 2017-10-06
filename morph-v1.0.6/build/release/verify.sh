#!/bin/sh
make
./morph
pushd output
files=`ls`
popd
for f in $files
do
    echo $f
    diff output/$f expected_output/$f
done
