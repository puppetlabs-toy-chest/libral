#include <libral/target/ssh.hpp>

#include <boost/filesystem.hpp>

#include <leatherman/execution/execution.hpp>

namespace exe = leatherman::execution;
namespace fs = boost::filesystem;
namespace aug = libral::augeas;

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

    std::stringstream buf;
    bool first=true;

    for (auto dir : data_dirs) {
      if (!first)
        buf << ":";
      first=false;
      buf << dir << "/lenses";
    }

    auto reader = [xfms, this](::augeas* aug) {
      for (auto& xfm : xfms) {
        auto res = read(xfm.second);
        if (res) {
          auto text = res.ok();
          auto in = "/in" + xfm.second;
          auto file = "/files" + xfm.second;

          if (text.back() != '\n') {
            text += "\n";
          }
          aug_set(aug, in.c_str(), text.c_str());
          aug_text_store(aug, xfm.first.c_str(), in.c_str(), file.c_str());
        }
      }
    };

    auto writer = [xfms, this](::augeas *aug) {
      for (auto& xfm : xfms) {
        auto in = "/in" + xfm.second;
        auto file = "/files" + xfm.second;
        auto out = "/out" + xfm.second;
        aug_text_retrieve(aug, xfm.first.c_str(), in.c_str(), file.c_str(),
                          out.c_str());
        const char *text_in, *text_out;
        aug_get(aug, in.c_str(), &text_in);
        aug_get(aug, out.c_str(), &text_out);
        if (text_in != NULL && text_out != NULL &&
            strcmp(text_in, text_out) != 0) {
          // The text has changed
          auto res = write(text_out, xfm.second);
          if (! res) {
            auto err = "/augeas" + file + "/error";
            aug_set(aug, err.c_str(), "write_failed");
            err += "/message";
            aug_set(aug, err.c_str(), res.err().detail.c_str());
          }
        }
      }
    };

    auto aug = aug::handle::make(buf.str(), reader, writer);
    err_ret( aug->load() );

    return aug;
  }

  bool ssh::executable(const std::string& file) {
    // We make commands executable when we upload them; the whole check is
    // probably a bit silly and we might just want to get rid of this
    // method.
    return true;
  }

  static const std::string sudo = "sudo";

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
    auto res = run_ssh({ "/bin/echo", token });
    if (res.success && res.output == token) {
      return result<void>();
    } else {
      return error(res.output + "\n" + res.error);
    }
  }

  ssh::~ssh() {
    if (!_keep && ! _tmpdir.empty()) {
      run_ssh({"rm", "-rf", _tmpdir});
    }
  }

  std::string ssh::which(const std::string& cmd) {
    auto res = run_ssh({ "which", cmd });
    if (res.success) {
      return res.output;
    } else {
      return "";
    }
  }

  result<std::string> ssh::upload(const std::string& cmd) {
    auto tmp = tmpdir();
    err_ret(tmp);

    auto path = (fs::path(tmp.ok()) / fs::path(cmd).filename()).native();
    auto res = run("scp", {"-pq", cmd, _target + ":" + path});
    if (! res.success) {
      return error(res.error);
    }
    res = run_ssh({ "chmod", "a+x", path });
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
    if (_sudo) {
      actual.push_back(sudo);
    }
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
    if (_sudo) {
      actual.push_back(sudo);
    }
    actual.push_back(cmd);
    actual.insert(actual.end(), args.begin(), args.end());
    return exe::each_line("ssh", actual, out_cb, err_cb);
  }

  command::result ssh::run_ssh(const std::vector<std::string>& args,
                               const std::string *stdin) {
    std::vector<std::string> actual;
    actual.push_back(_target);
    if (_sudo) {
      actual.push_back(sudo);
    }
    actual.insert(actual.end(), args.begin(), args.end());
    return run("ssh", actual, stdin);
  }

  command::result ssh::run(const std::string& file,
                           const std::vector<std::string>& args,
                           const std::string *stdin) {
    std::vector<std::string> actual = ssh_opts;
    actual.insert(actual.end(), args.begin(), args.end());

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

  result<std::string> ssh::read(const std::string& remote_path) {
    auto res = run_ssh({ "dd", "status=none", "if=" + remote_path });
    if (! res.success) {
      return error(res.output + "\n" + res.error);
    }

    return res.output;
  }

  result<void> ssh::write(const std::string& content,
                          const std::string& remote_path) {
    auto tmp = tmpdir();
    err_ret( tmp );

    auto tmppath =
      (fs::path(tmp.ok()) / fs::path(remote_path).filename()).native();

    std::vector<std::string> args;
    args.push_back(_target);
    if (_sudo) {
      args.push_back(sudo);
    }
    args.push_back("dd");
    args.push_back("status=none");
    args.push_back("of=" + tmppath);
    auto res = run("ssh", args, &content);

    if (! res.success) {
      return error(res.output + "\n" + res.error);
    }

    /* Even with all this cleverness, we will still destroy SELinux
     * context; but checking if the tools are there etc. is too annoying to
     * do that here. */
    std::string script =
      "r='" + remote_path + "'; t='" + tmppath + "';" +
      R"shell(
[ -f "$r" ] && ( chmod --reference="$r" "$t"; chown --quiet --reference="$r" "$t"; true);
mv "$t" "$r"
)shell";

    res = run_ssh({ "/bin/sh" }, &script);

    if (! res.success) {
      return error(res.error);
    }

    return result<void>();
  }

  result<std::string> ssh::tmpdir() {
    if (_tmpdir.empty()) {
      // Do not use sudo on this, we manage the tmpdir as the normal user
      auto res = run("ssh", { _target, "mktemp", "-t", "-d", "ralXXXXXX" });
      if (! res.success) {
        return error(res.error);
      }
      _tmpdir = res.output;
    }
    return _tmpdir;
  }
  }
}
