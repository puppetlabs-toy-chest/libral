#include <libral/mount.hpp>

#include <sstream>

#include <leatherman/locale/locale.hpp>

using namespace leatherman::locale;

namespace aug = libral::augeas;

namespace libral {

  result<prov::spec> mount_provider::describe() {
    static const std::string desc =
#include "mount.yaml"
      ;
    return prov::spec::read("mount", desc);
  }


  result<bool> mount_provider::suitable() {
    if (_aug == nullptr) {
      std::stringstream buf;
      bool first=true;
      for (auto dir : _ral->data_dirs()) {
        if (!first)
          buf << ":";
        first=false;
        buf << dir << "/lenses";
      }
      _aug = aug::handle::make(buf.str(), AUG_NO_MODL_AUTOLOAD);

      _aug->include("Mount_Fstab.lns", "/etc/fstab");
      _aug->include("Mount_Fstab.lns", "/etc/mtab");
      _aug->load();
      // FIXME: Check for errors from load()
    }
    _cmd_mount = command::create("mount");
    _cmd_umount = command::create("umount");
    return _cmd_mount && _cmd_umount;;
  }

  void mount_provider::flush() {
    _aug->save();
  }

  aug::node mount_provider::base(const std::string &name) {
    // FIXME: create if it doesn't exist yet
    return aug::node(_aug, "/files/etc/fstab/*[file = '" + name + "']");
  }

  /* Set RES[ATTR] to the corresponding value in BASE[LBL]; if that does
   * not exist, use DEFLT. If DEFLT is none, do not set RES[ATTR]
   */
  void extract(resource& res, const aug::node& base,
               const std::string& attr, const std::string& label,
               const boost::optional<std::string>& deflt = boost::none) {
    auto from = base[label];
    if (from) {
      res[attr] = *from;
    } else if (deflt) {
      res[attr] = *deflt;
    }
  }

  void extract(resource& res, const aug::node& base,
               const std::string& attr, const std::string& label,
               const char * deflt) {
    extract(res, base, attr, label, std::string(deflt));
  }

  /**
   * Creates a new resource. The name will be NAME. Other attributes will
   * be filled in from the node BASE which must point to an fstab entry
   * in the augeas tree. The ensure attribute will be set to ENS
   */
  resource
  mount_provider::make(const std::string& name, const aug::node& base,
                       const std::string& ens) {
    auto res = create(name);

    extract(res, base, "device", "spec");
    extract(res, base, "fstype", "vfstype");
    extract(res, base, "options", "options", "defaults");
    extract(res, base, "dump", "dump", "0");
    extract(res, base, "pass", "passno", "0");
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

    for(const auto& node
          : _aug->match("/files/etc/fstab/*[label() != '#comment']")) {
      auto name = node["file"];
      if (name) {
        resources.emplace(*name, std::move(make(*name, node, "unmounted")));
      }
    }

    for(const auto& node
          : _aug->match("/files/etc/mtab/*[label() != '#comment']")) {
      auto name = node["file"];
      if (name) {
        auto rsrc_iter = resources.find(*name);
        if (rsrc_iter != resources.end()) {
          rsrc_iter->second["ensure"] = "mounted";
        } else {
          resources.emplace(*name, std::move(make(*name, node, "ghost")));
        }
      }
    }

    std::vector<resource> result;
    for (auto pair = resources.begin(); pair != resources.end(); ++pair) {
      result.push_back(std::move(pair->second));
    }
    return std::move(result);
  }

  void mount_provider::update_base(const update &upd) {
    auto bs = base(upd.name());

    bs.erase();
    // FIXME: spec, file, and vfstype are mandatory, but we don't have a
    // way to report that they are missing right now, so we just make up
    // default values
    bs.set("spec", upd["device"], "NODEVICE");
    bs.set("file", upd.name(), "NONAME");
    bs.set("vfstype", upd["fstype"], "NOFSTYPE");
    bs.set("options", upd["options"], "defaults");
    bs.set("dump", upd["dump"], "0");
    bs.set("passno", upd["pass"], "0");
  }

  result<void>
  mount_provider::set(context &ctx,
                      const updates& upds) {
    for (auto upd : upds) {
      result<void> res = set(ctx, upd);
      if (!res)
        return res.err();
    }
    return result<void>();
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
    auto& is = upd.is;
    auto& should = upd.should;

    auto state = is.lookup<std::string>("ensure", "absent");
    auto ensure = should.lookup<std::string>("ensure", state);

    changes.add("ensure", upd);

    if (ensure == "present") {
      // make sure entry in fstab
      update_fstab(upd, changes);
    } else if (ensure == "absent") {
      // unmount, remove from fstab
      auto run_res = unmount(upd.name(), state);
      if (! run_res)
        return run_res.err();
      remove_from_fstab(upd);
    } else if (ensure == "unmounted") {
      // unmount, make sure in fstab
      auto run_res = unmount(upd.name(), state);
      if (! run_res)
        return run_res.err();
      update_fstab(upd, changes);
    } else if (ensure == "mounted") {
      // mount, make sure in fstab
      update_fstab(upd, changes);
      auto run_res = mount(upd.name(), state);
      if (! run_res)
        return run_res.err();
    } else {
      return error(_("ensure has illegal value '{1}'", ensure));
    }
    return result<void>();
  }

  void mount_provider::update_fstab(const update& upd, changes& changes) {
    changes.add({ "device", "fstype", "options", "dump", "pass"}, upd);
    update_base(upd);
  }

  void mount_provider::remove_from_fstab(const update &upd) {
    base(upd.name()).rm();
  }

  result<void>
  mount_provider::unmount(const std::string& name, const std::string& state) {
    flush();
    if (state != "unmounted" && state != "absent") {
      return _cmd_umount->run({ name });
    }
    return result<void>();
  }

  result<void>
  mount_provider::mount(const std::string& name, const std::string& state) {
    flush();
    if (state != "mounted") {
      return _cmd_mount->run({ name });
    }
    return result<void>();
  }
}
