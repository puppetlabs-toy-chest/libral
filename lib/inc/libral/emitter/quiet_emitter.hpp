#pragma once

#include <libral/emitter/emitter.hpp>

#include <leatherman/json_container/json_container.hpp>

namespace libral {
  class quiet_emitter : public emitter {
  public:
    void print_set(const type &type,
               const result<std::pair<update, changes>>& rslt) override { }

    void print_find(const type &type,
               const result<boost::optional<resource>> &resource) override { }

    void print_list(const type &type,
               const result<std::vector<resource>>& resources) override { }

    void print_types(const std::vector<std::unique_ptr<type>>& types) override { }
  };
}
