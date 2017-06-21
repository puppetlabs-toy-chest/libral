#pragma once

#include <libral/emitter/emitter.hpp>

#include <leatherman/json_container/json_container.hpp>

namespace libral {
  class quiet_emitter : public emitter {
  public:
    void print_set(const provider &prov,
               const result<std::pair<update, changes>>& rslt) override { }

    void print_find(const provider &prov,
               const result<boost::optional<resource>> &resource) override { }

    void print_list(const provider &prov,
               const result<std::vector<resource>>& resources) override { }

    void print_providers(const std::vector<std::shared_ptr<provider>>& providers) override { }
  };
}
