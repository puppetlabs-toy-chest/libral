#include <libral/command.hpp>

#include <leatherman/execution/execution.hpp>
#include <leatherman/locale/locale.hpp>

using namespace leatherman::locale;
namespace exe = leatherman::execution;

namespace libral {
  result<void> command::run(const std::vector<std::string> &args) {
    static const lth_util::option_set<exe::execution_options> options = {
      exe::execution_options::trim_output,
      exe::execution_options::merge_environment };
    auto status = exe::execute(_cmd, args, 0, options);

    if (status.success)
      return result<void>();

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

  boost::optional<command> command::create(const std::string& cmd) {
    auto abs_cmd = exe::which(cmd);
    if (abs_cmd.empty()) {
      return boost::none;
    } else {
      return command(abs_cmd);
    }
  }
}
