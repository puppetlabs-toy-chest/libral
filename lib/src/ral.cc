#include <libral/ral.hpp>

#include <libral/mount.hpp>
#include <libral/user.hpp>
#include <libral/file.hpp>

#include <libral/simple_provider.hpp>
#include <leatherman/file_util/directory.hpp>
#include <leatherman/execution/execution.hpp>
#include <leatherman/logging/logging.hpp>

#include <boost/nowide/iostream.hpp>

#include <yaml-cpp/yaml.h>

namespace exe = leatherman::execution;

namespace libral {
  ral::ral(const std::string& data_dir) : _data_dir(data_dir) { }

  bool ral::add_type(std::vector<std::unique_ptr<type>>& types,
                     const std::string& name, std::shared_ptr<provider> prov) {
    auto res = prov->suitable();
    if (res.is_ok()) {
      if (*res.ok()) {
        auto t = new type(name, prov);
        types.push_back(std::move(std::unique_ptr<type>(t)));
        return true;
      } else {
        LOG_INFO("provider for {1} is not suitable", name);
      }
    } else {
      LOG_WARNING("failed to prepare provider for {1}: {2}", name, res.err()->detail);
    }
    return false;
  }

  std::vector<std::unique_ptr<type>> ral::types(void) {
    // @todo lutter 2016-05-10:
    //   Need more magic here: need to find and register all types
    std::vector<std::unique_ptr<type>> result;
    auto mount_prov = std::shared_ptr<provider>(new mount_provider(_data_dir));
    add_type(result, "mount", mount_prov);

    auto user_prov = std::shared_ptr<provider>(new user_provider(_data_dir));
    add_type(result, "user", user_prov);

    auto file_prov = std::shared_ptr<provider>(new file_provider(_data_dir));
    add_type(result, "file", file_prov);

    // Find external providers
    std::vector<std::string> files;

    auto cb = [&files,&result,this](std::string const &path) {
      if (access(path.c_str(), X_OK) == 0) {
        auto res = exe::execute(path, { "ral_action=describe" },
                     0, { exe::execution_options::trim_output,
                          exe::execution_options::merge_environment });
        if (res.success && res.error.empty()) {
          YAML::Node node;
          try {
            node = YAML::Load(res.output);
          } catch (YAML::Exception& e) {
            LOG_ERROR("describing provider {1} produced invalid yaml: {2}",
                      path, e.what());
            return true;
          }
          if (!node) {
            LOG_ERROR("describing provider {1} produced invalid yaml", path);
            return true;
          }
          if (!node.IsMap()) {
            LOG_ERROR("describing provider {1} did not produce a map", path);
            return true;
          }
          auto meta = node["meta"];
          if (meta) {
            auto invoke = meta["invoke"].as<std::string>("(none)");
            if (invoke == "simple") {
              auto sprov = new simple_provider(path, node);
              auto prov = std::shared_ptr<provider>(sprov);
              auto type_name = meta["type"].as<std::string>();
              if (add_type(result, type_name, prov)) {
                LOG_INFO("provider %s (simple) for %s loaded", path, type_name);
              }
            } else if (invoke == "(none)") {
              LOG_ERROR("provider {1} does not specify a calling convention under 'meta.invoke'",
                        path);
            } else {
              LOG_ERROR("provider {1} uses unknown calling convention '{2}'",
                        path, invoke);
            }
          } else {
            LOG_ERROR("provider %s does not return proper metadata with 'ral_action=describe'", path);
          }
        } else {
          if (res.output.empty()) {
            LOG_WARNING("ignoring provider %s as it exited with status %d", path, res.exit_code);
          } else {
            LOG_WARNING("ignoring provider %s as it exited with status %d. Output was '%s'",
                      path, res.exit_code, res.output);
          }
          if (! res.error.empty()) {
            LOG_WARNING("provider produced stderr '%s'", res.error);
          }
        }
        files.push_back(path);
      } else {
        LOG_WARNING("file %s looks like a provider but is not executable", path);
      }
      return true;
    };

    leatherman::file_util::each_file(_data_dir + "/providers", cb , "\\.prov$");
    return result;
  }

  boost::optional<std::unique_ptr<type>> ral::find_type(const std::string& name) {
    auto types_vec = types();
    auto type_it = std::find_if(types_vec.begin(), types_vec.end(),
                                [name](std::unique_ptr<type> &t)
                                { return name == t->name(); });
    if (type_it == types_vec.end()) {
      return boost::none;
    } else {
      return std::move(*type_it);
    }
  }
}
