#include <libral/type.hpp>

#include <map>
#include <memory>
#include <boost/nowide/iostream.hpp>

#include <libral/mount.hpp>

#include <typeinfo>

namespace libral {
  std::vector<std::unique_ptr<resource>> type::instances(void) {
    std::vector<std::unique_ptr<resource>> result;
    result.push_back(std::unique_ptr<resource>(new resource(this->name() + "_1")));
    result.push_back(std::unique_ptr<resource>(new resource(this->name() + "_2")));
    return result;
  }

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
  std::vector<std::unique_ptr<resource>> mount_type::instances(void) {
    std::shared_ptr<mount_provider> p(new mount_provider());

    p->prepare();
    auto result = p->instances();
    p->flush();

    return result;
  }

#if 0
  /* Some code to help me think about things */
  type::lifecycle(void) {
    for (x : candidates(*this)) {
      if (x.suitable()) {
        x.prepare();
        // Loop over all providers
        for (auto p : x.instances()) {
          modify(p);
        }
        // or do something to a specific provider
        auto p = x.find(some_name);
        x.flush();
        if (use_only_first)
          break;
      }
    }
  }
#endif
}
