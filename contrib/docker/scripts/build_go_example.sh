#! /bin/bash

# Triggers a libral build if necessary and then builds the simple example go client:
#  - examples/go/listproviders.go
# This assumes that all necessary tools, including leatherman, have been set up already
# in the libral-build container.
#
# Usage: This is currently expected to run within a libral-build-go container, please
# refer to HACKING.md for information on building the Go example

set -ex

source ~/.bashrc

export PATH=${PATH}:/opt/pl-build-tools/bin/
export LIBRAL_SRC=/usr/src/libral
export GOPATH=/go
export LIBRAL_GO_PARENT_DIR=${GOPATH}/src/github.com/puppetlabs
export PROJECT_BINARY_DIR=${LIBRAL_SRC}/build/go-example
export GO_EXAMPLE_DIR=${LIBRAL_GO_PARENT_DIR}/libral/examples/go

# Build libral
if [ ! -f /opt/puppetlabs/puppet/lib/libral.a ]; then
    echo "Building libral"
    /usr/src/scripts/build.sh
fi

# Add libral repo to GOPATH
mkdir -p ${LIBRAL_GO_PARENT_DIR}
ln -s ${LIBRAL_SRC} ${LIBRAL_GO_PARENT_DIR}/libral

for p in /etc/profile.d/puppet-agent-tools.sh /etc/profile.d/puppet-agent.sh
do
    [ -f $p ] && source $p
done

mkdir -p ${PROJECT_BINARY_DIR}

go build -o ${PROJECT_BINARY_DIR}/listproviders.bin ${GO_EXAMPLE_DIR}/listproviders.go

# Pack example binary - reduces the size of the binary and adds
# necessary augeas and libral artifacts
/usr/src/scripts/pack_go_example.sh ${PROJECT_BINARY_DIR}