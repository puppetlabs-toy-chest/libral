Puppet::Type.newtype(:apt_libral) do
  @doc = 'A type representing an Apt package from libral'
  newparam(:name, namevar: true)
  newproperty(:platform)
  newproperty(:ensure)
end
