#include <libral/provider.hpp>

#include <leatherman/locale/locale.hpp>

using namespace leatherman::locale;

namespace libral {

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
