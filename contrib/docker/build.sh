#! /bin/bash

# Do the actual libral build. This assumes that all necessary tools,
# including leatherman, have been set up already

topdir=$PWD
set -ex

git clone https://github.com/puppetlabs/libral
mkdir libral/build
cd libral/build
cmake ..
make all install
