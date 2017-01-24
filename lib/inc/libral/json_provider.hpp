#pragma once

#include "provider.hpp"

#include <yaml-cpp/yaml.h>

#include <leatherman/json_container/json_container.hpp>

namespace libral {
  /* A provider backed by an executable that follows the 'simple' calling
     convention */
  class json_provider : public provider {
  public:

    class json_resource : public resource {
    public:
      json_resource(std::shared_ptr<json_provider>& prov,
                      const std::string& name)
        : resource(name), _prov(prov) { }

      result<changes> update(const attr_map &should) override;
    private:
      std::shared_ptr<json_provider> _prov;
      attr_map                         _attrs;
    };

    json_provider(const std::string& path, YAML::Node &node)
      : provider(), _path(path), _node(node) { };

    result<bool> suitable();
    void flush();
    std::unique_ptr<resource> create(const std::string& name);
    boost::optional<std::unique_ptr<resource>> find(const std::string &name);
    std::vector<std::unique_ptr<resource>> instances();

    const std::string& source() const { return _path; }
  protected:
    result<prov::spec> describe() override;
  private:
    result<leatherman::json_container::JsonContainer>
    run_action(const std::string& action,
               const leatherman::json_container::JsonContainer& json);
    bool contains_error(const leatherman::json_container::JsonContainer& json,
                        std::string& message,
                        std::string& kind);
    result<std::unique_ptr<resource>>
    resource_from_json(const leatherman::json_container::JsonContainer& json);

    std::string _path;
    YAML::Node _node;
  };
}
