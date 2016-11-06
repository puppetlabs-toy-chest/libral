#include <libral/ral.hpp>

#include <libral/mount.hpp>
#include <libral/simple_provider.hpp>
#include <leatherman/file_util/directory.hpp>
#include <leatherman/execution/execution.hpp>
#include <leatherman/logging/logging.hpp>

#include <boost/nowide/iostream.hpp>

#include <yaml-cpp/yaml.h>

namespace exe = leatherman::execution;

namespace libral {
  ral::ral(const std::string& data_dir) : _data_dir(data_dir) { }

  std::vector<std::unique_ptr<type>> ral::types(void) {
    // @todo lutter 2016-05-10:
    //   Need more magic here: need to find and register all types
    std::vector<std::unique_ptr<type>> result;
    auto mount_prov = std::shared_ptr<provider>(new mount_provider(_data_dir));
    if (mount_prov->suitable()) {
      auto mount_type = new type("mount", mount_prov);
      result.push_back(std::unique_ptr<type>(mount_type));
    }

    // Find external providers
    std::vector<std::string> files;

    auto cb = [&files,&result](std::string const &path) {
      if (access(path.c_str(), X_OK) == 0) {
        auto res = exe::execute(path, { "ral_action=describe" },
                     0, { exe::execution_options::trim_output,
                          exe::execution_options::merge_environment,
                          exe::execution_options::redirect_stderr_to_stdout });
        if (res.success) {
          auto node = YAML::Load(res.output);
          auto meta = node["meta"];
          if (meta) {
            if (meta["invoke"].as<std::string>() == "simple") {
              auto sprov = new simple_provider(path, node);
              auto prov = std::shared_ptr<provider>(sprov);
              /* If there are errors in the metadata, tell us now */
              prov->prepare();
              auto type_name = meta["type"].as<std::string>();
              auto t = new type(type_name, prov);
              result.push_back(std::unique_ptr<type>(t));
              LOG_INFO("provider %s (simple) for %s loaded", path, type_name);
            } else {
              LOG_ERROR("provider %s uses unknown calling convention '%s'",
                        path, meta["invoke"].Scalar());
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
        }
        files.push_back(path);
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
