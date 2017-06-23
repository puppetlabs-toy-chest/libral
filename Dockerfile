FROM ubuntu:latest
MAINTAINER Gareth Rushgrove "gareth@puppet.com"

ARG LIBRAL_DOWNLOAD_LOCATION="http://download.augeas.net/libral/"
ARG LIBRAL_FILENAME="ralsh-latest.tgz"

RUN apt-get update && \
    apt-get install --no-install-recommends -y wget && \
    wget "${LIBRAL_DOWNLOAD_LOCATION}${LIBRAL_FILENAME}" && \
    tar zxvf "$LIBRAL_FILENAME" -C /opt && \
    apt-get remove --purge -y wget && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    rm "$LIBRAL_FILENAME"

ENV PATH=/opt/ral/bin:$PATH

ENTRYPOINT ["/opt/ral/bin/ralsh"]
