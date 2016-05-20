#pragma once

#include <vector>
#include <memory>
#include <boost/optional.hpp>

#include "type.hpp"

namespace libral {
  class ral {
  public:
    ral();
    ral(const ral&);

    std::vector<std::unique_ptr<type>> types(void);
    boost::optional<std::unique_ptr<type>> find_type(const std::string& name);
  };
}
