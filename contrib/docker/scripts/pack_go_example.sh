#! /bin/bash

# Pack go binding example `listproviders` and supporting files so they can be
# copied onto a new Linux host.

if [ $# -lt 1 ]; then
    echo "usage: $0 PROJECT_BINARY_DIR"
    exit 1
fi

cd ${PROJECT_BINARY_DIR}

srcdir=/go/src/github.com/puppetlabs/libral
githash=$(cd $srcdir && git describe --always)

# Assemble stuff in libralgo-example/
rm -rf libralgo-example
mkdir -p libralgo-example/bin
cp ./listproviders.bin libralgo-example/bin/

cat > libralgo-example/README <<EOF
This is a statically linked build of https://github.com/puppetlabs/libral/examples/go/.

It was made from commit $githash of that repo on a $(cat /etc/system-release)
machine. The executable should work on any $(uname -m) Linux machine
that has $(rpm -q glibc) or later.
EOF

cp -pr /go/src/github.com/puppetlabs/libral/data libralgo-example

#
# Snarf up some augeas lenses that we need
#
augp=$(command -v augparse)
if [ -z "$augp" ]
then
    if [ -x /opt/puppetlabs/puppet/bin/augparse ]; then
        export PATH=$PATH:/opt/puppetlabs/puppet/bin
    else
        echo "could not find augparse anywhere"
        exit 1
    fi
fi

# Terrible hack to help us find builtin lenses we use without any
# modification
cat > libralgo-example/data/lenses/dep.aug <<EOF
module Dep =
let l1 = Authorized_keys.lns
let l2 = Hosts.lns
EOF
# Have augparse tell us all the lenses we need
for lns in libralgo-example/data/lenses/*.aug
do
    cp -np $(augparse --notypecheck --trace $lns | cut -d ' ' -f 2 | grep ^/) libralgo-example/data/lenses
done
rm -rf libralgo-example/data/lenses/dep.aug libralgo-example/data/lenses/tests

find libralgo-example/ -name \*~ -delete

# Reduce the executable as best we can
strip -R .note -R .comment -s libralgo-example/bin/*
UPX=$(type -p upx)
if [ -n "$UPX" ]
then
    $UPX -qq libralgo-example/bin/*
fi

# Add a wrapper to make sure we run with the right setup
cat > libralgo-example/bin/listproviders <<'EOF'
#! /bin/bash
topdir=$(readlink --canonicalize $(dirname $0)/..)
export RALSH_DATA_DIR=$topdir/data
export RALSH_LIBEXEC_DIR=$topdir/bin
exec $topdir/bin/listproviders.bin "$@"
EOF
chmod a+x libralgo-example/bin/listproviders

tarball=listproviders-$(date +%Y-%m-%dT%H.%M)-$githash.tgz
tar czf $tarball libralgo-example
echo
echo Made $tarball
echo
