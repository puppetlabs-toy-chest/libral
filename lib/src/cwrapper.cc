#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>

#include <libral/cwrapper.hpp>
#include <libral/ral.hpp>
#include <libral/emitter/json_emitter.hpp>
#include <leatherman/locale/locale.hpp>

namespace lib = libral;
using namespace leatherman::locale;

/* TODO in GOLand:
 *
 * [x] getTypes()(string, error) --> string == MUST serialize vector as JSON array
 * [x] getAllResources(type) (string, error) --> string == JSON object serialized
 * [x] getResource(type, resource) (string, error) --> string == JSON object serialized
 * [ ] getSchema(type)(string, error) --> how to derive a JSON schema??? Use JsonSchema???
 *
 */

//
// Helpers
//

static void str_to_cstr(const std::string &s, char **cs) {
    *cs = new char [s.length()+1];
    std::strcpy (*cs, s.c_str());

    return;
}

//
// Public interface
//

uint8_t get_providers(char **result) {
    std::vector<std::string> data_dirs;
    auto ral = lib::ral::create(data_dirs);
    auto raw_types = ral->types();
    lib::json_emitter em {};
    auto types = em.parse_types(raw_types);
    str_to_cstr(types, result);

    return EXIT_SUCCESS;
}

uint8_t get_resources(char **result, char *type_name) {
    std::vector<std::string> data_dirs;
    auto ral = lib::ral::create(data_dirs);
    auto opt_type = ral->find_type(std::string(type_name));

    if (!opt_type) {
        std::string error_msg = _("Provider {1} not found", std::string(type_name));
        str_to_cstr(error_msg, result);
        return EXIT_FAILURE;
    }

    auto resource_instances = (*opt_type)->instances();
    lib::json_emitter em {};
    auto resources = em.parse_list(**opt_type, resource_instances);
    str_to_cstr(resources, result);

    return EXIT_SUCCESS;
}

uint8_t get_resource(char **result, char *type_name, char *resource_name) {
    std::vector<std::string> data_dirs;
    auto ral = lib::ral::create(data_dirs);
    auto opt_type = ral->find_type(std::string(type_name));

    if (!opt_type) {
        std::string error_msg = _("Provider {1} not found", std::string(type_name));
        str_to_cstr(error_msg, result);
        return EXIT_FAILURE;
    }

    auto inst = (*opt_type)->find(std::string(resource_name));
    lib::json_emitter em {};

    auto resource = em.parse_find(**opt_type, inst);
    str_to_cstr(resource, result);

    return EXIT_SUCCESS;
}

uint8_t set_resource(char **result, char *type_name, char *resource_name, int desired_attributes_c, char **desired_attributes) {
    std::vector<std::string> data_dirs;
    auto ral = lib::ral::create(data_dirs);
    auto opt_type = ral->find_type(std::string(type_name));

    if (!opt_type) {
        std::string error_msg = _("Provider {1} not found", std::string(type_name));
        str_to_cstr(error_msg, result);
        return EXIT_FAILURE;
    }

    if (desired_attributes_c < 1) {
        str_to_cstr("Number of desired attributes must be > 0", result);
        return EXIT_FAILURE;
    }
    
    std::vector<std::string> av(desired_attributes, desired_attributes + desired_attributes_c);

    auto& type = *opt_type;
    lib::resource should = type->prov().create(std::string(resource_name));

    for (const auto& arg : av) {
        auto found = arg.find("=");
        if (found != std::string::npos) {
            auto attr = arg.substr(0, found);
            auto value = type->parse(attr, arg.substr(found+1));
            if (value) {
                should[attr] = value.ok();
            } else {
                std::string error_msg = _("Failed to read attribute {1}, resource_name: {2}, error: {3}", attr, std::string(type_name), value.err().detail);
                str_to_cstr(error_msg, result);
                return EXIT_FAILURE;
            }
        }
    }

    lib::json_emitter em {};

    auto res = type->set(should);
    auto setres = em.parse_set(**opt_type, res);
    
    str_to_cstr(setres, result);
    return EXIT_SUCCESS;
}