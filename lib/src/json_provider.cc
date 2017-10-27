#include <libral/json_provider.hpp>

#include <sstream>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>

#include <leatherman/locale/locale.hpp>

#include <libral/result.hpp>

#include <cstdio>

using namespace leatherman::locale;
namespace fs = boost::filesystem;
namespace json = leatherman::json_container;
using json_container = json::JsonContainer;
using json_keys = std::vector<json::JsonContainerKey>;


namespace libral {

  result<boost::optional<value>>
  json_provider::value_from_json(const std::string &name,
                                 const json_container& json,
                                 const json_keys& key) {
    const auto& attr = spec()->attr(name);
    if (!attr) {
      // silently skip extraneous attributes
      return boost::optional<value>(boost::none);
    }

    auto v = attr->from_json(json, key);
    err_ret(v);
    return boost::optional<value>(v.ok());
  }

  result<void>
  json_provider::set(context &ctx, const updates& upds) {
    auto inp = json_container();

    // Generate the provider input
    inp.set<bool>({ "ral", "noop" }, false);
    std::vector<json_container> json_upds;
    for (const auto& upd : upds) {
      json_container json_upd;
      json_upd.set<std::string>("name", upd.name());

      for (auto& k : upd.is.attrs()) {
        k.second.to_json(json_upd, { "is", k.first });
      }
      for (auto& k : upd.should.attrs()) {
        k.second.to_json(json_upd, { "should", k.first });
      }
      json_upds.push_back(json_upd);
    }
    inp.set<std::vector<json_container>>("updates", json_upds);

    // Run it
    auto out=run_action(ctx, "set", inp);
    if (!out) {
      return out.err();
    }

    try {
      std::string message, kind;
      if (contains_error(*out, message, kind)) {
        return ctx.error(_("update failed: {1}", message));
      }

      // convert output
      if (out->includes("changes")) {
        auto json_chgs = out->get<std::vector<json_container>>("changes");

        for (const auto& json_chg : json_chgs) {
          // json_chg is an object
          // { "name": ..., "attr1": ..., ..., "attrN": ... }
          if (! json_chg.includes("name")) {
            return ctx.error(
                   _("malformed change: entry does not contain a name"));
          }

          auto name = json_chg.get<std::string>("name");
          auto& chgs = ctx.changes_for(name);

          for (auto k : json_chg.keys()) {
            if (k == "name") continue;

            if (! json_chg.includes({ k, "is" })) {
              return ctx.error(
                _("malformed change: entry for {1}.{2} does not contain 'is'",
                  name, k));
            }
            if (! json_chg.includes({ k, "was" })) {
              return error(
                _("malformed change: entry for {1}.{2} does not contain 'was'",
                  name, k));
            }
            auto is = value_from_json(k, json_chg, { k, "is"});
            if (!is) {
              return ctx.error(is.err().detail);
            }
            auto was = value_from_json(k, json_chg, { k, "was"});
            if (! was) {
              return ctx.error(was.err().detail);
            }
            chgs.add(k, *is.ok(), *was.ok());
          }
        }
      }

      // auto-add changes derived from the update
      if (out->getWithDefault<bool>("derive", false)) {
        for (const auto& upd : upds) {
          if (! ctx.have_changes(upd.name())) {
            ctx.changes_for(upd.name()).maybe_add(upd);
          }
        }
      }
    } catch (const json::data_error& e) {
      return ctx.error(_("output does not conform to calling convention: {1}", out.ok().toString()));
    }

    return result<void>();
  }

  result<std::vector<resource>>
  json_provider::get(context &ctx,
                     const std::vector<std::string>& names,
                     const resource::attributes& config) {
    // run script with ral_action == list
    std::vector<resource> result;
    auto inp = json_container();
    inp.set<std::vector<std::string>>("names", names);

    auto out = run_action(ctx, "get", inp);
    err_ret(out);

    try {
      std::string message, kind;
      if (contains_error(*out, message, kind)) {
        return ctx.error(_("get failed with error {1}", message));
      }
      if (!out->includes("resources")) {
        return ctx.error(_("get did not produce a 'resources' entry"));
      }
      auto json_rsrcs = out->get<std::vector<json_container>>("resources");
      for (const auto& json_rsrc : json_rsrcs) {
        auto rsrc = resource_from_json(ctx, json_rsrc);
        err_ret(rsrc);

        result.push_back(std::move(rsrc.ok()));
      }
      return std::move(result);
    } catch (const json::data_error& e) {
      return ctx.error(_("output does not conform to calling convention: {1}", out.ok().toString()));
    }
  }

  result<json_container>
  json_provider::run_action(context& ctx,
                            const std::string& action,
                            const json_container& json) {
    auto inp = json.toString();
    ctx.log_debug("passing {1} on stdin", inp);
    auto res = _cmd->execute({ "ral_action=" + action }, inp);
    if (!res.error.empty()) {
      // FIXME: this should really happen in a callback while the command
      // is running rather than after the command finished. For that, we'd
      // have to switch to exe::each_line and accumulate res.output
      std::istringstream is(res.error);
      std::string line;
      while (std::getline(is, line, '\n')) {
        ctx.log_line(line);
      }
    }
    if (!res.success) {
      if (res.output.empty()) {
        return ctx.error(_("action '{1}' exited with status {2}",
                           action, res.exit_code));
      } else {
        if (res.error.empty()) {
          return ctx.error(
                      _("action '{1}' exited with status {2}. Output was '{3}'",
                        action, res.exit_code, res.output));
        }
      }
    }

    try {
      return json_container(res.output);
    } catch (json::data_parse_error& e) {
      return ctx.error(_("action '{1}' returned invalid JSON '{2}'",
                         action, res.output));
    }
  }

  bool json_provider::contains_error(const json_container& json,
                                     std::string& message,
                                     std::string& kind) {
    auto res = json.includes("error");
    if (res) {
      message = json.getWithDefault<std::string>({"error", "message"}, "");
      kind = json.getWithDefault<std::string>({"error", "kind"}, "failed");
    }
    return res;
  }

  result<resource>
  json_provider::resource_from_json(const context& ctx,
                                    const json_container& json) {
    auto name = json.get<std::string>("name");
    auto rsrc = create(name);

    for (auto k : json.keys()) {
      if (k == "name") continue;

      auto v = value_from_json(k, json, { k });
      if (!v) {
        // Make sure the message gets marked with the provider name
        return ctx.error(v.err().detail);
      }
      if (v.ok())
        rsrc[k] = *v.ok();
    }
    return rsrc;
  }
}
