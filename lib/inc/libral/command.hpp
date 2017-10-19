#pragma once

#include <vector>
#include <boost/optional.hpp>
#include <libral/result.hpp>

namespace libral {
  /** A convenience wrapper around leatherman::execution for running simple
      commands */
  class command {
  public:

    /**
     * Encapsulates return value from executing a process.
     */
    struct result {
      result(bool s, std::string o, std::string e, int ec)
        : success(s), output(move(o)), error(move(e)), exit_code(ec) {}

      /**
       * Whether or not the command succeeded, defaults to true.
       */
      bool success = true;
      /**
       * Output from stdout.
       */
      std::string output;
      /**
       * Output from stderr (if not redirected).
       */
      std::string error;
      /**
       * The process exit code, defaults to 0.
       */
      int exit_code = 0;
    };

    /* Run the command with the given args. If the command exits with a
       non-zero exit code, return an error result */
    libral::result<void> run(const std::vector<std::string>& args);

    result execute(const std::vector<std::string>& args);

    result execute(const std::vector<std::string>& args,
                   const std::string& stdin);

    const std::string& path() const { return _cmd; }

    /* Create a new command for running cmd, which will be looked up on the
       PATH */
    static boost::optional<command> create(const std::string& cmd);

    /* Create a new command backed by the script cmd. */
    static boost::optional<command> script(const std::string& cmd);

    bool each_line(std::vector<std::string> const& arguments,
                   std::function<bool(std::string&)> stdout_callback,
                   std::function<bool(std::string&)> stderr_callback = nullptr);

  private:
    /* Create a new command with absolute path cmd */
    command(const std::string& cmd) : _cmd(cmd) { };
    std::string _cmd;
  };
}
