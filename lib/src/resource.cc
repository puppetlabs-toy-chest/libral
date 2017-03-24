#include <libral/resource.hpp>

namespace libral {

  bool resource::is_name(const std::string& key) const {
    // This hardcodes namevar for now
    return key == "name";
  }

  const value& resource::operator[](const std::string& key) const {
    if (is_name(key)) {
      throw std::invalid_argument {
        "The name can not be accessed with operator[]" };
    }

    auto v = _attrs.find(key);
    if (v == _attrs.end()) {
      return value::none;
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

  template<typename T>
  const T& resource::lookup(const std::string& key, const T& deflt) const {
    auto it = _attrs.find(key);
    if (it == _attrs.end()) {
      return deflt;
    } else if (auto p = it->second.as<T>()) {
      return *p;
    } else {
      return deflt;
    }
  }

  template<typename T>
  const T* resource::lookup(const std::string& key) const {
    auto it = _attrs.find(key);
    if (it == _attrs.end()) {
      return nullptr;
    } else {
      return it->second.as<T>();
    }
  };

  template const std::string&
  resource::lookup(const std::string&, const std::string&) const;
  template const std::string*
  resource::lookup(const std::string&) const;
  template const array&
  resource::lookup(const std::string&, const array&) const;
  template const array*
  resource::lookup(const std::string&) const;
}
