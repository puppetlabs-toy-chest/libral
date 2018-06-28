from puppet/puppet-agent:latest

RUN apt-get update && \
    apt-get install --no-install-recommends -y wget && \ 
    wget http://download.augeas.net/libral/ralsh-latest.tgz && \
    tar zxvf ralsh-latest.tgz -C /usr/src && \
    apt-get remove --purge -y wget && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENV PATH=/usr/src/ral/bin:$PATH

COPY apt_libral /etc/puppetlabs/code/modules/apt_libral
COPY puppet.prov /usr/src/ral/data/providers

CMD ["resource", "apt_libral"] 
