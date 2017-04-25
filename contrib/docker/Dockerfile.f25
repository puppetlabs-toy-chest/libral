#
# Container-based build environment
#
# Usage examples:
# Build image:      docker build -t libral-build examples
# Run bash cmdline: docker run --rm -ti -v $(pwd):/usr/src/libral libral-build
#   Build libral:   (mkdir -p build; cd build; cmake ..; make all install)
#   Run ralsh:      ./build/bin/ralsh <args>
# Or run pre-built: docker run --rm /usr/local/bin/ralsh <args>
#
# Additional details and build instructions in HACKING.md
#

FROM fedora:25

# Install build tools and libraries
RUN dnf update -y && \
    dnf install -y git gcc-c++ cmake make gettext boost-devel \
      curl-devel yaml-cpp-devel augeas-devel ruby rake bison  \
      rubygem-bundler rubygem-yard rubygem-rake-compiler      \
      ruby-devel redhat-rpm-config

# Build Leatherman
RUN git -C /usr/src clone https://github.com/puppetlabs/leatherman && \
    mkdir -p /usr/src/leatherman/build && \
    cd /usr/src/leatherman/build && \
    git checkout -q 1.0.0 && \
    cmake .. && \
    make all install

# Build Libral
RUN git -C /usr/src clone https://github.com/puppetlabs/libral && \
    mkdir -p /usr/src/libral/build && \
    cd /usr/src/libral/build && \
    cmake .. && \
    make all install

WORKDIR /usr/src/libral

CMD ["bash"]
