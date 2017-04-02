#include <libral/json_provider.hpp>

#include <sstream>
#include <iostream>
#include <string>

#include <leatherman/execution/execution.hpp>
#include <boost/filesystem.hpp>

#include <leatherman/locale/locale.hpp>

#include <libral/result.hpp>

#include <cstdio>

using namespace leatherman::locale;
namespace exe = leatherman::execution;
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
    for (auto upd : upds) {
      auto res = set(ctx, upd);
      if (!res)
        return res.err();
    }
    return result<void>();
  }

  result<void>
  json_provider::set(context &ctx, const update &upd) {
    auto inp = json_container();
    inp.set<bool>({ "ral", "noop" }, false);
    inp.set<std::string>({ "resource", "name" }, upd.name());

    for (auto& k : upd.should.attrs()) {
      k.second.to_json(inp, { "resource", k.first });
    }
    auto out=run_action(ctx, "update", inp);
    if (!out) {
      return out.err();
    }

    std::string message, kind;
    if (contains_error(*out, message, kind)) {
      return error(_("update failed: {1}", message));
    }

    auto& chgs = ctx.changes_for(upd.name());

    // FIXME: should we consider this an error or an indication that no
    // changes were made ?
    if (! out->includes("changes")) {
      return result<void>();
    }

    auto json_chgs = out->get<json_container>("changes");
    for (auto k : json_chgs.keys()) {
      if (! json_chgs.includes({ k, "is" })) {
        return error(_("malformed change: entry for {1} does not contain 'is'",
                       k));
      }
      if (! json_chgs.includes({ k, "was" })) {
        return error(_("malformed change: entry for {1} does not contain 'was'",
                       k));
      }

      auto is = value_from_json(k, json_chgs, { k, "is"});
      err_ret(is);
      auto was = value_from_json(k, json_chgs, { k, "was"});
      err_ret(was);
      chgs.add(k, *is.ok(), *was.ok());
    }
    return result<void>();
  }

  result<prov::spec> json_provider::describe(environment &env) {
    auto name = fs::path(_path).filename().stem();

    return env.parse_spec(name.native(), _node);
  }

#if 0
  result<boost::optional<resource_uptr>>
  json_provider::find(const std::string &name) {
    auto inp = json_container();
    inp.set<std::string>({ "resource", "name" }, name);

    auto out=run_action(ctx, "find", inp);
    if (!out) {
      return error(_("provider[{1}]: {2}", _path, out.err().detail));
    }

    std::string message, kind;
    if (contains_error(*out, message, kind)) {
      if (kind == "unknown") {
        return boost::optional<resource_uptr>(boost::none);
      } else {
        return
          error(_("provider[{1}]: find for name '{2}' failed with error {3}",
                  _path, name, message));
      }
    }
    if (!out->includes("resource")) {
      return error(_("provider[{1}]: find did not produce a 'resource' entry",
                     _path));
    }
    auto json_rsrc = out->get<json_container>("resource");
    auto rsrc = resource_from_json(json_rsrc);
    if (!rsrc) {
      return error(_("provider[{1}]: find of '{2}': {3}",
                     _path, name, rsrc.err().detail));
    }
    if ((*rsrc)->name() != name) {
      return error(_("provider[{1}]: find of name '{2}' returned resource named '{3}'", _path, name, (*rsrc)->name()));
    }
    return boost::optional<resource_uptr>(std::move(*rsrc));
  }
#endif

  result<std::vector<resource>>
  json_provider::get(context &ctx,
                     const std::vector<std::string>& names,
                     const resource::attributes& config) {
    // run script with ral_action == list
    std::vector<resource> result;
    auto inp = json_container();
    auto out = run_action(ctx, "list", inp);
    if (!out) {
      return error(_("provider[{1}]: {2}", _path, out.err().detail));
    }
    std::string message, kind;
    if (contains_error(*out, message, kind)) {
      return error(_("provider[{1}]: list failed with error {2}",
                     _path, message));
    }
    if (!out->includes("resources")) {
      return error(_("provider[{1}]: list did not produce a 'resources' entry",
                     _path));
    }
    auto json_rsrcs = out->get<std::vector<json_container>>("resources");
    for (auto json_rsrc : json_rsrcs) {
      auto rsrc = resource_from_json(json_rsrc);
      if (!rsrc) {
        return error(_("provider[{1}]: list failed: {2}",
                       _path, rsrc.err().detail));
      }
      result.push_back(std::move(*rsrc));
    }
    return std::move(result);
  }

  result<json_container>
  json_provider::run_action(context& ctx,
                            const std::string& action,
                            const json_container& json) {
    auto inp = json.toString();
    ctx.log_debug("passing {1} on stdin", inp);
    auto res = exe::execute(_path, { "ral_action=" + action },
                            inp,
                            0, { exe::execution_options::trim_output,
                                exe::execution_options::merge_environment });
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
        return error(_("action '{1}' exited with status {2}",
                       action, res.exit_code));
      } else {
        if (res.error.empty()) {
          return error(_("action '{1}' exited with status {2}. Output was '{3}'",
                         action, res.exit_code, res.output));
        }
      }
    }

    try {
      return json_container(res.output);
    } catch (json::data_parse_error& e) {
      return error(_("action '{1}' returned invalid JSON '{2}'",
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
  json_provider::resource_from_json(const json_container& json) {
    if (!json.includes("name")) {
      return error(_("resource does not have a name"));
    }

    auto name = json.get<std::string>("name");
    auto rsrc = create(name);

    for (auto k : json.keys()) {
      if (k == "name") {
        continue;
      }
      auto v = value_from_json(k, json, { k });
      err_ret(v);
      if (v.ok())
        rsrc[k] = *v.ok();
    }
    return rsrc;
  }
}
