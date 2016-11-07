#include <libral/mount.hpp>

#include <iostream>

#include <leatherman/execution/execution.hpp>

namespace libral {

  bool mount_provider::suitable() {
    _cmd_mount = leatherman::execution::which("mount");
    _cmd_umount = leatherman::execution::which("umount");
    return !_cmd_mount.empty() && !_cmd_umount.empty();
  }

  void mount_provider::prepare() {
    auto h = new aug::handle(_data_dir + "/lenses", AUG_NO_MODL_AUTOLOAD);
    this->aug = std::unique_ptr<aug::handle>(h);

    aug->include("Mount_Fstab.lns", "/etc/fstab");
    aug->include("Mount_Fstab.lns", "/etc/mtab");
    aug->load();
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

  static void extract(boost::optional<std::string>& to,
                      const boost::optional<std::string>& from) {
    if (from) {
      to = *from;
    }
  }

  static void extract(boost::optional<std::string>& to,
                      const boost::optional<std::string>& from,
                      const std::string& deflt) {
    if (from) {
      to = *from;
    } else {
      to = deflt;
    }
  }

  void mount_provider::mount_resource::extract_base() {
    auto& self = *this;

    extract(self["device"], _base["spec"]);
    extract(self["fstype"], _base["vfstype"]);
    extract(self["options"], _base["options"], "defaults");
    extract(self["dump"], _base["dump"], "0");
    extract(self["pass"], _base["passno"], "0");
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
    _base.set("spec", self["device"], "NODEVICE");
    _base.set("file", name(), "NONAME");
    _base.set("vfstype", self["fstype"], "NOFSTYPE");
    _base.set("options", self["options"], "defaults");
    _base.set("dump", self["dump"], "0");
    _base.set("passno", self["pass"], "0");
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
    auto state = lookup("ensure", "absent");
    // Why is there no simple way to say "give me an existing entry, or a
    // default value otherwise" ?
    auto ensure_it = should.find("ensure");
    auto ensure = (ensure_it != should.end() && ensure_it->second)
      ? *ensure_it->second : state;

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
    return res;
  }

  void mount_provider::mount_resource::update_fstab(const attr_map& should,
                                                    changes& changes) {
    auto& self = *this;

    for (auto prop : { "device", "fstype", "options", "dump", "pass"}) {
      auto value = should.find(prop);
      if (value != should.end() && self[prop] != value->second) {
        changes.push_back(change(prop, value->second, self[prop]));
        self[prop] = value->second;
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
