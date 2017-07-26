#!/bin/bash

# This script is used to install Go and is run from the Dockerfile
# It expects to be passed the version of Go to install.

set -ex

die() {
    [ -n "$1" ] && echo "$1"
    echo "usage: $0 GO_VERSION"
    exit 1
}

[ $# -lt 1 ] && die

# Download and Install Go
curl -O https://storage.googleapis.com/golang/go${1}.linux-amd64.tar.gz
tar -C /usr/local -xzf go${1}.linux-amd64.tar.gz
rm -rf ./go${1}.linux-amd64.tar.gz

# Update .bashrc
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
echo 'export GOPATH=/go' >> ~/.bashrc