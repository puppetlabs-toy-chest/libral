#include <libral/type.hpp>

#include <map>
#include <memory>
#include <boost/nowide/iostream.hpp>

#include <typeinfo>

namespace libral {

  result<boost::optional<resource_uptr>>
  type::find(const std::string &name) {
    return _prov->find(name);
  }

  result<std::vector<resource_uptr>> type::instances(void) {
    auto result = _prov->instances();
    _prov->flush();

    return result;
  }

  result<std::pair<resource_uptr, changes>>
  type::update(const std::string& name,
               const attr_map& attrs) {
    auto opt_rsrc = _prov->find(name);

    if (!opt_rsrc) {
      return opt_rsrc.err();
    }

    resource_uptr res;
    if (*opt_rsrc) {
      res = std::move(**opt_rsrc);
    } else {
      res = _prov->create(name);
    }
    auto ch = res->update(attrs);
    if (!ch) {
      return ch.err();
    }
    return std::pair<resource_uptr, changes>(std::move(res), std::move(*ch));
  }

  result<value> type::parse(const std::string &name, const std::string &v) {
    return _prov->parse(name, v);
  }

}
