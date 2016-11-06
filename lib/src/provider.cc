#include <libral/provider.hpp>

namespace libral {
  static const std::string blank = std::string("");

  static bool is_name(const std::string& key) {
    // This basically hardcodes namevar for now
    return key == "name";
  }

  const value resource::operator[](const std::string& key) const {
    if (is_name(key)) {
      throw std::invalid_argument {
        "The name can not be accessed with operator[]" };
    }

    auto v = _attrs.find(key);
    if (v == _attrs.end()) {
      return boost::none;
    } else {
      return v->second;
    }
  }

  value& resource::operator[](const std::string& key) {
    if (is_name(key)) {
      throw std::invalid_argument {
        "The name can not be accessed with operator[]" };
    }
    return _attrs[key];
  }

  const std::string& resource::lookup(const std::string& key,
                                      const std::string& deflt) const {
    auto it = _attrs.find(key);
    if (it == _attrs.end() || !it->second) {
      return deflt;
    } else {
      return *it->second;
    }
  }

  void resource::set_attrs(const attr_map& should) {
    _attrs = should;
  }

  boost::optional<std::unique_ptr<resource>> provider::find(const std::string &name) {
    for (auto& inst : instances()) {
      if (inst->name() == name)
        return std::move(inst);
    }
    return boost::none;
  }

}
