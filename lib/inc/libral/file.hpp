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
    result<std::vector<resource>>
    get(context &ctx,
        const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

  protected:
    result<prov::spec> describe(environment &env) override;
  private:
    result<void> set(context &ctx, const update& upd);

    result<void> update_metadata(context &ctx, const update& upd);
    result<void> update_content(context &ctx, const update& upd);
    result<void> update_target(context &ctx, const update& upd);

    result<void>
    remove(const std::string& path, const std::string& state, bool force);

    result<void> create_file(const std::string& path);
    result<void> create_directory(const std::string& path);

    void load(resource &res);
    void find_from_stat(resource &res);
    std::string time_as_iso_string(std::time_t *time);
  };

}
