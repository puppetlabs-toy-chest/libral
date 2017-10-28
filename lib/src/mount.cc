#include <libral/mount.hpp>

#include <sstream>

#include <leatherman/locale/locale.hpp>

using namespace leatherman::locale;

namespace aug = libral::augeas;

namespace libral {

  result<prov::spec> mount_provider::describe(environment& env) {
    static const std::string desc =
#include "mount.yaml"
      ;

    auto aug = env.augeas({ { "Mount_Fstab.lns", "/etc/fstab" },
                            { "Mount_Fstab.lns", "/etc/mtab"  } });
    err_ret(aug);

    _aug = aug.ok();

    _cmd_mount = env.command("mount");
    _cmd_umount = env.command("umount");

    return env.parse_spec("mount", desc);
  }

  aug::node mount_provider::base(const update &upd) {
    if (upd.present()) {
      auto res =
        _aug->make_node("/files/etc/fstab/*[file = '" + upd.name() + "']");
      res.resolve();
      return res;
    } else {
      return _aug->make_node_seq_next("/files/etc/fstab");
    }
  }

  /**
   * Creates a new resource. The name will be NAME. Other attributes will
   * be filled in from the node BASE which must point to an fstab entry
   * in the augeas tree. The ensure attribute will be set to ENS
   */
  result<resource>
  mount_provider::make(const std::string& name, const aug::node& base,
                       const std::string& ens) {
    auto res = create(name);

    err_ret( base.extract(res, "device", "spec") );
    err_ret( base.extract(res, "fstype", "vfstype") );
    err_ret( base.extract(res, "options", "options", "defaults"));
    err_ret( base.extract(res, "dump", "dump", "0") );
    err_ret( base.extract(res, "pass", "passno", "0") );
    res["ensure"] = ens;
    // @todo lutter 2016-05-20: we actually need to pay attention to target
    res["target"] = "/etc/fstab";

    return res;
  }

  result<std::vector<resource>>
  mount_provider::get(context &ctx,
                      const std::vector<std::string>& names,
                      const resource::attributes& config) {
    std::map<std::string, resource> resources;

    auto nodes = _aug->match("/files/etc/fstab/*[label() != '#comment']");
    if (!nodes) return nodes.err();

    for(const auto& node : nodes.ok()) {
      auto name = node["file"];
      err_ret(name);

      if (name.ok()) {
        auto r = make(**name, node, "unmounted");
        err_ret(r);

        resources.emplace(**name, std::move(*r));
      } else {
        // Can't happen, the lens makes sure we always have a 'file' entry
        return error(_("Missing file/mountpoint"));
      }
    }

    nodes = _aug->match("/files/etc/mtab/*[label() != '#comment']");
    if (!nodes) return nodes.err();

    for(const auto& node : nodes.ok()) {
      auto name = node["file"];
      err_ret(name);

      if (name.ok()) {
        auto rsrc_iter = resources.find(*name.ok());
        if (rsrc_iter != resources.end()) {
          rsrc_iter->second["ensure"] = "mounted";
        } else {
          auto r = make(*name.ok(), node, "ghost");
          err_ret(r);

          resources.emplace(**name, std::move(*r));
        }
      } else {
        // Can't happen, the lens makes sure we always have a 'file' entry
        return error(_("Missing file/mountpoint"));
      }
    }

    std::vector<resource> result;
    for (auto pair = resources.begin(); pair != resources.end(); ++pair) {
      result.push_back(std::move(pair->second));
    }
    return std::move(result);
  }

  result<void>
  mount_provider::update_base(const update &upd) {
    for (auto& k : { "device", "fstype" }) {
      if (! upd[k]) {
        return error(_("The mount {1} is missing the '{2}' field",
                       upd.name(), k));
      }
    }

    auto bs = base(upd);

    err_ret( bs.erase() );
    err_ret( bs.set("spec", upd["device"]) );
    err_ret( bs.set("file", upd.name()) );
    err_ret( bs.set("vfstype", upd["fstype"]) );
    err_ret( bs.set("options", upd["options"], "defaults") );
    err_ret( bs.set("dump", upd["dump"], "0") );
    return bs.set("passno", upd["pass"], "0");
  }

  result<void>
  mount_provider::set(context &ctx,
                      const updates& upds) {
    for (auto upd : upds) {
      err_ret( set(ctx, upd) );
    }
    return flush();
  }

  result<void>
  mount_provider::set(context &ctx, const update &upd) {
    changes& changes = ctx.changes_for(upd.name());

    /* Possible values for ensure:

       Control what to do with this mount. Set this attribute to unmounted
       to make sure the filesystem is in the filesystem table but not
       mounted (if the filesystem is currently mounted, it will be
       unmounted). Set it to absent to unmount (if necessary) and remove
       the filesystem from the fstab. Set to mounted to add it to the fstab
       and mount it. Set to present to add to fstab but not change
       mount/unmount status.

       Valid values are defined (also called present),
       unmounted, absent, mounted.
    */

    auto state = upd.is.lookup<std::string>("ensure", "absent");
    auto ensure = upd.should.lookup<std::string>("ensure", state);

    changes.add("ensure", upd);

    if (ensure == "present") {
      // make sure entry in fstab
      err_ret( update_fstab(upd, changes) );
    } else if (ensure == "absent") {
      // unmount, remove from fstab
      err_ret( unmount(upd.name(), state) );
      err_ret( remove_from_fstab(upd) );
    } else if (ensure == "unmounted") {
      // unmount, make sure in fstab
      err_ret( unmount(upd.name(), state) );
      err_ret( update_fstab(upd, changes) );
    } else if (ensure == "mounted") {
      // mount, make sure in fstab
      err_ret( update_fstab(upd, changes) );
      err_ret( mount(upd.name(), state) );
    } else {
      return error(_("ensure has illegal value '{1}'", ensure));
    }
    return result<void>();
  }

  result<void>
  mount_provider::update_fstab(const update& upd, changes& changes) {
    changes.add({ "device", "fstype", "options", "dump", "pass"}, upd);
    return update_base(upd);
  }

  result<void>
  mount_provider::remove_from_fstab(const update &upd) {
    return base(upd).rm();
  }

  result<void>
  mount_provider::unmount(const std::string& name, const std::string& state) {
    err_ret(flush());
    if (state != "unmounted" && state != "absent") {
      return _cmd_umount->run({ name });
    }
    return result<void>();
  }

  result<void>
  mount_provider::mount(const std::string& name, const std::string& state) {
    err_ret(flush());
    if (state != "mounted") {
      return _cmd_mount->run({ name });
    }
    return result<void>();
  }

  result<void> mount_provider::flush() {
    return _aug->save();
  }
}
