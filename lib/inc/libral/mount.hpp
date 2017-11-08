#pragma once

#include <libral/command.hpp>
#include <libral/ral.hpp>
#include <libral/provider.hpp>
#include <libral/augeas.hpp>

namespace libral {
  /* A provider here consists of two separate classes, since we can't do
     all the metaprogramming magic that Ruby does. The two classes are

     - a subclass of provider, which is responsible for dealing with all
       resources of a certain kind (what would be class methods in a Ruby
       provider)

     - a subclass of resource, which represents an instance of whatever
       resources the corresponding provider manages
  */

  /* Mount type:
     name        => # (namevar) The mount path for the mount.
     ensure      => * { defined == present, unmounted, absent, mounted }
     atboot      => * bool
     blockdevice => * Solaris only
     device      => * The device providing the mount.
     dump        => * /(0|1)/
     fstype      => * (required) The mount type.
     options     => * A single string containing options
     pass        => * The pass in which the mount is checked
     provider    => # { parsed }
     remounts    => # bool; Whether the mount can be remounted
     target      => * The file in which to store the mount table
  */

  class mount_provider : public provider {
  public:
    mount_provider() : _aug(nullptr) { };

    result<std::vector<resource>>
    get(context &ctx, const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

  protected:
    result<prov::spec> describe(environment& env) override;

  private:
    augeas::node base(const update &upd);
    result<resource> make(const std::string& name,
                          const augeas::node& base, const std::string& ens);
    result<void> update_base(const update &upd);
    result<void> set(context &ctx, const update &upd);
    result<void> update_fstab(const update& upd, changes& changes);
    result<void> remove_from_fstab(const update &upd);
    result<void> unmount(const std::string& name, const std::string& state);
    result<void> mount(const std::string& name, const std::string& state);
    result<void> flush();

    std::shared_ptr<augeas::handle> _aug;
    command::uptr                   _cmd_mount;
    command::uptr                   _cmd_umount;
  };

}
