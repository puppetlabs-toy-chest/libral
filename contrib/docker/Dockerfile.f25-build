#
# Tools image for libral builds. Contains everything besides libral itself
#
FROM fedora:25

COPY scripts /usr/src/scripts
RUN /usr/src/scripts/image.sh f25-build

WORKDIR /usr/src

CMD /bin/bash
