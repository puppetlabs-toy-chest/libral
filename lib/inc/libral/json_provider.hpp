#pragma once

#include "provider.hpp"

#include <yaml-cpp/yaml.h>

#include <leatherman/json_container/json_container.hpp>

namespace libral {
  /* A provider backed by an executable that follows the 'simple' calling
     convention */
  class json_provider : public provider {
  public:
    json_provider(const std::string& path, YAML::Node &node)
      : provider(), _path(path), _node(node) { };

    result<bool> suitable() override;

    result<std::vector<resource>>
    get(context& ctx, const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

    const std::string& source() const override { return _path; }
  protected:
    result<prov::spec> describe() override;
  private:
    result<void> set(context &ctx, const update &upd);

    result<leatherman::json_container::JsonContainer>
    run_action(const std::string& action,
               const leatherman::json_container::JsonContainer& json);

    bool contains_error(const leatherman::json_container::JsonContainer& json,
                        std::string& message,
                        std::string& kind);

    result<resource>
    resource_from_json(const leatherman::json_container::JsonContainer& json);

    std::string _path;
    YAML::Node _node;
  };
}
