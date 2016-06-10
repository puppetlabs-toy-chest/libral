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

    auto v = _values.find(key);
    if (v == _values.end()) {
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
    return _values[key];
  }

  const std::string& resource::lookup(const std::string& key,
                                      const std::string& deflt) const {
    auto it = _values.find(key);
    if (it == _values.end() || !it->second) {
      return deflt;
    } else {
      return *it->second;
    }
  }

  void resource::update_values(const attr_map& should) {
    _values = should;
  }

  boost::optional<std::unique_ptr<resource>> provider::find(const std::string &name) {
    for (auto& inst : instances()) {
      if (inst->name() == name)
        return std::move(inst);
    }
    return boost::none;
  }

}
