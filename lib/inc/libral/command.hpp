#pragma once

#include <vector>
#include <boost/optional.hpp>
#include <libral/result.hpp>

namespace libral {
  /** A convenience wrapper around leatherman::execution for running simple
      commands */
  class command {
  public:
    /* Run the command with the given args. If the command exits with a
       non-zero exit code, return an error result */
    result<bool> run(const std::vector<std::string>& args);

    /* Create a new command for running cmd, which will be looked up on the
       PATH */
    static boost::optional<command> create(const std::string& cmd);
  private:
    /* Create a new command with absolute path cmd */
    command(const std::string& cmd) : _cmd(cmd) { };
    std::string _cmd;
  };
}
