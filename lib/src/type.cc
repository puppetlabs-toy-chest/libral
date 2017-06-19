#include <libral/type.hpp>

#include <map>
#include <memory>
#include <boost/nowide/iostream.hpp>

#include <leatherman/locale/locale.hpp>

#include <typeinfo>

using namespace leatherman::locale;

namespace libral {

  result<std::vector<resource>>
  type::get(const std::vector<std::string>& names) {
    resource::attributes config;
    context ctx { _prov };

    return _prov->get(ctx, names, config);
  }

  result<boost::optional<resource>>
  type::find(const std::string &name) {
    auto rsrcs = get({ name });
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

  result<std::pair<update, changes>>
  type::set(const resource& should) {
    auto opt_rsrc = find(should.name());
    err_ret(opt_rsrc);

    resource is = (*opt_rsrc) ? std::move(**opt_rsrc)
      : _prov->create(should.name());

    context ctx { _prov };
    update upd = { .is = is, .should = should };

    auto res = _prov->set(ctx, { upd });
    err_ret(res);

    auto chgs = ctx.changes_for(should.name());

    return std::pair<update, changes>(std::move(upd), std::move(chgs));
  }

  result<std::vector<std::pair<update, changes>>>
  type::set(const std::vector<resource>& shoulds) {
    context ctx { _prov };
    resource::attributes config;

    // get the resoures mentioned in should
    std::vector<std::string> names;

    for (const auto& should : shoulds) {
      names.push_back(should.name());
    }
    auto r = _prov->get(ctx, names, config);
    err_ret(r);

    auto iss = r.ok();

    updates upds;
    for (const auto& should: shoulds) {
      bool found = false;
      for (const auto& is : iss) {
        if (is.name() == should.name()) {
          found = true;
          upds.push_back({ .is = is, .should = should });
        }
      }
      if (!found) {
        auto is = _prov->create(should.name());
        upds.push_back({ .is = is, .should = should });
      }
    }

    err_ret( _prov->set(ctx, upds) );

    std::vector<std::pair<update, changes>> res;
    for (const auto& upd : upds) {
      res.push_back(std::make_pair(upd, ctx.changes_for(upd.name())));
    }
    return res;
  }

  result<value> type::parse(const std::string &name, const std::string &v) {
    return _prov->parse(name, v);
  }

}
