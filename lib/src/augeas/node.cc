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

  result<std::vector<node>>
  node::match(const std::string& pathx) const {
    return _aug->match(append(pathx));
  }

  result<void>
  node::extract(resource& res, const std::string& attr,
                const std::string& label,
                const boost::optional<std::string>& deflt) const {
    auto from = operator[](label);
    err_ret( from );

    if (from.ok()) {
      res[attr] = *from.ok();
    } else if (deflt) {
      res[attr] = *deflt;
    }
    return result<void>();
  }

  result<void>
  node::extract(resource& res, const std::string& attr,
                const std::string& label, const char * deflt) const {
    return extract(res, attr, label, std::string(deflt));
  }

  result<void>
  node::extract_array(resource& res, const std::string& attr,
                      const std::string& label) const {
    array ary;
    auto matches = match(label);
    err_ret(matches);
    for (auto& match : matches.ok()) {
      auto val = match.get();
      err_ret(val);
      if (val.ok()) {
        ary.push_back(*val.ok());
      }
    }
    if (! ary.empty())
      res[attr] = ary;
    return result<void>();
  }

  result<void> node::resolve() {
    auto m = _aug->match(_path);
    err_ret( m );
    if (m.ok().size() != 1) {
      return error(_("Expected {1} to match exactly one node, but it matched {2}",
                     _path, m.ok().size()));
    }
    _path = m.ok().front().path();
    return result<void>();
  }


} }
