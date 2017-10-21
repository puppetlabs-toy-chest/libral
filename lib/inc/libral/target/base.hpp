#pragma once

#include <memory>

#include <libral/result.hpp>
#include <libral/command.hpp>
#include <libral/augeas.hpp>

namespace libral {
  namespace target {
  class base : public std::enable_shared_from_this<base> {
  public:
    virtual ~base() = default;

    virtual result<void> connect() = 0;

    virtual libral::command::uptr command(const std::string& cmd) = 0;

    virtual libral::command::uptr script(const std::string& cmd) = 0;

    virtual result<std::shared_ptr<augeas::handle>>
    augeas(const std::vector<std::string>& data_dirs,
           const std::vector<std::pair<std::string, std::string>>& xfms) = 0;

    virtual bool executable(const std::string& file) = 0;

    virtual std::string which(const std::string& cmd) = 0;

    virtual result<std::string> upload(const std::string& cmd) = 0;

    virtual command::result execute(const std::string& cmd,
                                    const std::vector<std::string>& args,
                                    const std::string *stdin = nullptr) = 0;

    virtual bool each_line(const std::string& cmd,
                           std::vector<std::string> const& args,
                           std::function<bool(std::string&)> out_cb,
                           std::function<bool(std::string&)> err_cb) = 0;
  };
  }
}
