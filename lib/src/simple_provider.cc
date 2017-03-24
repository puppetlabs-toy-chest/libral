#include <libral/simple_provider.hpp>

#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <leatherman/execution/execution.hpp>

#include <leatherman/locale/locale.hpp>
#include <leatherman/logging/logging.hpp>

#include <libral/result.hpp>

#include <cstdio>

using namespace leatherman::locale;

namespace libral {

  result<void>
  simple_provider::set(context &ctx, const updates& upds) {
    for (auto upd : upds) {
      std::vector<std::string> args;
      auto& name = upd.name();
      auto& chgs = ctx.changes_for(name);

      auto cb = [&name, &chgs](std::string& key, std::string& value)
        -> result<bool> {
        if (key == "name") {
          if (value != name) {
            return error(_("wrong name changed by update: '{1}' instead of '{2}'",
                           value, name));
          }
        } else if (key == "ral_was") {
          chgs.back().was = value;
        } else {
          chgs.push_back(change(key, value));
        }
        return true;
      };

      for (auto p : upd.should.attrs()) {
        args.push_back(p.first + "='" + p.second.to_string() + "'");
      }
      auto r = run_action("update", cb, args);
      if (!r)
        return r.err();
    }
    return result<void>();
  }

  result<prov::spec> simple_provider::describe() {
    return prov::spec::read(_path, _node);
  }

  result<bool> simple_provider::suitable() {
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

  result<std::vector<resource>>
  simple_provider::get(context &ctx,
      const std::vector<std::string>& names,
      const resource::attributes& config) {
    if (names.size() == 1) {
      return find(names.front());
    } else {
      return instances();
    }
  }

  result<std::vector<resource>>
  simple_provider::find(const std::string &name) {
    std::vector<resource> res;

    auto cb = [this, &res, &name](std::string key, std::string value) -> result<bool> {
      if (key == "name") {
        res.push_back(create(value));
        return true;
      } else if (key == "ral_unknown") {
        return error(_("unknown resource {1}", name));
      } else {
        res.back()[key] = value;
        return true;
      }
    };

    auto r = run_action("find", cb, { "name='" + name + "'" });
    if (!r) {
      return r.err();
    }

    return res;
  }

  result<std::vector<resource>> simple_provider::instances() {
    // run script with ral_action == list
    std::vector<resource> res;

    auto cb = [this, &res](std::string key, std::string value) -> result<bool> {
      if (key == "name") {
        res.push_back(create(value));
      } else {
        if (res.size() == 0) {
          return error(_("format error: attribute values must come after the name"));
        }
        // FIXME: check that KEY is a valid attribute and VALUE is a valid
        // for the attribute's type
        res.back()[key] = value;
      }
      return true;
    };
    auto r = run_action("list", cb);
    if (!r) {
      return r.err();
    };
    return res;
  }

  result<bool>
  simple_provider::run_action(const std::string& action,
         std::function<result<bool>(std::string&, std::string&)> entry_cb,
         std::vector<std::string> args) {
    int line_cnt = 0;
    bool in_error = false;
    result<bool> rslt = true;
    std::string errmsg;

    args.push_back("ral_action=" + action);
    auto err_cb = [&errmsg](std::string &line) {
      errmsg += line;
      return true;
    };
    auto out_cb = [&line_cnt,&in_error,&errmsg,&entry_cb,&rslt](std::string &line) {
      line_cnt +=1;
      if (line_cnt == 1) {
        if (line != "# simple") {
          rslt = error(_("invalid line: '%s'. Expected '# simple'", line));
        }
      } else if (in_error) {
        if (line != "ral_eom") {
          errmsg += line;
        } else {
          rslt = error(errmsg);
        }
      } else {
        size_t pos = line.find_first_of(":");
        if (pos == std::string::npos) {
          rslt = error(_("invalid line: '%s'. Expected '<KEY>: <VALUE>' but couldn't find a ':'", line));
        }
        auto key = line.substr(0, pos);
        auto value = line.substr(pos+1);
        boost::trim(value);
        if (key == "ral_error") {
          in_error = true;
          errmsg = value;
        } else {
          auto r = entry_cb(key, value);
          if (r.is_err()) {
            rslt = r;
          }
        }
      }
      return rslt.is_ok();
    };
    auto r = leatherman::execution::each_line(_path, args, out_cb, err_cb);
    if (! r && rslt.is_ok()) {
      if (errmsg.empty()) {
        rslt = error(_("Something went wrong running %s ral_action=%s",
                       _path, action));
      } else {
        rslt = error(_("Something went wrong running %s ral_action=%s\n%s",
                       _path, action, errmsg));
      }
    }
    return rslt;
  }
}
