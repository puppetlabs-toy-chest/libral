#pragma once

#include <libral/emitter/emitter.hpp>

#include <stdint.h>

namespace libral {
  class puppet_emitter : public emitter {
  public:
    puppet_emitter();
    void print_set(const provider &prov,
               const result<std::pair<update, changes>>& rslt) override;

    void print_find(const provider &prov,
               const result<boost::optional<resource>> &resource) override;

    void print_list(const provider &prov,
               const result<std::vector<resource>>& resources) override;

    void
    print_providers(const std::vector<std::shared_ptr<provider>>& provs) override;

  private:
    void print_resource(const provider &prov, const resource &resource);
    void print_resource_attr(const std::string& name,
                             const value& v,
                             uint16_t maxlen);
  };
}
