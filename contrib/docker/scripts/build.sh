#! /bin/bash

# Do the actual libral build. This assumes that all necessary tools,
# including leatherman, have been set up already

topdir=$(readlink --canonicalize $(dirname $0)/..)

set -ex

cd $topdir

source build_config.sh

for p in /etc/profile.d/puppet-agent-tools.sh /etc/profile.d/puppet-agent.sh
do
    [ -f $p ] && source $p
done

git clone https://github.com/puppetlabs/libral
mkdir libral/build
cd libral/build

$CMAKE -DLIBRAL_STATIC=$STATIC ..

make all test install

./bin/libral_test

if [ "$STATIC" = "ON" ]
then
    $topdir/scripts/check-static-libs.rb
fi
