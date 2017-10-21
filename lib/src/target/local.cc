#include <libral/target/local.hpp>

#include <unistd.h>

#include <sstream>
#include <fstream>

#include <leatherman/execution/execution.hpp>

namespace exe = leatherman::execution;
namespace aug = libral::augeas;

namespace libral {
  namespace target {

  result<void> local::connect() {
    return result<void>();
  }

  libral::command::uptr local::command(const std::string& cmd) {
    auto abs_cmd = which(cmd);
    if (abs_cmd.empty()) {
      return std::unique_ptr<libral::command>();
    } else {
      auto raw = new libral::command(shared_from_this(), abs_cmd);
      return std::unique_ptr<libral::command>(raw);
    }
  }

  libral::command::uptr local::script(const std::string& cmd) {
    return command(cmd);
  }

  result<std::shared_ptr<augeas::handle>>
  local::augeas(const std::vector<std::string>& data_dirs,
                const std::vector<std::pair<std::string, std::string>>& xfms) {
    std::stringstream buf;
    bool first=true;

    for (auto dir : data_dirs) {
      if (!first)
        buf << ":";
      first=false;
      buf << dir << "/lenses";
    }

    auto aug = aug::handle::make(buf.str());
    for (auto& xfm : xfms) {
      err_ret( aug->include(xfm.first, xfm.second) );
    }
    err_ret( aug->load() );

    return aug;
  }

  bool local::executable(const std::string& file) {
    return access(file.c_str(), X_OK) == 0;
  }

  std::string local::which(const std::string& cmd) {
    return exe::which(cmd);
  }

  result<std::string> local::upload(const std::string& cmd) {
    return cmd;
  }

  static command::result as_command_result(const exe::result& res) {
    return command::result(res.success, res.output, res.error, res.exit_code);
  }

  command::result local::execute(const std::string& cmd,
                                 const std::vector<std::string>& args,
                                 const std::string *stdin) {
    if (stdin == nullptr) {
      auto res = exe::execute(cmd, args,
                              0, { exe::execution_options::trim_output,
                                  exe::execution_options::merge_environment });
      return as_command_result(res);
    } else {
      auto res = exe::execute(cmd, args, *stdin,
                              0, { exe::execution_options::trim_output,
                                  exe::execution_options::merge_environment });
      return as_command_result(res);
    }
  }

  bool local::each_line(const std::string& cmd,
                        std::vector<std::string> const& args,
                        std::function<bool(std::string&)> out_cb,
                        std::function<bool(std::string&)> err_cb) {
    return exe::each_line(cmd, args, out_cb, err_cb);
  }

  result<std::string> local::read(const std::string& remote_path) {
    std::ifstream file(remote_path);
    std::ostringstream buf;
    buf << file.rdbuf();
    return buf.str();
  }

  result<void> local::write(const std::string& content,
                            const std::string& remote_path) {
    std::ofstream file(remote_path);
    file << content;
    return result<void>();
  }

  }
}
