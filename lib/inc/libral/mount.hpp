#pragma once

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

    class mount_resource : public resource {
    public:
      mount_resource(std::shared_ptr<mount_provider>& prov, const std::string& name, const aug::node& base)
        : resource(name), _prov(prov), _base(base) { extract_base(); }

      std::unique_ptr<result<changes>> update(const attr_map& should);
    private:
      // Copy properties from _base into _values
      void extract_base();
      // Copy properties from _values into _base
      void update_base();
      void extract(const std::string &attr,
                   const boost::optional<std::string>& from,
                   const std::string& deflt);
      void extract(const std::string &attr,
                   const boost::optional<std::string>& from);

      void update_fstab(const attr_map& should, changes &changes);
      void remove_from_fstab();
      void unmount(const std::string& state);
      void mount(const std::string& state);

      std::shared_ptr<mount_provider> _prov;
      aug::node                       _base;
    };

    mount_provider(const std::string& data_dir)
      : aug(nullptr), _data_dir(data_dir), _seq(1) { };

    const std::string& description();
    result<bool> suitable();
    void flush();
    std::vector<std::unique_ptr<resource>> instances();
    std::unique_ptr<resource> create(const std::string& name);
  private:
    std::shared_ptr<aug::handle>  aug;
    std::string                  _cmd_mount;
    std::string                  _cmd_umount;
    std::string                  _data_dir;
    // We use this to create new paths in the augeas tree when creating entries
    int                          _seq;
  };

}
