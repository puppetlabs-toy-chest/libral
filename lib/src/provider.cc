#include <libral/provider.hpp>

namespace libral {
  static const std::string blank = std::string("");

  const value attr_map::operator[](const std::string& key) const {
    auto v = find(key);
    if (v == end()) {
      return boost::none;
    } else {
      return v->second;
    }
  }

  value& attr_map::operator[](const std::string& key) {
    return std::map<std::string, value>::operator[](key);
  }

  template<typename T>
  const T& attr_map::lookup(const std::string& key, const T& deflt) const {
    auto it = find(key);
    if (it == end()) {
      return deflt;
    } else if (auto p = it->second.as<T>()) {
      return *p;
    } else {
      return deflt;
    }
  }

  template<typename T>
  const T* attr_map::lookup(const std::string& key) const {
    auto it = find(key);
    if (it == end()) {
      return nullptr;
    } else {
      return it->second.as<T>();
    }
  }

  template const std::string& attr_map::lookup(const std::string&, const std::string&) const;
  template const std::string* attr_map::lookup(const std::string&) const;

  static bool is_name(const std::string& key) {
    // This basically hardcodes namevar for now
    return key == "name";
  }

  const value resource::operator[](const std::string& key) const {
    if (is_name(key)) {
      throw std::invalid_argument {
        "The name can not be accessed with operator[]" };
    }

    return _attrs[key];
  }

  value& resource::operator[](const std::string& key) {
    if (is_name(key)) {
      throw std::invalid_argument {
        "The name can not be accessed with operator[]" };
    }
    return _attrs[key];
  }

  template<typename T>
  const T& resource::lookup(const std::string& key, const T& deflt) const {
    return _attrs.lookup<T>(key, deflt);
  }

  template<typename T>
  const T* resource::lookup(const std::string& key) const {
    return _attrs.lookup<T>(key);
  };

  template const std::string& resource::lookup(const std::string&, const std::string&) const;
  template const std::string* resource::lookup(const std::string&) const;

  boost::optional<std::unique_ptr<resource>> provider::find(const std::string &name) {
    for (auto& inst : instances()) {
      if (inst->name() == name)
        return std::move(inst);
    }
    return boost::none;
  }

}
