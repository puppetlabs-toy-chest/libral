#pragma once

#include <memory>

namespace libral {
  namespace target {

  // Forward declarations to break include loop. All these classes are
  // declared in the corresponding files in the target/ subdirectory
  class base;

  using sptr = std::shared_ptr<base>;

  sptr make_local();

  sptr make_ssh(const std::string& target);

  }
}
