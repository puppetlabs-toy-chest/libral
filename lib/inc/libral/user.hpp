#pragma once

#include <libral/provider.hpp>

#include <libral/command.hpp>

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
    const std::string& description();
    result<bool> suitable() override;

    result<std::vector<resource>>
    get(context &ctx, const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void>
    set(context &ctx, const updates& upds) override;

  protected:
    result<prov::spec> describe() override;
  private:
    result<void> set(context &ctx, const update& upd);

    boost::optional<command>     _cmd_useradd;
    boost::optional<command>     _cmd_usermod;
    boost::optional<command>     _cmd_userdel;
    std::string                  _data_dir;
  };

}
