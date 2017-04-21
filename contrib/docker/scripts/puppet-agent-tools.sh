# Add /opt/puppetlabs/puppet/bin to the path so we use puppet-agent's ruby

if ! echo $PATH | grep -q /opt/puppetlabs/puppet/bin ; then
    export PATH=$PATH:/opt/puppetlabs/puppet/bin
fi
