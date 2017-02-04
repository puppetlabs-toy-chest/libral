#pragma once

#include <memory>
#include <ctime>
#include <vector>

#include <libral/result.hpp>
#include <libral/provider.hpp>

namespace libral {
  /* File provider (POSIX)

     There are a few things that make file special and would break the
     provider/resource API:
       * using one resource to recurse into directories breaks the
         assumption that 'update' operates on a single resource and that
         therefore any changes that it performs are about that resource,
         not other resources
       * 'instances' is pointless and doesn't return anything; we should
         really have it return a result, with an error that indicates that
         listing is not supported. It would also be nice to have some
         notion of filters to pass to 'instances' so you could ask, for
         example, for all files in a directory

     attributes: (key: # not implemented, * property, : parameter)
     name                    => * string, namevar (path for the file)
     ensure                  => * enum(present, absent, file, directory, link)
     backup                  => # Whether (and how) file content should be backed...
     checksum                => : enum(md5, md5lite, sha256, sha256lite, mtime, ctime, none)
     checksum_value          => * string
     content                 => * string
     ctime                   => * string (timestamp)
     force                   => : bool
     group                   => * string|int (group name or id)
     ignore                  => # A parameter which omits action on files matching
     links                   => # How to handle links during file actions.  During
     mode                    => * string (really?)
     mtime                   => * string (timestamp)
     owner                   => * string|int (user name or id)
     provider                => # The specific backend to use for this `file...
     purge                   => # Whether unmanaged files should be purged. This...
     recurse                 => # Whether to recursively manage the _contents_ of...
     recurselimit            => # How far Puppet should descend into...
     replace                 => # Whether to replace a file or symlink that...
     selinux_ignore_defaults => # If this is set then Puppet will not ask SELinux...
     selrange                => # What the SELinux range component of the context...
     selrole                 => # What the SELinux role component of the context...
     seltype                 => # What the SELinux type component of the context...
     seluser                 => # What the SELinux user component of the context...
     show_diff               => # Whether to display differences when the file...
     source                  => # A source file, which will be copied into place...
     source_permissions      => # Whether (and how) Puppet should copy owner...
     sourceselect            => # Whether to copy all valid sources, or just the...
     target                  => * string (path)
     type                    => * enum(file, directory,link)
     validate_cmd            => # A command for validating the file's syntax...
     validate_replacement    => # The replacement string in a `validate_cmd` that...
   */
  class file_provider : public provider {
  public:

    using shared_provider_ptr = std::shared_ptr<file_provider>;

    class file_resource : public resource {
    public:
      file_resource(shared_provider_ptr& prov, const std::string& name)
        : resource(name), _prov(prov) { }

      result<changes> update(const attr_map& should) override;
    private:
      void update_metadata(result<changes>& res, const attr_map& should);
      void update_content(result<changes>& res, const attr_map& should);
      void update_target(result<changes>& res, std::string& state,
                         const std::string& target);
      void remove(result<changes>& res, std::string& state, bool force);
      void create_file(result<changes>& res);
      void create_directory(result<changes>& res);

      shared_provider_ptr _prov;
    };

    result<bool> suitable() { return true; };

    /* Always returns an empty vector for now, can't list all files this
       way. The API is missing a way to indicate an error from instances */
    result<std::vector<resource_uptr>> instances() override;
    result<boost::optional<resource_uptr>> find(const std::string &name) override;

    std::unique_ptr<resource> create(const std::string& name);
  protected:
    result<prov::spec> describe() override;
  private:
    void load(resource &res);
    void find_from_stat(resource &res);
    std::string time_as_iso_string(std::time_t *time);
  };

}
