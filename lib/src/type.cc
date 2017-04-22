#include <libral/type.hpp>

#include <map>
#include <memory>
#include <boost/nowide/iostream.hpp>

#include <leatherman/locale/locale.hpp>

#include <typeinfo>

using namespace leatherman::locale;

namespace libral {

  result<boost::optional<resource>>
  type::find(const std::string &name) {
    std::vector<std::string> names = { name };
    resource::attributes config;
    context ctx { _prov };
    auto rsrcs = _prov->get(ctx, names, config);
    err_ret(rsrcs);

    boost::optional<resource> res;
    for (auto& rsrc : rsrcs.ok()) {
      if (rsrc.name() == name) {
        if (res) {
          return error(_("[{1}] more than one resource with name {2}",
                         _prov->spec()->qname(), name));
        } else {
          res = rsrc;
        }
      }
    }
    return res;
  }

  result<std::vector<resource>> type::instances(void) {
    std::vector<std::string> names;
    resource::attributes config;
    context ctx { _prov };
    auto result = _prov->get(ctx, names, config);

    return result;
  }

  result<std::pair<update, changes>>
  type::set(const resource& should) {
    auto opt_rsrc = find(should.name());

    if (!opt_rsrc) {
      return opt_rsrc.err();
    }

    resource is = (*opt_rsrc) ? std::move(**opt_rsrc)
      : _prov->create(should.name());

    context ctx { _prov };
    update upd = { .is = is, .should = should };

    auto res = _prov->set(ctx, { upd });
    if (!res) {
      return res.err();
    }

    auto chgs = ctx.changes_for(should.name());

    return std::pair<update, changes>(std::move(upd), std::move(chgs));
  }

  result<value> type::parse(const std::string &name, const std::string &v) {
    return _prov->parse(name, v);
  }

}
