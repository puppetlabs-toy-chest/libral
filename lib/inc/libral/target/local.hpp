#pragma once

#include <libral/target/base.hpp>

namespace libral {
  namespace target {
  class local : public base {
  public:
    result<void> connect() override;

    libral::command::uptr command(const std::string& cmd) override;

    libral::command::uptr script(const std::string& cmd) override;

    result<std::shared_ptr<augeas::handle>>
    augeas(const std::vector<std::pair<std::string, std::string>>& xfms) override;

    bool executable(const std::string& file) override;

    std::string which(const std::string& cmd) override;

    result<std::string> upload(const std::string& cmd) override;

    command::result execute(const std::string& cmd,
                            const std::vector<std::string>& args,
                            const std::string *stdin = nullptr) override;

    bool each_line(const std::string& cmd,
                   std::vector<std::string> const& args,
                   std::function<bool(std::string&)> out_cb,
                   std::function<bool(std::string&)> err_cb) override;

    result<std::string> read(const std::string& remote_path) override;

    result<void> write(const std::string& content,
                       const std::string& remote_path) override;

  };
  }
}
