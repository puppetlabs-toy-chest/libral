#! /bin/bash

# This script is used to set up the Docker image and is run from the Dockerfile
# It expects to be passed the type of build we are making.
# It also exects the following envvars to be set with versions of the
# dependencies to download/install.
#
# PL_BUILD_TOOLS_VER
# RPMFORGE_RELEASE_VER
# LEATHERMAN_VER
#
# The script will exit with an error if any of the above envvars are not set.

set -ex

die() {
    [ -n "$1" ] && echo "$1"
    echo "usage: $0 BUILD_TYPE"
    exit 1
}

[ $# -lt 1 ] && die

case "$1"
in
    f25-build)
        BUILD_TYPE=$1
        CMAKE=cmake
        STATIC=OFF
        ;;
    el6-build-static)
        BUILD_TYPE=$1
        CMAKE=pl-cmake
        STATIC=ON
        ;;
    *)
        die "unknown build type $1"
        ;;
esac

cd /usr/src

# Install tooling
if [ $BUILD_TYPE = f25-build ]
then
    dnf update -y
    dnf install -y git gcc-c++ cmake make gettext boost-devel curl-devel \
        yaml-cpp-devel augeas-devel ruby rake bison
    dnf clean all
elif [ $BUILD_TYPE = el6-build-static ]
then
    yum update -y
    rpm -ivh https://dl.fedoraproject.org/pub/epel/epel-release-latest-6.noarch.rpm
    rpm -ivh http://yum.puppetlabs.com/puppetlabs-release-pc1-el-6.noarch.rpm
    # This requires that we are on the internal VPN
    rpm -ivh http://pl-build-tools.delivery.puppetlabs.net/yum/el/6/x86_64/pl-build-tools-release-${PL_BUILD_TOOLS_VER:?"Need to set PL_BUILD_TOOLS_VER"}.el6.noarch.rpm
    rpm -ivh http://ftp.tu-chemnitz.de/pub/linux/dag/redhat/el6/en/x86_64/rpmforge/RPMS/rpmforge-release-${RPMFORGE_RELEASE_VER:?"Need to set RPMFORGE_RELEASE_VER"}.el6.rf.x86_64.rpm
    yum -y install git puppet-agent pl-boost pl-cmake pl-yaml-cpp pl-gcc \
        zlib-static libselinux-static readline-static ncurses-static \
        upx bison
    yum clean all

    install scripts/puppet-agent-tools.sh /etc/profile.d
    install -m a+x scripts/pl-cmake /usr/local/bin
else
    die "unknown build type $BUILD_TYPE"
fi

# Write out the build config that build.sh expects
cat > build_config.sh <<EOF
BUILD_TYPE=$BUILD_TYPE
CMAKE=$CMAKE
STATIC=$STATIC
EOF

# Build Leatherman
git clone https://github.com/puppetlabs/leatherman
mkdir -p leatherman/build
cd leatherman/build
git checkout -q ${LEATHERMAN_VER:?"Need to set LEATHERMAN_VER"}
$CMAKE -DBOOST_STATIC=$STATIC ..
make all install
make clean
