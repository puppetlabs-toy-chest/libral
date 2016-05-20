#include <libral/resource.hpp>

namespace libral {
  static const std::string blank = std::string("");

  const std::string& resource::operator[](const std::string& key) const {
    auto v = _values.find(key);
    if (v == _values.end()) {
      return blank;
    } else {
      return v->second;
    }
  }

  std::string& resource::operator[](const std::string& key) {
    // @todo lutter 2016-05-20: this also allows changing the name. Is that
    // a good idea ?
    return _values[key];
  }

  void mount_resource::extract_base() {
    auto& self = *this;
    self["device"] = _base["spec"];
    self["fstype"] = _base["vfstype"];
    self["options"] = _base["options"];
    self["dump"] = _base["dump"];
    self["pass"] = _base["passno"];
    self["ensure"] = "unmounted";
    // @todo lutter 2016-05-20: we actually need to pay attention to target
    self["target"] = "/etc/fstab";
  }
}
