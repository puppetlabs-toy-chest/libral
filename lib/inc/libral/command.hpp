#pragma once

#include <vector>
#include <memory>

#include <libral/result.hpp>
#include <libral/target.hpp>

namespace libral {
  /** A convenience wrapper around leatherman::execution for running simple
      commands */
  class command {
  public:

    using uptr = std::unique_ptr<command>;

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

    command(target::sptr tgt, const std::string& cmd, bool needs_upload=false)
      : _cmd(cmd), _tgt(tgt), _needs_upload(needs_upload) { }

    /* Run the command with the given args. If the command exits with a
       non-zero exit code, return an error result */
    libral::result<void> run(const std::vector<std::string>& args);

    result execute(const std::vector<std::string>& args);

    result execute(const std::vector<std::string>& args,
                   const std::string& stdin);

    const std::string& path() const { return _cmd; }

    bool executable();

    bool each_line(std::vector<std::string> const& arguments,
                   std::function<bool(std::string&)> stdout_callback,
                   std::function<bool(std::string&)> stderr_callback = nullptr);

  private:
    libral::result<void> upload();

    std::string  _cmd;
    target::sptr _tgt;
    bool         _needs_upload;
  };
}
