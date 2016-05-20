#include <libral/resource.hpp>

namespace libral {
  static const std::string blank = std::string("");

  static bool is_name(const std::string& key) {
    // This basically hardcodes namevar for now
    return key == "name";
  }

  const std::string& resource::operator[](const std::string& key) const {
    if (is_name(key)) {
      return _name;
    } else {
      auto v = _values.find(key);
      if (v == _values.end()) {
        return blank;
      } else {
        return v->second;
      }
    }
  }

  std::string& resource::operator[](const std::string& key) {
    if (is_name(key)) {
      // @todo lutter 2016-05-20: this makes it possible to change the
      // name. Is that a good idea ?
      return _name;
    } else {
      return _values[key];
    }
  }
}
