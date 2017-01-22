#include <libral/prov/spec.hpp>

#include <leatherman/logging/logging.hpp>

using namespace leatherman::locale;

namespace libral { namespace prov {

  boost::optional<const attr::spec&>
  spec::attr(const std::string& name) const {
    auto it = _attr_specs.find(name);
    if (it == _attr_specs.end()) {
      return boost::none;
    } else {
      return it->second;
    }
  }

  result<spec> spec::read(const std::string& prov_name,
                          const YAML::Node &node) {
    attr_spec_map attr_specs;

    auto prov_node = node["provider"];
    if (! prov_node) {
      return error(_("provider[{1}]: could not find 'provider' entry in YAML",
                     prov_name));
    }

    auto attrs_node = prov_node["attributes"];
    if (! attrs_node) {
      return error(_("provider[{1}]: could not find entry 'provider.attributes' in YAML", prov_name));
    }

    if (! attrs_node.IsMap()) {
      // FIXME: would like to say what it is, but there's no simple way to
      // turn the YAML::NodeType into a name
      return error(_("provider[{1}]: expected 'provider.attributes' to be a hash but it's something else", prov_name));
    }

    for (auto it : attrs_node) {
      auto name = it.first.as<std::string>();
      auto spec_node = it.second;
      auto desc = spec_node["desc"].as<std::string>("(missing description)");
      auto type = spec_node["type"].as<std::string>("string");
      auto kind = spec_node["kind"].as<std::string>("rw");

      auto attr = attr::spec::create(name, desc, type, kind);
      if (!attr) {
        return attr.err();
      }
      attr_specs.insert(std::make_pair(name, *attr));
    }

    if (attr_specs.find("name") == attr_specs.end()) {
      return error(_("provider[{1}]: no attribute 'name' has been defined, but that is mandatory", prov_name));
    }
    return spec(std::move(attr_specs));
  }

  result<spec> spec::read(const std::string& name,
                          const std::string &yaml) {
    YAML::Node node;
    try {
      node = YAML::Load(yaml);
    } catch (YAML::Exception& e) {
      return error(
        _("provider[{1}]: metadata is not valid yaml: {2}",
          name, e.what()));
    }
    if (!node) {
      return error(_("provider[{1}]: metadata is not valid yaml",
                     name));
    }
    if (!node.IsMap()) {
      return error(_("provider[{1}]: metadata must be a map but isn't",
                     name));
    }
    return spec::read(name, node);
  }

} }
