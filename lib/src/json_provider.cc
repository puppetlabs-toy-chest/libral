#include <libral/json_provider.hpp>

#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <leatherman/execution/execution.hpp>
#include <boost/filesystem.hpp>

#include <leatherman/locale/locale.hpp>

#include <libral/result.hpp>

#include <cstdio>

using namespace leatherman::locale;
namespace exe = leatherman::execution;
namespace json = leatherman::json_container;
namespace fs = boost::filesystem;

namespace libral {

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
    auto inp = json::JsonContainer();
    inp.set<bool>({ "ral", "noop" }, false);
    inp.set<std::string>({ "resource", "name" }, upd.name());

    for (auto& k : upd.should.attrs()) {
      // FIXME: We should really map libral values <-> JSON values (e.g. bool)
      inp.set<std::string>({ "resource", k.first }, k.second.to_string());
    }
    auto out=run_action("update", inp);
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

    auto json_chgs = out->get<json::JsonContainer>("changes");
    for (auto k : json_chgs.keys()) {
      if (! json_chgs.includes({ k, "is" })) {
        return error(_("malformed change: entry for {1} does not contain 'is'",
                       k));
      }
      if (! json_chgs.includes({ k, "was" })) {
        return error(_("malformed change: entry for {1} does not contain 'was'",
                       k));
      }

      auto is = value(json_chgs.get<std::string>({ k, "is"}));
      auto was = value(json_chgs.get<std::string>({ k, "was"}));
      chgs.add(k, is, was);
    }
    return result<void>();
  }

  result<prov::spec> json_provider::describe() {
    auto name = fs::path(_path).filename().stem();
    return prov::spec::read(name.native(), _node);
  }

  result<bool> json_provider::suitable() {
    // Same as in simple_provider
    auto meta = _node["provider"];
    if (! meta.IsMap()) {
      return error(_("expected 'provider' key in metadata to contain a map"));
    }
    auto s = meta["suitable"].as<std::string>();
    if (s != "true" && s != "false") {
      return error(_("provider {1} (simple): metadata 'suitable' must be either 'true' or 'false' but was '{2}'", _path, s));
    }
    return (s == "true");
  }

#if 0
  result<boost::optional<resource_uptr>>
  json_provider::find(const std::string &name) {
    auto inp = json::JsonContainer();
    inp.set<std::string>({ "resource", "name" }, name);

    auto out=run_action("find", inp);
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
    auto json_rsrc = out->get<json::JsonContainer>("resource");
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
    auto inp = json::JsonContainer();
    auto out = run_action("list", inp);
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
    auto json_rsrcs = out->get<std::vector<json::JsonContainer>>("resources");
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

  result<json::JsonContainer>
  json_provider::run_action(const std::string& action,
                            const json::JsonContainer& json) {
    auto inp = json.toString();
    auto res = exe::execute(_path, { "ral_action=" + action },
                            inp,
                            0, { exe::execution_options::trim_output,
                                exe::execution_options::merge_environment });
    if (!res.success) {
      if (res.output.empty()) {
        if (res.error.empty()) {
          return error(_("action '{1}' exited with status {2}",
                         action, res.exit_code));
        } else {
          return error(_("action '{1}' exited with status {2}. stderr was '{3}'",
                         action, res.exit_code, res.error));
        }
      } else {
        if (res.error.empty()) {
          return error(_("action '{1}' exited with status {2}. Output was '{3}'",
                         action, res.exit_code, res.output));
        } else {
          return error(_("action '{1}' exited with status {2}. Output was '{3}'. stderr was '{4}'",
                         action, res.exit_code, res.output, res.error));
        }
      }
    }
    if (! res.error.empty()) {
      return error(_("action '{1}' produced stderr '{2}'", action, res.error));
    }

    try {
      return json::JsonContainer(res.output);
    } catch (json::data_parse_error& e) {
      return error(_("action '{1}' returned invalid JSON '{2}'",
                     action, res.output));
    }
  }

  bool json_provider::contains_error(const json::JsonContainer& json,
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
  json_provider::resource_from_json(const json::JsonContainer& json) {
    if (!json.includes("name")) {
      return error(_("resource does not have a name"));
    }

    auto name = json.get<std::string>("name");
    auto rsrc = create(name);

    for (auto k : json.keys()) {
      if (k == "name") {
        continue;
      }
      rsrc[k] = value(json.get<std::string>(k));
    }
    return rsrc;
  }
}
