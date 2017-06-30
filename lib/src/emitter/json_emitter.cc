#include <libral/emitter/json_emitter.hpp>

#include <leatherman/logging/logging.hpp>
#include <leatherman/json_container/json_container.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace leatherman::logging;
using namespace leatherman::locale;

namespace libral {

  using json = leatherman::json_container::JsonContainer;

  std::string json_emitter::parse_set(const type &type,
                       const result<std::pair<update, changes>>& rslt) {
    json js;

    if (!rslt) {
      json err;
      err.set<std::string>("message", _("failed: {1}", rslt.err().detail));
      js.set<json>("error", err);
    } else {
      const auto rsrc = type.prov().create(rslt->first, rslt->second);
      js.set<json>("resource", resource_to_json(type, rsrc));
      const auto &changes = rslt->second;
      std::vector<json> json_changes;
      for (auto ch : changes) {
        json json_ch;
        json_ch.set<std::string>("attr", ch.attr);
        json_set_value(json_ch, "is", ch.is);
        json_set_value(json_ch, "was", ch.was);
        json_changes.push_back(json_ch);
      }
      js.set<std::vector<json>>("changes", json_changes);
    }
    return js.toString();
  }

  std::string json_emitter::parse_find(const type &type,
                       const result<boost::optional<resource>> &inst) {
    json js;

    if (!inst) {
      json err;
      err.set<std::string>("message", _("failed: {1}", inst.err().detail));
      js.set<json>("error", err);
    } else if (inst.ok()) {
      js.set<json>("resource", resource_to_json(type, *inst.ok()));
    }
    return js.toString();
  }

  std::string json_emitter::parse_list(const type &type,
                       const result<std::vector<resource>>& rslt) {
    json js;

    if (!rslt) {
      json err;
      err.set<std::string>("message", _("failed: {1}", rslt.err().detail));
      js.set<json>("error", err);
    } else {
      std::vector<json> list;
      for (const auto& inst : rslt.ok()) {
        list.push_back(resource_to_json(type, inst));
      }
      js.set<std::vector<json>>("resources", list);
    }
    return js.toString();
  }

  std::string json_emitter::parse_types(const std::vector<std::unique_ptr<type>>& types) {
    json js;
    std::vector<json> list;
    for (const auto& t : types) {
      json entry;
      entry.set<std::string>("name", t->qname());
      entry.set<std::string>("type", t->type_name());
      entry.set<std::string>("source", t->prov().source());

      const auto& spec = t->prov().spec();
      if (spec) {
        entry.set<std::string>("desc", spec->desc());
        entry.set<bool>("suitable", spec->suitable());
        std::vector<json> attributes;
        for (auto it = spec->attr_begin(); it != spec->attr_end(); it++) {
          const auto &a = it->second;
          json attr;

          attr.set<std::string>("name", a.name());
          attr.set<std::string>("desc", a.desc());

          std::ostringstream os;
          os << a.data_type();
          attr.set<std::string>("type", os.str());
          os.str("");
          os << a.kind();
          attr.set<std::string>("kind", os.str());
          attributes.push_back(attr);
        }
        entry.set<std::vector<json>>("attributes", attributes);
      }

      list.push_back(entry);
    }
    js.set<std::vector<json>>("providers", list);
    return js.toString();
  }

  void json_emitter::print_set(const type &type,
               const result<std::pair<update, changes>>& rslt) {
    auto js_s = parse_set(type, rslt);
    std::cout << js_s << std::endl;
  }

  void json_emitter::print_find(const type &type,
               const result<boost::optional<resource>> &resource) {
    auto js_s = parse_find(type, resource);
    std::cout << js_s << std::endl;
  }

  void json_emitter::print_list(const type &type,
               const result<std::vector<resource>>& resources) {
    auto js_s = parse_list(type, resources);
    std::cout << js_s << std::endl;
  }

  void json_emitter::print_types(const std::vector<std::unique_ptr<type>>& types) {
    auto js_s = parse_types(types);
    std::cout << js_s << std::endl;
  }

  json json_emitter::resource_to_json(const type &type, const resource &res) {
    json js;

    js.set<std::string>("name", res.name());
    for (const auto& a : res.attrs()) {
      json_set_value(js, a.first, a.second);
    }
    js.set<json>("ral", json_meta(type));
    return js;
  }

  json json_emitter::json_meta(const type &type) {
      json meta;
      meta.set<std::string>("type", type.type_name());
      meta.set<std::string>("provider", type.qname());
      return meta;
  }

  /* Set a json attribute from a value */
  struct value_to_json_visitor : boost::static_visitor<> {
    value_to_json_visitor(json &js, const std::string &key)
      : _js(js), _key(key) { };

    void operator()(const boost::none_t& n) const {
      // FIXME: we really want JSON null here
      _js.set<std::string>(_key, "__none__");
    }

    void operator()(const bool& b) const {
      _js.set<bool>(_key, b);
    }

    void operator()(const std::string& s) const {
      _js.set<std::string>(_key, s);
    }

    void operator()(const array& ary) const {
      _js.set<std::vector<std::string>>(_key, ary);
    }

  private:
    json& _js;
    const std::string& _key;
  };

  void json_emitter::json_set_value(json &js, const std::string& key,
                                    const value &v) {
    boost::apply_visitor(value_to_json_visitor(js, key), v);
  }

}
