#include <libral/type.hpp>

#include <map>
#include <memory>
#include <boost/nowide/iostream.hpp>

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

  std::pair<std::unique_ptr<resource>,std::unique_ptr<result<changes>>>
  type::update(const std::string& name,
               const attr_map& attrs) {
    _prov->prepare();
    auto opt_rsrc = _prov->find(name);
    std::unique_ptr<resource> res;

    if (opt_rsrc) {
      res = std::move(*opt_rsrc);
    } else {
      res = _prov->create(name);
    }
    auto ch = res->update(attrs);
    return
      std::pair<std::unique_ptr<resource>,
                std::unique_ptr<result<changes>>>(std::move(res),
                                                  std::move(ch));
  }

  result<value> type::parse(const std::string &name, const std::string &v) {
    // Right now, this is pretty much a noop; in the future, we want to use
    // attribute metadata to guide this conversion
    return value::read_string(v);
  }

}
