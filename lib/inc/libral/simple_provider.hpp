#pragma once

#include "provider.hpp"

#include <yaml-cpp/yaml.h>

namespace libral {
  /* A provider backed by an executable that follows the 'simple' calling
     convention */
  class simple_provider : public provider {
  public:
    simple_provider(command& cmd, YAML::Node &node)
      : provider(), _cmd(cmd), _node(node) { };

    result<std::vector<resource>>
    get(context &ctx,
        const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

    const std::string& source() const override { return _cmd.path(); }
  protected:
    result<prov::spec> describe(environment &env) override;
  private:
    result<std::vector<resource>> find(context& ctx, const std::string &name);
    result<std::vector<resource>> instances(context& ctx);

    result<bool>
    run_action(context& ctx, const std::string& action,
               std::function<result<bool>(std::string&, std::string&)> entry_cb,
               std::vector<std::string> args = {});

    command _cmd;
    YAML::Node _node;
  };
}
