#!/bin/sh

# Compiles morph on a system like vampire (i.e. with the module system)
module purge
module load gsl
module load cmake
module load boost
export BOOST_ROOT=/software/shared/apps/x86_64/boost/1.55  # work-around for incorrect BOOST_ROOT
module load yaml-cpp
module load gcc
./cmake_
make
