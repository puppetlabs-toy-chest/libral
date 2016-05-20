#include <libral/provider.hpp>

#include <libral/resource.hpp>

#include <iostream>

namespace libral {
  void mount_provider::prepare() {
    // @todo lutter 2016-05-16: pass in the directory with our lenses
    this->aug = std::unique_ptr<aug::handle>(new aug::handle(NULL, "/home/lutter/code/libral/lenses", AUG_NO_MODL_AUTOLOAD));

    aug->include("Mount_Fstab.lns", "/etc/fstab");
    aug->include("Mount_Fstab.lns", "/etc/mtab");
    aug->load();
  }

  void mount_provider::flush() {
    // @todo lutter 2016-05-16: call aug->save() ?
  }

  std::vector<std::unique_ptr<resource>> mount_provider::instances() {
    std::map<std::string, std::unique_ptr<resource>> resources;
    auto shared_this = std::static_pointer_cast<mount_provider>(shared_from_this());

    for(const auto& node
          : aug->match("/files/etc/fstab/*[label() != '#comment']")) {
      auto mr = new mount_resource(shared_this, node);
      auto res = std::unique_ptr<mount_resource>(mr);
      resources.emplace(res->name(), std::move(res));
    }

    for(const auto& node
          : aug->match("/files/etc/mtab/*[label() != '#comment']")) {
      auto name = node["file"];
      auto rsrc_iter = resources.find(name);
      if (rsrc_iter != resources.end()) {
        (*rsrc_iter->second)["ensure"] = "mounted";
      } else {
        auto res = std::unique_ptr<resource>(new mount_resource(shared_this, node));
        (*res)["ensure"] = "ghost";
        resources.emplace(res->name(), std::move(res));
      }
    }

    std::vector<std::unique_ptr<resource>> result;
    for (auto pair = resources.begin(); pair != resources.end(); ++pair) {
      result.push_back(std::move(pair->second));
    }
    return result;
  }
}
