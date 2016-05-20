#pragma once

#include <vector>
#include <memory>

#include "type.hpp"

namespace libral {
  class ral {
  public:
    ral();
    ral(const ral&);

    std::vector<std::unique_ptr<type>> types(void);
  };
}
