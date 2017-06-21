#pragma once

#include <libral/provider.hpp>

namespace libral {
  class emitter {
  public:
    virtual void print_set(const provider &prov,
                 const result<std::pair<update, changes>>& rslt) = 0;

    virtual void print_find(const provider &prov,
                 const result<boost::optional<resource>> &resource) = 0;

    virtual void print_list(const provider &prov,
                 const result<std::vector<resource>>& resources) = 0;

    virtual void
    print_providers(const std::vector<std::shared_ptr<provider>>& provs) = 0;
  };
}
