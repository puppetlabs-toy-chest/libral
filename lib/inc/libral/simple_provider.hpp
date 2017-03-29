#pragma once

#include "provider.hpp"

#include <yaml-cpp/yaml.h>

namespace libral {
  /* A provider backed by an executable that follows the 'simple' calling
     convention */
  class simple_provider : public provider {
  public:
    simple_provider(const std::string& path, YAML::Node &node)
      : provider(), _path(path), _node(node) { };

    result<std::vector<resource>>
    get(context &ctx,
        const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

    const std::string& source() const override { return _path; }
  protected:
    result<prov::spec> describe(environment &env) override;
  private:
    result<std::vector<resource>> find(const std::string &name);
    result<std::vector<resource>> instances();

    result<bool>
    run_action(const std::string& action,
               std::function<result<bool>(std::string&, std::string&)> entry_cb,
               std::vector<std::string> args = {});

    std::string _path;
    YAML::Node _node;
  };
}
