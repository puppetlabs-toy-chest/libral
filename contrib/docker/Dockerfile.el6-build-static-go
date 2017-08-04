#
# Tools image for libral go example client builds.
# Contains everything besides libral itself
#
# # Usage examples:
#
# ## Build image
# Requires that the libral-build image has been built.
#     docker build -t libral-build-go -f Dockerfile.el6-build-static-go .
#
# ## Build the go client example
# Running the container without any arguments will trigger a build of the client.
# If no local workspace is mounted then the build will clone the latest from master.
#
# Mount your local libral workspace, this will mount your local workspace within the
# container, build libral, the go example client (`examples/go/listproviders.go`)
# and packages it for use on other hosts.
#
#     docker run -ti -v ${PWD}:/usr/src/libral libral-build-go
#
# The resulting archive will be available in your local repo in the
# `build/go-example` directory.
#
# ## Run bash
# Run from within the root of your local libral workspace, this will mount your
# local workspace within the container, and give you a bash prompt from which you
# can manually build.
#
#     docker run -ti -v ${PWD}:/usr/src/libral libral-build-go bash
#
FROM libral/el6-build-static

ENV GOPATH=/go
ENV LIBRAL_EL6_BUILD_STATIC_GO=1

RUN /usr/src/scripts/install_go.sh 1.8.3

WORKDIR /usr/src

CMD ["/usr/src/scripts/build_go_example.sh"]
