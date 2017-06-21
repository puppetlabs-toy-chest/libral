#pragma once

#include <libral/emitter/emitter.hpp>

#include <leatherman/json_container/json_container.hpp>

#include <string>

namespace libral {
  class json_emitter : public emitter {
  public:

    using json = leatherman::json_container::JsonContainer;

    std::string parse_set(const provider &prov,
               const result<std::pair<update, changes>>& rslt);

    std::string parse_find(const provider &prov,
               const result<boost::optional<resource>> &resource);

    std::string parse_list(const provider &prov,
               const result<std::vector<resource>>& resources);

    std::string
    parse_providers(const std::vector<std::shared_ptr<provider>>& provs);

    void print_set(const provider &prov,
               const result<std::pair<update, changes>>& rslt) override;

    void print_find(const provider &prov,
               const result<boost::optional<resource>> &resource) override;

    void print_list(const provider &prov,
               const result<std::vector<resource>>& resources) override;

    void
    print_providers(const std::vector<std::shared_ptr<provider>>& provs) override;

  private:
    json resource_to_json(const provider &prov, const resource &res);
    json json_meta(const provider &prov);
    /* Set KEY in JS by appropriately converting V */
    void json_set_value(json &js, const std::string& key, const value &v);
  };
}
