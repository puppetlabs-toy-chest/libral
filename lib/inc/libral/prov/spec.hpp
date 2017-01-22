#pragma once

#include <boost/optional.hpp>
#include <yaml-cpp/yaml.h>

#include <libral/attr/spec.hpp>

namespace libral { namespace prov {
  class spec {
  public:
    using attr_spec_map = std::map<std::string, attr::spec>;

    /**
     * Returns the attribute spec for attribute name or none if there is no
     * attribute of that name.
     */
    boost::optional<const attr::spec &> attr(const std::string& name) const;

    /**
     * Reads a provider specification from a parsed YAML representation
     *
     * @param name the provider name, used in error messages
     * @param node the parsed YAML document
     */
    static result<spec> read(const std::string& name, const YAML::Node &node);

    /**
     * Reads a provider specification from a string that must contain valid YAML
     *
     * @param name the provider name, used in error messages
     * @param yaml the YAML text
     */
    static result<spec> read(const std::string& name, const std::string &yaml);

    attr_spec_map::const_iterator attr_begin() const
      { return _attr_specs.cbegin(); }
    attr_spec_map::const_iterator attr_end() const
      { return _attr_specs.cend(); }

  private:
    spec(attr_spec_map&& attr_specs) : _attr_specs(std::move(attr_specs)) { };
    attr_spec_map _attr_specs;
  };
} }
