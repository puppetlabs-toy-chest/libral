# Building libral

This file describes how to build `libral` from source. You should follow
the first set of instructions as much as possible as the instructions for
building a static executable are much more hairy.

## Building on Fedora

These instructions work on Fedora 24 and 25. They should also work on other
Linux distros if you adjust for differences in package names.

### Install needed packages

```bash
    dnf -y install gcc-c++ cmake make gettext boost-devel \
      curl-devel yaml-cpp-devel augeas-devel
```

### Build Leatherman

```bash
    git clone https://github.com/puppetlabs/leatherman.git
    cd leatherman
    git checkout 0.10.1
    mkdir build && cd build
    cmake ..
    make
    sudo make install
```

### Build libral

```bash
    git clone https://github.com/puppetlabs/libral
    cd libral
    mkdir build && cd build
    cmake ..
    make
```

You can now run `ralsh` with

```bash
    export RALSH_DATA_DIR=$(realpath ../data)
    ./bin/ralsh
    ./bin/ralsh user
    ./bin/ralsh user root
```

## Building a static executable on CentOS6

It is currently not possible to build a static executable on CentOS/Fedora,
as those distros do not ship some key static libraries such as
`augeas-static`, `libicu-static`, and `yamlcpp-static`. The best way to
build a static executable is therefore to use the
[`pl-build-tools`](https://github.com/puppetlabs/pl-build-tools-vanagon)
toolchain, which sadly is only available internally at Puppet (or you can
build your own from
[this repo](https://github.com/puppetlabs/pl-build-tools-vanagon)).

Assuming you have that toolchain, you need to do the following:

### Install needed packages

```bash
      rpm -ivh http://yum.puppetlabs.com/puppetlabs-release-pc1-el-6.noarch.rpm
      rpm -ivh http://$INTERNAL_HOST_FOR_PL_BUILD_TOOLS/yum/el/6/x86_64/pl-build-tools-release-22.0.3-1.el6.noarch.rpm
      yum -y install puppet-agent pl-boost pl-cmake pl-yaml-cpp pl-gcc
      yum -y install zlib-static libselinux-static
```

### Build Leatherman

```bash
    # This alias will be used a couple of times
    alias pl-cmake='/opt/pl-build-tools/bin/cmake \
           -DCMAKE_BUILD_TYPE=Debug \
           -DCMAKE_TOOLCHAIN_FILE=/opt/pl-build-tools/pl-build-toolchain.cmake \
           -DCMAKE_PREFIX_PATH=/opt/pl-build-tools \
           -DCMAKE_INSTALL_PREFIX=/opt/puppetlabs/puppet'
    git clone https://github.com/puppetlabs/leatherman.git
    cd leatherman
    git checkout 0.10.1
    mkdir build && cd build
    pl-cmake -DBOOST_STATIC=ON ..
    make
    sudo make install
```

### Build libral

```bash
    git clone https://github.com/puppetlabs/libral
    cd libral
    mkdir build && cd build
    PKG_CONFIG_PATH=/opt/puppetlabs/puppet/lib/pkgconfig \
      pl-cmake -DLIBRAL_STATIC=ON ..
    make
    # Put everything into one place so it can be easily copied
    # into VM's or containers
    ../examples/dpack
```
