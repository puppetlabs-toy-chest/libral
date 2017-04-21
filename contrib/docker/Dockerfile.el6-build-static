#
# Tools image for libral builds. Contains everything besides libral itself
#
FROM centos:6

COPY scripts /usr/src/scripts
RUN /usr/src/scripts/image.sh el6-build-static

WORKDIR /usr/src

CMD ["/usr/src/scripts/build.sh"]
