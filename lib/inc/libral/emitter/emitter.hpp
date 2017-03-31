#pragma once

#include <libral/type.hpp>
#include <libral/provider.hpp>

namespace libral {
  class emitter {
  public:
    virtual void print_set(const type &type,
                 const result<std::pair<update, changes>>& rslt) = 0;

    virtual void print_find(const type &type,
                 const result<boost::optional<resource>> &resource) = 0;

    virtual void print_list(const type &type,
                 const result<std::vector<resource>>& resources) = 0;

    virtual void print_types(const std::vector<std::unique_ptr<type>>& types) = 0;
  };
}
