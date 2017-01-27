#pragma once

#include <libral/provider.hpp>

namespace libral {
  /* User provider for posix systems

  attributes: (key: # not implemented, * property, : parameter)
  name                 => * string, namevar
  ensure               => * enum(present,absent,role)
  allowdupe            => # Whether to allow duplicate UIDs. Defaults to...
  attribute_membership => # Whether specified attribute value pairs should...
  attributes           => # Specify AIX attributes for the user in an array...
  auth_membership      => # Whether specified auths should be considered the
  auths                => # The auths the user has.  Multiple auths should...
  comment              => * string
  expiry               => # The expiry date for this user. Must be provided...
  forcelocal           => # Forces the management of local accounts when...
  gid                  => * string|number
  groups               => # array(string)
  home                 => * string
  ia_load_module       => # The name of the I&A module to use to manage this
  iterations           => # This is the number of iterations of a chained...
  key_membership       => # Whether specified key/value pairs should be...
  keys                 => # Specify user attributes in an array of key ...
  loginclass           => # The name of login class to which the user...
  managehome           => # bool
  membership           => # If `minimum` is specified, Puppet will ensure...
  password             => # string
  password_max_age     => # number
  password_min_age     => # number
  profile_membership   => # Whether specified roles should be treated as the
  profiles             => # The profiles the user has.  Multiple profiles...
  project              => # The name of the project associated with a user.
  provider             => # The specific backend to use for this `user...
  purge_ssh_keys       => # Whether to purge authorized SSH keys for this...
  role_membership      => # Whether specified roles should be considered the
  roles                => # The roles the user has.  Multiple roles should...
  salt                 => # This is the 32-byte salt used to generate the...
  shell                => * string
  system               => : bool
  uid                  => * integer
  */
  class user_provider : public provider {
  public:

    using provider_ptr = std::shared_ptr<user_provider>;

    class user_resource : public resource {
    public:
      user_resource(provider_ptr& prov, const std::string& name, bool exists)
        : resource(name), _prov(prov), _exists(exists) { }

      result<changes> update(const attr_map& should) override;
    private:
      provider_ptr   _prov;
      /* Indicate whether this resource already exists on the system or is
         brand new. We use this to determine whether to call useradd or
         usermod when we update the user */
      bool           _exists;
    };

    user_provider(const std::string& data_dir) : _data_dir(data_dir) { };

    const std::string& description();
    result<bool> suitable();
    void flush();
    std::vector<std::unique_ptr<resource>> instances();
    std::unique_ptr<resource> create(const std::string& name);
  protected:
    result<prov::spec> describe() override;
  private:
    std::string                  _cmd_useradd;
    std::string                  _cmd_usermod;
    std::string                  _cmd_userdel;
    std::string                  _data_dir;
  };

}
