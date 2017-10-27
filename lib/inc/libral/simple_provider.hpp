#pragma once

#include "provider.hpp"

namespace libral {
  /* A provider backed by an executable that follows the 'simple' calling
     convention */
  class simple_provider : public provider {
  public:
    simple_provider(command::uptr& cmd, prov::spec& spec)
      : provider(spec), _cmd(std::move(cmd)) { };

    result<std::vector<resource>>
    get(context &ctx,
        const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

    const std::string& source() const override { return _cmd->path(); }
  private:
    result<std::vector<resource>> find(context& ctx, const std::string &name);
    result<std::vector<resource>> instances(context& ctx);

    result<bool>
    run_action(context& ctx, const std::string& action,
               std::function<result<bool>(std::string&, std::string&)> entry_cb,
               std::vector<std::string> args = {});

    command::uptr _cmd;
  };
}
