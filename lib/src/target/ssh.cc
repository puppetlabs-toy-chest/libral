#include <libral/target/ssh.hpp>

#include <boost/filesystem.hpp>

#include <leatherman/execution/execution.hpp>

namespace exe = leatherman::execution;
namespace fs = boost::filesystem;

namespace libral {
  namespace target {

  libral::command::uptr ssh::command(const std::string& cmd) {
    auto abs_cmd = which(cmd);
    if (abs_cmd.empty()) {
      return libral::command::uptr();
    } else {
      auto raw = new libral::command(shared_from_this(), abs_cmd);
      return libral::command::uptr(raw);
    }
  }

  libral::command::uptr ssh::script(const std::string& cmd) {
    auto abs_cmd = upload(cmd);
    if (abs_cmd.is_ok()) {
      auto raw = new libral::command(shared_from_this(), abs_cmd.ok());
      return libral::command::uptr(raw);
    } else {
      return libral::command::uptr();
    }
  }

  result<std::shared_ptr<augeas::handle>>
  ssh::augeas(const std::vector<std::string>& data_dirs,
              const std::vector<std::pair<std::string, std::string>>& xfms) {
#if 0
    std::stringstream buf;
    bool first=true;

    for (auto dir : data_dirs()) {
      if (!first)
        buf << ":";
      first=false;
      buf << dir << "/lenses";
    }

    // FIXME: set augeas root to something where we won't hurt anything
    auto aug = aug::handle::make(buf.str(), AUG_NO_MODL_AUTOLOAD);
    for (auto& xfm : xfms) {
      auto text = conn->read(xfm.second);
      auto in = "/in" + xfm.second;
      aug.set(in, text);
      aug.store(xfm.first, in, "/files" + xfm.second);
    }
#endif
    // FIXME: need to make aug.save() run aug.retrieve() and conn.write()
    // Maybe also make aug.load() redirect reading files through conn
    return not_implemented_error();
  }

  bool ssh::executable(const std::string& file) {
    // We make commands executable when we upload them; the whole check is
    // probably a bit silly and we might just want to get rid of this
    // method.
    return true;
  }

  static const std::vector<std::string> ssh_opts = {
    "-o", "ControlMaster=auto",
    "-o", "ControlPersist=60s",
    "-o", "ControlPath=~/.ssh/cm-%r@%h:%p" };

  /*
    ControlMaster handling:
    start: ssh -f -M -S /socket _target sleep 3600
    check: ssh -O check -S /socket _target
    use: ssh -S /socket _target uptime
    stop: ssh -O stop -S /socket _target
   */

  result<void> ssh::connect() {
    static const std::string token = "inlikeflynn";
    auto res = run("ssh", { _target, "/bin/echo", token });
    if (res.success && res.output == token) {
      return result<void>();
    } else {
      return error(res.output + "\n" + res.error);
    }
  }

  ssh::~ssh() {
    if (! _tmpdir.empty()) {
      run("ssh", { _target, "rm", "-rf", _tmpdir});
    }
  }

  std::string ssh::which(const std::string& cmd) {
    auto res = run("ssh", { _target, "which", cmd });
    if (res.success) {
      return res.output;
    } else {
      return "";
    }
  }

  result<std::string> ssh::upload(const std::string& cmd) {
    if (_tmpdir.empty()) {
      auto res = run("ssh", { _target, "mktemp", "-t", "-d", "ralXXXXXX"});
      if (! res.success) {
        return error(res.error);
      }
      _tmpdir = res.output;
    }
    auto path = (fs::path(_tmpdir) / fs::path(cmd).filename()).native();
    auto res = run("scp", {"-pq", cmd, _target + ":" + path});
    if (! res.success) {
      return error(res.error);
    }
    res = run("ssh", {_target, "chmod", "a+x", path });
    if (! res.success) {
      return error(res.error);
    }
    return path;
  }

  command::result ssh::execute(const std::string& cmd,
                                      const std::vector<std::string>& args,
                                      const std::string *stdin) {
    std::vector<std::string> actual;
    actual.push_back(_target);
    actual.push_back(cmd);
    for (auto& arg : args) {
      actual.push_back(arg);
    }
    return run("ssh", actual, stdin);
  }

  bool ssh::each_line(const std::string& cmd,
                      std::vector<std::string> const& args,
                      std::function<bool(std::string&)> out_cb,
                      std::function<bool(std::string&)> err_cb) {
    std::vector<std::string> actual;
    actual.push_back(_target);
    actual.push_back(cmd);
    for (auto& arg : args) {
      actual.push_back(arg);
    }
    return exe::each_line("ssh", actual, out_cb, err_cb);
  }

  command::result ssh::run(const std::string& file,
                           const std::vector<std::string>& args,
                           const std::string *stdin) {
    std::vector<std::string> actual = ssh_opts;
    for (auto& arg : args) {
      actual.push_back(arg);
    }

    if (stdin == nullptr) {
      auto res = exe::execute(file, actual, 20,
                              { exe::execution_options::trim_output,
                                  exe::execution_options::merge_environment });
      return command::result(res.success, res.output, res.error, res.exit_code);
    } else {
      auto res = exe::execute(file, actual, *stdin, 20,
                         { exe::execution_options::trim_output,
                             exe::execution_options::merge_environment });
      return command::result(res.success, res.output, res.error, res.exit_code);
    }
  }
  }
}
