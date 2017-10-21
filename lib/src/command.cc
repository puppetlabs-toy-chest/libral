#include <libral/command.hpp>

#include <leatherman/execution/execution.hpp>
#include <leatherman/locale/locale.hpp>

using namespace leatherman::locale;
namespace exe = leatherman::execution;

namespace libral {
  bool command::executable() const {
    return access(_cmd.c_str(), X_OK) == 0;
  }

  result<void> command::run(const std::vector<std::string> &args) {
    static const lth_util::option_set<exe::execution_options> options = {
      exe::execution_options::trim_output,
      exe::execution_options::merge_environment };
    auto status = exe::execute(_cmd, args, 0, options);

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
    auto res = exe::execute(_cmd, args, 0, { exe::execution_options::trim_output,
                                exe::execution_options::merge_environment });
    return result(res.success, res.output, res.error, res.exit_code);
  }


  command::result command::execute(const std::vector<std::string>& args,
                 const std::string& stdin) {
    auto res = exe::execute(_cmd, args, stdin,
                            0, { exe::execution_options::trim_output,
                                exe::execution_options::merge_environment });
    return result(res.success, res.output, res.error, res.exit_code);
  }

  bool command::each_line(std::vector<std::string> const& args,
         std::function<bool(std::string&)> out_cb,
         std::function<bool(std::string&)> err_cb) {
    return leatherman::execution::each_line(_cmd, args, out_cb, err_cb);
  }

  boost::optional<command> command::create(const std::string& cmd) {
    auto abs_cmd = exe::which(cmd);
    if (abs_cmd.empty()) {
      return boost::none;
    } else {
      return command(abs_cmd);
    }
  }

  boost::optional<command> command::script(const std::string& cmd) {
    // For remote targets, we need to upload cmd to the target first
    return create(cmd);
  }
}
