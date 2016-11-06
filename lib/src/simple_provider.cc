#include <libral/simple_provider.hpp>

#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <leatherman/execution/execution.hpp>

#include <leatherman/logging/logging.hpp>

namespace libral {

  void simple_provider::simple_resource::update(const attr_map &should) {
    // Run the script with ral_action = 'update' and attrs should
  }

  bool simple_provider::suitable() {
    // Return node["meta"]["suitable"]
    return false;
  }

  void simple_provider::prepare() {
    // Check/setup stuff
  }

  void simple_provider::flush() {
    // Noop. Not supported/needed
  }

  std::unique_ptr<resource> simple_provider::create(const std::string& name) {
    auto shared_this =
      std::static_pointer_cast<simple_provider>(shared_from_this());
    return std::unique_ptr<resource>(new simple_resource(shared_this, name));
  }

  boost::optional<std::unique_ptr<resource>>
  simple_provider::find(const std::string &name) {
    bool unknown = false;
    std::unique_ptr<resource> rsrc;

    auto cb = [this, &unknown, &rsrc](std::string key, std::string value) {
      if (key == "name") {
        rsrc = create(value);
        return true;
      } else if (key == "ral_unknown") {
        unknown=true;
        return false;
      } else {
        (*rsrc)[key] = value;
        return true;
      }
    };
    run_action("find", cb, { "name='" + name + "'" });
    if (unknown) {
      return boost::none;
    } else {
      return std::move(rsrc);
    }
  }

  std::vector<std::unique_ptr<resource>> simple_provider::instances() {
    // run script with ral_action == list
    std::vector<std::unique_ptr<resource>> result;
    std::unique_ptr<resource> rsrc;

    auto cb = [this, &result, &rsrc](std::string key, std::string value) {
      if (key == "name") {
        if (rsrc != nullptr) {
          result.push_back(std::move(rsrc));
        }
        rsrc = create(value);
      } else {
        (*rsrc)[key] = value;
      }
      return true;
    };
    run_action("list", cb);
    return result;
  }

  bool simple_provider::run_action(const std::string& action,
         std::function<bool(std::string&, std::string&)> entry_callback,
         std::vector<std::string> args) {
    int line_cnt = 0;
    bool in_error = false;
    std::string errmsg;

    args.push_back("ral_action=" + action);
    auto cb = [&line_cnt,&in_error,&errmsg,&entry_callback](std::string &line) {
      line_cnt +=1;
      if (line_cnt == 1) {
        if (line == "# simple") {
          return true;
        } else {
          LOG_ERROR("invalid line: '%s'. Expected '# simple'", line);
          return false;
        }
      } else if (in_error) {
        if (line != "ral_eom") {
          errmsg += line;
          return true;
        } else {
          return false;
        }
      } else {
        size_t pos = line.find_first_of(":");
        if (pos == std::string::npos) {
          LOG_ERROR("invalid line: '%s'. Expected '<KEY>: <VALUE>' but couldn't find a ':'", line);
          return true;
        }
        auto key = line.substr(0, pos);
        auto value = line.substr(pos+1);
        boost::trim(value);
        if (key == "ral_error") {
          in_error = true;
          errmsg = value;
        } else {
          return entry_callback(key, value);
        }
        return true;
      }
    };
    auto result = leatherman::execution::each_line(_path, args, cb);
    if (! result) {
      LOG_ERROR("Something went wrong running %s ral_action=%s", _path, action);
    }
    return result;
  }
}
