#include <libral/augeas/node.hpp>

namespace libral { namespace augeas {

  node::node(std::shared_ptr<handle> aug, const std::string path)
    : _aug(aug), _path(path) { }

  std::string node::append(const std::string& p2) const {
    if (p2.front() == '/') {
      return p2;
    }
    if (_path.back() == '/') {
      return _path + p2;
    } else {
      return _path + "/" + p2;
    }
  }

  boost::optional<std::string> node::operator[](const std::string& path) const {
    return _aug->value(append(path));
  }

  void node::set(const std::string& path, const value& v) {
    if (v) {
      _aug->set(append(path), v.to_string());
    }
  }

  void node::set(const std::string& path,
                 const value& v,
                 const std::string& deflt) {
    if (v) {
      _aug->set(append(path), v.to_string());
    } else {
      _aug->set(append(path), deflt);
    }
  }

  void node::set_maybe(const std::string& path,
                       const boost::optional<std::string>& value) {
    if (value) {
      _aug->set(append(path), *value);
    }
  }

  void node::erase() {
    _aug->rm(append("*"));
  }

  void node::rm() {
    _aug->rm(_path);
  }

  void node::clear() {
    _aug->clear(_path);
  }

} }
