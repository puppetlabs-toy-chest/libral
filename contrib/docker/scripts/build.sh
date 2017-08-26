#! /bin/bash

# Do the actual libral build. This assumes that all necessary tools,
# including leatherman, have been set up already

set -ex

cd /usr/src

source build_config.sh

for p in /etc/profile.d/puppet-agent-tools.sh /etc/profile.d/puppet-agent.sh
do
    [ -f $p ] && source $p
done

if [ ! -d libral ]; then
    echo "Cloning libral"
    git clone https://github.com/puppetlabs/libral
else
    # When we build in containers in Travis, we use the checkout that
    # Travis made on the host, and do not check anything out ourselves
    echo "Building from existing libral directory"
fi

cd libral
echo "Building commit $(git rev-parse HEAD)"
src_dir=$(pwd)

cd /var/tmp
rm -rf build
mkdir build

cd build
$CMAKE -DLIBRAL_STATIC=$STATIC $src_dir

make all test install

./bin/libral_test

if [ "$STATIC" = "ON" ]
then
    /usr/src/libral/contrib/docker/scripts/check-static-libs.rb
fi
