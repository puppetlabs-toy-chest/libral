#include <libral/cwrapper.hpp>
// TODO wrapper for the c binding, expose pure c functions
#include <libral/export.h>
#include <libral/libral.hpp>
#include <libral/ral.hpp>
#include <libral/host.hpp>
#include <libral/emitter/puppet_emitter.hpp>
#include <libral/emitter/json_emitter.hpp>
#include <string>
#include <vector>

namespace lib = libral;
using json = leatherman::json_container::JsonContainer;


typedef long long intgo;
typedef struct { char *p; intgo n; } _gostring_;

json json_meta(const libral::type &type) {
    json meta;
    meta.set<std::string>("type", type.type_name());
    meta.set<std::string>("provider", type.qname());
    return meta;
}

/* Set a json attribute from a value */
struct value_to_json_visitor : boost::static_visitor<>
{
    value_to_json_visitor(json &js, const std::string &key)
        : _js(js), _key(key){};

    void operator()(const boost::none_t &n) const
    {
        // FIXME: we really want JSON null here
        _js.set<std::string>(_key, "__none__");
    }

    void operator()(const bool &b) const
    {
        _js.set<bool>(_key, b);
    }

    void operator()(const std::string &s) const
    {
        _js.set<std::string>(_key, s);
    }

    void operator()(const libral::array &ary) const
    {
        _js.set<std::vector<std::string>>(_key, ary);
    }

  private:
    json &_js;
    const std::string &_key;
};

void json_set_value(json &js, const std::string &key,
                    const libral::value &v)
{
    boost::apply_visitor(value_to_json_visitor(js, key), v);
}

json resource_to_json(const libral::type &type, const libral::resource &res)
{
    json js;

    js.set<std::string>("name", res.name());
    for (const auto &a : res.attrs())
    {
        json_set_value(js, a.first, a.second);
    }
    js.set<json>("ral", json_meta(type));
    return js;
}

char* get_all(char* type_name_c)
{
    std::string type_name(type_name_c);
    std::vector<std::string> data_dirs;
    auto ral = lib::ral::create(data_dirs);
    std::unique_ptr<lib::emitter> emp;
    // Use JSON emitter
    emp = std::unique_ptr<lib::emitter>(new lib::json_emitter());
    auto opt_type = ral->find_type(type_name);
    auto &type = *opt_type;

    auto insts = type->instances();
    // lib::emitter &em = *emp;
    // em.print_list(*type, insts);

    json js;
    std::vector<json> list;
    for (const auto &inst : insts.ok())
    {
        list.push_back(resource_to_json(*type, inst));
    }
    js.set<std::vector<json>>("resources", list);

    std::string js_str = js.toString();

    std::cout << "Generated: " << js_str << std::endl;

    char * c_js_str = new char [js_str.length()+1];
    std::strcpy (c_js_str, js_str.c_str());

    return c_js_str;
}
