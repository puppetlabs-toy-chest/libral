#include <libral/host.hpp>

namespace aug = libral::augeas;

namespace libral {
  result<prov::spec> host_provider::describe(environment& env) {
    static const std::string desc =
#include "host.yaml"
      ;

    bool suitable = env.is_local();

    if (suitable) {
      auto aug = env.augeas({ { "Hosts.lns", "/etc/hosts" } });
      err_ret(aug);

      _aug = aug.ok();
    }

    return env.parse_spec("host", desc, suitable);
  }

  result<aug::node>
  host_provider::base(const update &upd) {
    // There could be multiple matches in a malformed /etc/hosts file. The
    // libral framework makes sure we do not modify these entries before we
    // ever get here - if there are duplicate entries, set() will never be
    // called, which is the only thing using this method
    auto nodes =
      _aug->match("/files/etc/hosts/*[canonical = '" + upd.name() + "']");
    err_ret( nodes );

    if (nodes.ok().size() == 1) {
      return (nodes.ok())[0];
    } else if (nodes.ok().size() == 0) {
      return _aug->make_node_seq_next("/files/etc/hosts");
    } else {
      // We still generate an error if we get more than one match just to be
      // defensive
      return error(_("multiple entries with hostname '{1}'", upd.name()));
    }
  }

  /**
   * Creates a new resource. The name will be NAME. Other attributes will
   * be filled in from the node BASE which must point to an hosts entry
   * in the augeas tree. The ensure attribute will be set to ENS
   */
  result<resource>
  host_provider::make(const std::string& name, const aug::node& base,
                      const std::string& ens) {
    auto res = create(name);

    err_ret( base.extract(res, "ip", "ipaddr") );
    err_ret( base.extract_array(res, "host_aliases", "alias") );
    err_ret( base.extract(res, "comment", "#comment") );

    res["ensure"] = ens;
    // @todo lutter 2016-05-20: we actually need to pay attention to target
    res["target"] = "/etc/hosts";

    return res;
  }

  result<std::vector<resource>>
  host_provider::get(context &ctx,
                     const std::vector<std::string>& names,
                     const resource::attributes& config) {
    std::vector<resource> res;

    auto nodes = _aug->match("/files/etc/hosts/*[label() != '#comment']");
    err_ret( nodes );

    for(const auto& node : nodes.ok()) {
      auto name = node["canonical"];
      err_ret(name);

      if (name.ok()) {
        auto r = make(**name, node, "present");
        err_ret(r);

        res.push_back(std::move(r.ok()));
      } else {
        // Can't happen, the lens makes sure we always have a 'canonical' entry
        return error(_("Missing canonical host name"));
      }
    }

    ctx.add_absent(res, names);
    return std::move(res);
  }

  result<void>
  host_provider::update_base(const update &upd) {
    auto rbs = base(upd);
    err_ret( rbs );
    auto bs = rbs.ok();

    err_ret( bs.erase() );
    // FIXME: ip is mandatory, but we don't have a way to report that they
    // are missing right now, so we just make up default values
    err_ret( bs.set("ipaddr", upd["ip"]) );
    err_ret( bs.set("canonical", upd.name()) );
    const auto& aliases = upd["host_aliases"].as<array>();
    if (aliases && ! aliases->empty()) {
      for (const auto& a : *aliases) {
        err_ret( bs.set("alias[last()+1]", a) );
      }
    }
    return bs.set("#comment", upd["comment"]);
  }

  result<void>
  host_provider::set(context &ctx,
                      const updates& upds) {
    for (auto upd : upds) {
      err_ret( set(ctx, upd) );
    }
    return _aug->save();
  }

  result<void>
  host_provider::set(context &ctx, const update &upd) {
    changes& changes = ctx.changes_for(upd.name());

    auto state = upd.is.lookup<std::string>("ensure", "absent");
    auto ensure = upd.should.lookup<std::string>("ensure", state);

    changes.add("ensure", upd);

    if (ensure == "present") {
      changes.add({ "ip", "host_aliases", "comment" }, upd);
      return update_base(upd);
    } else if (ensure == "absent") {
      return base(upd).ok().rm();
    } else {
      return error(_("ensure has illegal value '{1}'", ensure));
    }
  }
}
