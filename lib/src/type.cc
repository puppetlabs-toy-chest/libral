#include <libral/type.hpp>

#include <map>
#include <memory>
#include <boost/nowide/iostream.hpp>

#include <libral/mount.hpp>

#include <typeinfo>

namespace libral {

  boost::optional<std::unique_ptr<resource>>
  type::find(const std::string &name) {
    _prov->prepare();
    return _prov->find(name);
  }

  std::vector<std::unique_ptr<resource>> type::instances(void) {
    _prov->prepare();
    auto result = _prov->instances();
    _prov->flush();

    return result;
  }

  std::unique_ptr<resource> type::update(const std::string& name,
                                         const attr_map& attrs) {
    _prov->prepare();
    auto opt_rsrc = _prov->find(name);
    std::unique_ptr<resource> result;

    if (opt_rsrc) {
      result = std::move(*opt_rsrc);
    } else {
      result = _prov->create(name);
    }
    result->update(attrs);
    return result;
  }
}
