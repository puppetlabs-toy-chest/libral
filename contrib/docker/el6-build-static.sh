#! /bin/bash

set -ex

# Install tooling
yum update -y
rpm -ivh https://dl.fedoraproject.org/pub/epel/epel-release-latest-6.noarch.rpm
rpm -ivh http://yum.puppetlabs.com/puppetlabs-release-pc1-el-6.noarch.rpm
rpm -ivh http://pl-build-tools.delivery.puppetlabs.net/yum/el/6/x86_64/pl-build-tools-release-22.0.3-1.el6.noarch.rpm
rpm -ivh http://ftp.tu-chemnitz.de/pub/linux/dag/redhat/el6/en/x86_64/rpmforge/RPMS/rpmforge-release-0.5.3-1.el6.rf.x86_64.rpm
yum -y install git puppet-agent pl-boost pl-cmake pl-yaml-cpp pl-gcc \
    zlib-static libselinux-static readline-static ncurses-static \
    upx bison
yum clean all

cd /usr/src

cat > /etc/profile.d/puppet-agent-tools.sh <<'EOF'
# Add /opt/puppetlabs/puppet/bin to the path so we use puppet-agent's ruby

if ! echo $PATH | grep -q /opt/puppetlabs/puppet/bin ; then
  export PATH=$PATH:/opt/puppetlabs/puppet/bin
fi
EOF

# Lots of special stuff to mkae cmake work and produce a static build
cat > /usr/local/bin/pl-cmake <<'EOF'
#! /bin/bash
export PKG_CONFIG_PATH=/opt/puppetlabs/puppet/lib/pkgconfig
/opt/pl-build-tools/bin/cmake \
   -DCMAKE_BUILD_TYPE=Debug \
   -DCMAKE_TOOLCHAIN_FILE=/opt/pl-build-tools/pl-build-toolchain.cmake \
   -DCMAKE_PREFIX_PATH=/opt/pl-build-tools \
   -DCMAKE_INSTALL_PREFIX=/opt/puppetlabs/puppet "$@"
EOF
chmod a+x /usr/local/bin/pl-cmake

# Almost the same as the standard build.sh for Fedora, except
#   - use pl-cmake
#   - pass LIBRAL_STATIC
#   - we run cmake twice, since the first run fails because of some
#     weird cmake/pkg-config iinteraction
cat > /usr/src/build.sh <<'EOF'
#! /bin/bash

# Do the actual libral build. This assumes that all necessary tools,
# including leatherman, have been set up already

topdir=$PWD
set -ex

export PATH=${PATH}:/opt/puppetlabs/puppet/bin

git clone https://github.com/puppetlabs/libral
mkdir libral/build
cd libral/build
pl-cmake -DLIBRAL_STATIC=ON .. || :
pl-cmake -DLIBRAL_STATIC=ON ..
make all install
EOF
chmod a+x /usr/src/build.sh

# Build Leatherman
git clone https://github.com/puppetlabs/leatherman
mkdir -p leatherman/build
cd leatherman/build
git checkout -q 0.10.1
pl-cmake -DBOOST_STATIC=ON ..
make all install
make clean
