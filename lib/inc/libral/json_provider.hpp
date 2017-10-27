#pragma once

#include "provider.hpp"

#include <leatherman/json_container/json_container.hpp>

namespace libral {
  /* A provider backed by an executable that follows the 'simple' calling
     convention */
  class json_provider : public provider {
  public:
    using json_container = leatherman::json_container::JsonContainer;
    using json_keys = std::vector<leatherman::json_container::JsonContainerKey>;

    json_provider(command::uptr& cmd, prov::spec &spec)
      : provider(spec), _cmd(std::move(cmd)) { };

    result<std::vector<resource>>
    get(context& ctx, const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

    const std::string& source() const override { return _cmd->path(); }
  private:
    result<json_container>
    run_action(context& ctx,
               const std::string& action,
               const json_container& json);

    bool contains_error(const json_container& json,
                        std::string& message,
                        std::string& kind);

    result<resource>
    resource_from_json(const context& ctx, const json_container& json);

    result<boost::optional<value>>
    value_from_json(const std::string &name,
                    const json_container& json,
                    const json_keys& key);

    command::uptr _cmd;
  };
}
