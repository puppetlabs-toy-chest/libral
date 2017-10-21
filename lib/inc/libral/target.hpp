#pragma once

#include <memory>

namespace libral {
  /**
   * Classes in this namespace represent different ways to access the
   * system that we want to manage. For an overview of the API, see
   * target::base.
   */
  namespace target {

  // Forward declarations to break include loop. All these classes are
  // declared in the corresponding files in the target/ subdirectory
  class base;

  using sptr = std::shared_ptr<base>;

  /**
   * Make a local target, i.e. one that accesses files and runs commands
   * through normal system calls
   */
  sptr make_local();

  /**
   * Make an ssh target, i.e. one that accesses files and commands on a
   * remote system through ssh and scp
   */
  sptr make_ssh(const std::string& target, bool sudo = false, bool keep = false);

  }
}
