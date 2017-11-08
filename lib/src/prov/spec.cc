#include <libral/prov/spec.hpp>

#include <leatherman/logging/logging.hpp>

#include <libral/environment.hpp>

using namespace leatherman::locale;

namespace libral { namespace prov {

  spec::spec(const std::string& name, const std::string& type,
             const std::string& desc, attr_spec_map&& attr_specs)
    : _name(name), _type(type), _desc(desc),
      _qname(make_qname(name, type)),
      _attr_specs(std::move(attr_specs)) { };

  boost::optional<const attr::spec&>
  spec::attr(const std::string& name) const {
    auto it = _attr_specs.find(name);
    if (it == _attr_specs.end()) {
      return boost::none;
    } else {
      return it->second;
    }
  }

  result<spec> spec::read(const environment& env,
                          const std::string& prov_name,
                          const YAML::Node &node) {
    attr_spec_map attr_specs;

    auto prov_node = node["provider"];
    if (! prov_node) {
      return error(_("provider[{1}]: could not find 'provider' entry in YAML",
                     prov_name));
    }

    auto type_node = prov_node["type"];
    if (! type_node) {
      return error(_("provider[{1}]: missing 'type' attribute", prov_name));
    }
    auto name = prov_node["name"].as<std::string>(prov_name);
    auto type = prov_node["type"].as<std::string>();
    auto desc = prov_node["desc"].as<std::string>("");

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

    spec spec(name, type, desc, std::move(attr_specs));
    auto suitable = read_suitable(env, prov_node["suitable"], prov_name);
    err_ret(suitable);
    spec.suitable(*suitable);
    return spec;
  }

  result<spec> spec::read(const environment &env,
                          const std::string& name,
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
    return spec::read(env, name, node);
  }

  std::string
  spec::make_qname(const std::string& name, const std::string& type) {
    return type + "::" + name;
  }

  result<bool>
  spec::read_suitable(const environment&env,
                      const YAML::Node& node, const std::string& prov_name) {
    static const std::string op_not = "not ";

    if (! node.IsDefined()) {
      // Treat missing 'suitable:' entries as false
      return false;
    } else if (node.IsScalar()) {
      auto s = node.as<std::string>("false");
      if (s != "true" && s != "false") {
        return error(_("provider {1}: metadata 'suitable' must be either 'true' or 'false' but was '{2}'", prov_name, s));
      }
      return (s == "true");
    } else if (node.IsMap()) {
      auto cmds = node["commands"];
      if (cmds.IsSequence()) {
        for (auto it = cmds.begin(); it != cmds.end(); it++) {
          if (! it->IsScalar()) {
            return error(_("provider {1}: the entries in 'suitable.commands' must all be strings", prov_name));
          }
          auto cmd = it->as<std::string>();
          size_t pos = 0;
          if (cmd.find(op_not) == 0) {
            // check that command is not there
            pos = cmd.find_first_not_of(" \t\n", op_not.length());
            if (!env.which(cmd.substr(pos)).empty()) {
              return false;
            }
          } else {
            if (env.which(cmd).empty()) {
              return false;
            }
          }
        }
        return true;
      } else {
        return error(_("provider {1}: metadata 'suitable.commands' must be an array of strings", prov_name));
      }
    } else {
      return error(_("provider {1}: illegal format for 'suitable' metadata",
                     prov_name));
    }
  }
} }
