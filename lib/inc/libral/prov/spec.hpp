#pragma once

#include <boost/optional.hpp>
#include <yaml-cpp/yaml.h>

#include <libral/attr/spec.hpp>

namespace libral {
  class environment;

  namespace prov {
  class spec {
  public:
    using attr_spec_map = std::map<std::string, attr::spec>;

    /**
     * Returns the attribute spec for attribute name or none if there is no
     * attribute of that name.
     */
    boost::optional<const attr::spec &> attr(const std::string& name) const;

    /**
     * Returns the name of the provider
     */
    const std::string& name() const { return _name; }

    /**
     * Returns the name of the provider's type
     */
    const std::string& type_name() const {return _type; }

    /**
     * Returns the full name (type and provider name) of the provider
     */
    const std::string& qname() const { return _qname; }

    /**
     * Returns the description of the provider
     */
    const std::string& desc() const {return _desc; }

    /**
     * Returns true if the provider is suitable, i.e., can be used
     * successfully on this system
     */
    bool suitable() const { return _suitable; }

    void suitable(bool s) { _suitable = s; }

    /**
     * Reads a provider specification from a parsed YAML representation
     *
     * @param name the provider name, used in error messages
     * @param node the parsed YAML document
     */
    static result<spec> read(const libral::environment& env,
                             const std::string& name,
                             const YAML::Node &node);

    /**
     * Reads a provider specification from a string that must contain valid YAML
     *
     * @param name the provider name, used in error messages
     * @param yaml the YAML text
     */
    static result<spec> read(const libral::environment &env,
                             const std::string& name,
                             const std::string &yaml);

    attr_spec_map::const_iterator attr_begin() const
      { return _attr_specs.cbegin(); }
    attr_spec_map::const_iterator attr_end() const
      { return _attr_specs.cend(); }

  private:
    spec(const std::string& name, const std::string& type,
         const std::string& desc, attr_spec_map&& attr_specs);
    std::string make_qname(const std::string& name, const std::string& type);

    static result<bool>
    read_suitable(const environment& env,
                  const YAML::Node& node, const std::string& prov_name);

    std::string   _name;
    std::string   _type;
    std::string   _desc;
    std::string   _qname;
    bool          _suitable;

    attr_spec_map _attr_specs;
  };
} }
