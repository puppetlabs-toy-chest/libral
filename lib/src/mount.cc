#include <libral/mount.hpp>

#include <iostream>

#include <leatherman/execution/execution.hpp>

namespace libral {

  result<prov::spec> mount_provider::describe() {
    static const std::string desc =
#include "mount.yaml"
      ;
    return prov::spec::read("mount", desc);
  }


  result<bool> mount_provider::suitable() {
    if (this->aug == nullptr) {
      this->aug = aug::handle::make(_data_dir + "/lenses", AUG_NO_MODL_AUTOLOAD);

      aug->include("Mount_Fstab.lns", "/etc/fstab");
      aug->include("Mount_Fstab.lns", "/etc/mtab");
      aug->load();
      // FIXME: Check for errors from load()
    }
    _cmd_mount = leatherman::execution::which("mount");
    _cmd_umount = leatherman::execution::which("umount");
    return !_cmd_mount.empty() && !_cmd_umount.empty();
  }

  void mount_provider::flush() {
    aug->save();
  }

  std::vector<std::unique_ptr<resource>> mount_provider::instances() {
    std::map<std::string, std::unique_ptr<resource>> resources;
    auto shared_this = std::static_pointer_cast<mount_provider>(shared_from_this());

    for(const auto& node
          : aug->match("/files/etc/fstab/*[label() != '#comment']")) {
      auto name = node["file"];
      if (name) {
        auto mr = new mount_resource(shared_this, *name, node);
        auto res = std::unique_ptr<mount_resource>(mr);
        resources.emplace(res->name(), std::move(res));
      }
    }

    for(const auto& node
          : aug->match("/files/etc/mtab/*[label() != '#comment']")) {
      auto name = node["file"];
      if (name) {
        auto rsrc_iter = resources.find(*name);
        if (rsrc_iter != resources.end()) {
          (*rsrc_iter->second)["ensure"] = "mounted";
        } else {
          auto mr = new mount_resource(shared_this, *name, node);
          auto res = std::unique_ptr<resource>(mr);
          (*res)["ensure"] = "ghost";
          resources.emplace(res->name(), std::move(res));
        }
      }
    }

    std::vector<std::unique_ptr<resource>> result;
    for (auto pair = resources.begin(); pair != resources.end(); ++pair) {
      result.push_back(std::move(pair->second));
    }
    return result;
  }

  std::unique_ptr<resource> mount_provider::create(const std::string& name) {
    auto shared_this = std::static_pointer_cast<mount_provider>(shared_from_this());
    auto path = "/files/etc/fstab/0" + std::to_string(_seq++);
    auto base = aug->make_node(path);
    return std::unique_ptr<resource>(new mount_resource(shared_this, name, base));
  }

  void mount_provider::mount_resource::extract(const std::string& attr,
                               const boost::optional<std::string>& from) {
    if (from) {
      (*this)[attr] = *from;
    }
  }

  void mount_provider::mount_resource::extract(const std::string& attr,
                               const boost::optional<std::string>& from,
                               const std::string& deflt) {
    auto& self = *this;
    if (from) {
      self[attr] = *from;
    } else {
      self[attr] = deflt;
    }
  }

  void mount_provider::mount_resource::extract_base() {
    auto& self = *this;

    extract("device", _base["spec"]);
    extract("fstype", _base["vfstype"]);
    extract("options", _base["options"], "defaults");
    extract("dump", _base["dump"], "0");
    extract("pass", _base["passno"], "0");
    self["ensure"] = "unmounted";
    // @todo lutter 2016-05-20: we actually need to pay attention to target
    self["target"] = "/etc/fstab";
  }

  void mount_provider::mount_resource::update_base() {
    auto& self = *this;
    _base.erase();
    // FIXME: spec, file, and vfstype are mandatory, but we don't have a
    // way to report that they are missing right now, so we just make up
    // default values
    _base.set("spec", self["device"].to_string(), "NODEVICE");
    _base.set("file", name(), "NONAME");
    _base.set("vfstype", self["fstype"].to_string(), "NOFSTYPE");
    _base.set("options", self["options"].to_string(), "defaults");
    _base.set("dump", self["dump"].to_string(), "0");
    _base.set("passno", self["pass"].to_string(), "0");
  }

  std::unique_ptr<result<changes>>
  mount_provider::mount_resource::update(const attr_map& should) {
    auto res = std::unique_ptr<result<changes>>(new result<changes>(changes()));
    changes& changes = *res->ok();

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
    auto state = lookup<std::string>("ensure", "absent");
    auto ensure = should.lookup<std::string>("ensure", state);

    if (ensure == "present") {
      // make sure entry in fstab
      update_fstab(should, changes);
    } else if (ensure == "absent") {
      // unmount, remove from fstab
      unmount(state);
      remove_from_fstab();
    } else if (ensure == "unmounted") {
      // unmount, make sure in fstab
      unmount(state);
      update_fstab(should, changes);
    } else if (ensure == "mounted") {
      // mount, make sure in fstab
      update_fstab(should, changes);
      mount(state);
    } else {
      // raise "illegal value error"
    }

    if (state != ensure) {
      changes.push_back(change("ensure", ensure, state));
      (*this)["ensure"] = ensure;
    }

    return res;
  }

  void mount_provider::mount_resource::update_fstab(const attr_map& should,
                                                    changes& changes) {
    auto& self = *this;

    for (auto prop : { "device", "fstype", "options", "dump", "pass"}) {
      auto p = should.lookup<std::string>(prop);
      if (p && self[prop] != value(*p)) {
        changes.push_back(change(prop, *p, self[prop]));
        self[prop] = *p;
      }
    }
    update_base();
  }

  void mount_provider::mount_resource::remove_from_fstab() {
    _base.rm();
  }

  void mount_provider::mount_resource::unmount(const std::string& state) {
    _prov->flush();
    if (state != "unmounted" && state != "absent") {
      // FIXME: check results and return update_res
      leatherman::execution::execute(_prov->_cmd_umount, { this->name() });
    }
  }

  void mount_provider::mount_resource::mount(const std::string& state) {
    _prov->flush();
    if (state != "mounted") {
      // FIXME: check results and return update_res
      leatherman::execution::execute(_prov->_cmd_mount, { this->name() });
    }
  }
}
