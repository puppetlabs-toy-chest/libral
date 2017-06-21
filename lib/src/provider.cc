#include <libral/provider.hpp>

#include <leatherman/locale/locale.hpp>

using namespace leatherman::locale;

namespace libral {

  result<std::vector<resource>>
  provider::get(const std::vector<std::string>& names) {
    resource::attributes config;
    context ctx(shared_from_this());

    return get(ctx, names, config);
  }

  result<boost::optional<resource>>
  provider::find(const std::string &name) {
    auto rsrcs = get({ name });
    err_ret(rsrcs);

    boost::optional<resource> res;
    for (auto& rsrc : rsrcs.ok()) {
      if (rsrc.name() == name) {
        if (res) {
          return error(_("[{1}] more than one resource with name {2}",
                         qname(), name));
        } else {
          res = rsrc;
        }
      }
    }
    return res;
  }

  result<std::vector<std::pair<update, changes>>>
  provider::set(const std::vector<resource>& shoulds) {
    context ctx(shared_from_this());
    resource::attributes config;

    // get the resoures mentioned in should
    std::vector<std::string> names;

    for (const auto& should : shoulds) {
      names.push_back(should.name());
    }
    auto r = get(ctx, names, config);
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
        auto is = create(should.name());
        upds.push_back({ .is = is, .should = should });
      }
    }

    err_ret( set(ctx, upds) );

    std::vector<std::pair<update, changes>> res;
    for (const auto& upd : upds) {
      res.push_back(std::make_pair(upd, ctx.changes_for(upd.name())));
    }
    return res;
  }

  result<std::pair<update, changes>>
  provider::set(const resource& should) {
    std::vector<resource> shoulds({ should });
    auto res = set(std::vector<resource>({should}));
    err_ret( res );
    return res.ok().at(0);
  }

  result<value> provider::parse(const std::string& name, const std::string& v) {
    if (!_spec) {
      return error(_("internal error: spec was not initialized"));
    }
    auto spec = _spec->attr(name);
    if (!spec) {
      return error(_("there is no attribute '{1}'", name));
    }

    return spec->read_string(v);
  }

  const std::string& provider::source() const {
    static const std::string s_builtin = "builtin";
    return s_builtin;
  }

  result<void> provider::prepare(environment &env) {
    auto res = describe(env);
    err_ret(res);

    _spec = res.ok();
    return result<void>();
  }

  resource provider::create(const std::string& name) const {
    return resource(name);
  }

  resource provider::create(const update &upd, const changes &chgs) const {
    resource res(upd.name());

    if (upd["ensure"] == value("absent")) {
      res["ensure"] = "absent";
    } else {
      res._attrs.insert(upd.is.attrs().begin(), upd.is.attrs().end());
      res._attrs.insert(upd.should.attrs().begin(), upd.should.attrs().end());
      for (const auto& ch : chgs) {
        res[ch.attr] = ch.is;
      }
    }
    return res;
  }

}
