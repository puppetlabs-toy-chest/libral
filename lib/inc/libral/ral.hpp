#pragma once

#include <vector>
#include <memory>
#include <boost/optional.hpp>

#include <yaml-cpp/yaml.h>

#include <libral/result.hpp>
#include <libral/provider.hpp>
#include <libral/environment.hpp>

namespace libral {
  class ral : public std::enable_shared_from_this<ral> {
  public:
    std::vector<std::shared_ptr<provider>> providers(void);
    boost::optional<std::shared_ptr<provider>>
    find_provider(const std::string& name);

    /* Create an instance of the RAL */
    static std::shared_ptr<ral> create(std::vector<std::string> data_dirs);

    /* Connect to a remote (ssh) target. The target must be a string that
     * the ssh command understands, i.e. 'ssh $target uptime' needs to work
     */
    result<void> connect(const std::string& _target);

    const std::vector<std::string>& data_dirs() const { return _data_dirs; }

    boost::optional<std::string>
    find_in_data_dirs(const std::string& file) const;

  protected:
    friend class environment;

    ral(const std::vector<std::string>& data_dir);

    environment make_env();

    target::sptr target() const { return _target; }

    bool is_local() const { return _local; }

  private:
    bool init_provider(environment &env,
                       const std::string& name,
                       std::shared_ptr<provider>& prov);
    result<YAML::Node> parse_metadata(const std::string& path,
                                      const std::string& yaml) const;
    result<std::string> get_metadata(command& cmd, const std::string& path) const;
    result<std::string> run_describe(command& cmd) const;

    std::vector<std::string> _data_dirs;
    target::sptr _target;
    bool _local;
  };
}
