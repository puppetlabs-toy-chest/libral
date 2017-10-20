#include <libral/command.hpp>

#include <libral/target/base.hpp>

namespace libral {

  bool command::executable() const {
    return _tgt->executable(_cmd);
  }

  result<void> command::run(const std::vector<std::string> &args) {
    auto status = _tgt->execute(_cmd, args);

    if (status.success)
      return libral::result<void>();

    if (status.output.empty()) {
      if (status.error.empty()) {
        return error(_("Command {1} failed with status code {2}.",
                       _cmd, status.exit_code));
      } else {
        return error(_("Command {1} failed with status code {2}. It printed on stderr '{3}'", _cmd, status.exit_code, status.error));
      }
    } else {
      if (status.error.empty()) {
        return error(_("Command {1} failed with status code {2}. It printed on stdout '{3}'", _cmd, status.exit_code, status.output));
      } else {
        return error(_("Command {1} failed with status code {2}. It printed on stdout '{3}' and on stderr '{4}'", _cmd, status.exit_code, status.output, status.error));
      }
    }
  }

  command::result command::execute(const std::vector<std::string>& args) {
    return _tgt->execute(_cmd, args);
  }


  command::result command::execute(const std::vector<std::string>& args,
                                         const std::string& stdin) {
    return _tgt->execute(_cmd, args, &stdin);
  }

  bool command::each_line(std::vector<std::string> const& args,
         std::function<bool(std::string&)> out_cb,
         std::function<bool(std::string&)> err_cb) {
    return _tgt->each_line(_cmd, args, out_cb, err_cb);
  }
}
