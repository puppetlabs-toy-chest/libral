#pragma once

#include "provider.hpp"

#include <yaml-cpp/yaml.h>

namespace libral {
  /* A provider backed by an executable that follows the 'simple' calling
     convention */
  class simple_provider : public provider {
  public:

    class simple_resource : public resource {
    public:
      simple_resource(std::shared_ptr<simple_provider>& prov,
                      const std::string& name)
        : resource(name), _prov(prov) { }

      result<changes> update(const attr_map &should) override;
    private:
      std::shared_ptr<simple_provider> _prov;
      attr_map                         _attrs;
    };

    simple_provider(const std::string& path, YAML::Node &node)
      : provider(), _path(path), _node(node) { };

    result<bool> suitable() override;
    void flush() override;
    std::unique_ptr<resource> create(const std::string& name) override;
    result<boost::optional<resource_uptr>> find(const std::string &name) override;
    result<std::vector<resource_uptr>> instances() override;

    const std::string& source() const override { return _path; }
  protected:
    result<prov::spec> describe() override;
  private:
    result<bool>
    run_action(const std::string& action,
               std::function<result<bool>(std::string&, std::string&)> entry_cb,
               std::vector<std::string> args = {});

    std::string _path;
    YAML::Node _node;
  };
}
