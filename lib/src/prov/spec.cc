#include <libral/prov/spec.hpp>

#include <leatherman/logging/logging.hpp>

#include <libral/environment.hpp>
#include <libral/mruby.hpp>

using namespace leatherman::locale;

namespace libral { namespace prov {

  spec::spec(const std::string& name, const std::string& type,
             const std::string& desc, const std::string& invoke,
             bool suitable,
             attr_spec_map&& attr_specs)
    : _name(name), _type(type), _desc(desc), _invoke(invoke),
      _qname(make_qname(name, type)), _suitable(suitable),
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

  result<bool>
  read_suitable(const environment&env,
                mruby& mrb, mrb_value& prov_node,
                const std::string& prov_name);

  result<spec> spec::read(const environment &env,
                          const std::string &prov_name,
                          const std::string &yaml,
                          bool suitable) {
    auto mrb = mruby::open();
    if (mrb.is_err()) {
      return mrb.err();
    }

    struct RClass *yaml_class = mrb->module_get("YAML");
    auto hash = mrb->funcall(yaml_class, "load", yaml);
    if (! hash) {
      return error(_("failed to parse provider metadata: {1}",
                     hash.err().detail));
    }

    if (! mrb_hash_p(*hash)) {
       return error(_("metadata must be a map but isn't"));
    }

    auto prov_node = mrb->hash_get(*hash, "provider");
    if (mrb_nil_p(prov_node)) {
      return error(_("could not find 'provider' entry in YAML"));
    }

    auto invoke = mrb->hash_get_string(prov_node, "invoke", "(none)");

    auto type_node = mrb->hash_get(prov_node, "type");
    if (mrb_nil_p(type_node)) {
      return error(_("missing 'type' attribute"));
    }
    auto name = mrb->hash_get_string(prov_node, "name", prov_name);
    auto type = mrb->hash_get_string(prov_node, "type");
    auto desc = mrb->hash_get_string(prov_node, "desc");

    auto attrs_node = mrb->hash_get(prov_node, "attributes");
    if (mrb_nil_p(attrs_node)) {
      return error(_("could not find entry 'provider.attributes' in YAML"));
    }

    if (! mrb_hash_p(attrs_node)) {
      return error(_("expected 'provider.attributes' to be a hash but it's something else"));
    }

    attr_spec_map attr_specs;
    auto keys = mrb->hash_keys(attrs_node);
    for (int i=0; i < mrb->ary_len(keys); i++) {
      auto key = ary_elt(keys, i);
      auto value = mrb->hash_get(attrs_node, key);
      auto name = mrb->as_string(key);
      auto desc = mrb->hash_get_string(value, "desc",
                                       "(missing description)");
      auto type = mrb->hash_get_string(value, "type", "string");
      auto kind = mrb->hash_get_string(value, "kind", "rw");

      auto attr = attr::spec::create(name, desc, type, kind);
      if (!attr) {
        return attr.err();
      }
      attr_specs.insert(std::make_pair(name, *attr));
    }

    if (attr_specs.find("name") == attr_specs.end()) {
      return error(_("no attribute 'name' has been defined, but that is mandatory"));
    }

    if (mrb->has_exc()) {
      return mrb->exc_as_error();
    }

    if (suitable) {
      auto s = read_suitable(env, *mrb, prov_node, prov_name);
      err_ret( s );

      suitable = s.ok();
    }
    return spec(name, type, desc, invoke, suitable, std::move(attr_specs));
  }

  std::string
  spec::make_qname(const std::string& name, const std::string& type) {
    return type + "::" + name;
  }

  result<bool>
  read_suitable(const environment&env,
                mruby& mrb, mrb_value& prov_node,
                const std::string& prov_name) {
    static const std::string op_not = "not ";
    mrb_value suitable = mrb.hash_get(prov_node, "suitable");

    if (mrb_nil_p(suitable)) {
      return error(_("missing 'provider.suitable' entry"));
    } else if (mrb.bool_p(suitable)) {
      return mrb_bool(suitable);
    } else if (mrb_string_p(suitable)) {
      auto s = mrb.as_string(suitable, "false");
      if (s != "true" && s != "false") {
        return error(_("provider {1}: metadata 'suitable' must be either 'true' or 'false' but was '{2}'", prov_name, s));
      }
      return (s == "true");
    } else if (mrb_hash_p(suitable)) {
      auto cmds = mrb.hash_get(suitable, "commands");
      if (mrb_array_p(cmds)) {
        for (int i=0; i < mrb.ary_len(cmds); i++) {
          auto mrb_cmd = ary_elt(cmds, i);
          if (! mrb_string_p(mrb_cmd)) {
            return error(_("provider {1}: the entries in 'suitable.commands' must all be strings", prov_name));
          }
          auto cmd = mrb.as_string(mrb_cmd);
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
