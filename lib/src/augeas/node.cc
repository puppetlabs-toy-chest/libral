#include <libral/augeas/node.hpp>

namespace libral { namespace augeas {

  node::node(std::shared_ptr<handle> aug, const std::string& path)
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

  result<boost::optional<std::string>>
  node::operator[](const std::string& path) const {
    return _aug->get(append(path));
  }

  result<void>
  node::set(const std::string& path, const value& v) {
    if (v) {
      return _aug->set(append(path), v.to_string());
    }
    return result<void>();
  }

  result<void>
  node::set(const std::string& path, const value& v, const std::string& deflt) {
    if (v) {
      return _aug->set(append(path), v.to_string());
    } else {
      return _aug->set(append(path), deflt);
    }
  }

  result<void> node::erase() {
    return _aug->rm(append("*"));
  }

  result<void> node::rm() {
    return _aug->rm(_path);
  }

  result<void> node::clear() {
    return _aug->clear(_path);
  }

} }
