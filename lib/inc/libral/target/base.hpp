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

    /**
     * Establishes the connection to the target system. This must be called
     * before any other methods in this class.
     */
    virtual result<void> connect() = 0;

    /**
     * Create a new command, i.e. something that can run an executable that
     * is already on the managed system.
     */
    virtual libral::command::uptr command(const std::string& cmd) = 0;

    /**
     * Create a new script, i.e. something that we need to upload from cmd
     * to the target system so that we can run it there as a command. This
     * method takes care of uploading and making the uploaded script
     * executable.
     */
    virtual libral::command::uptr script(const std::string& cmd) = 0;

    /**
     * Create an augeas instance that reads and writes files on the
     * target. The data_dirs are the directories on the local system where
     * we should look for lenses. The xfms tell us which lenses to use with
     * what files: for each transformation (pair), the first element names
     * a lens like Hosts.lns and the second is the absolute path to a
     * file. Note that globs in filenames are currently not supported.
     */
    virtual result<std::shared_ptr<augeas::handle>>
    augeas(const std::vector<std::pair<std::string, std::string>>& xfms) = 0;

    /**
     * Returns true if file is executable
     */
    virtual bool executable(const std::string& file) = 0;

    /**
     * Returns the absolute path to the command cmd. If the command does
     * not exist, returns an empty string.
     */
    virtual std::string which(const std::string& cmd) = 0;

    /**
     * Uploads the local file cmd to a temporary directory and makes it
     * executable there. Returns the absolute path to that file on the
     * target on success.
     */
    virtual result<std::string> upload(const std::string& cmd) = 0;

    /**
     * Executes the file cmd, passing the command line arguments args and,
     * optionally, *stdin on stdin. The file cmd must already exist on the
     * target and be executable.
     */
    virtual command::result execute(const std::string& cmd,
                                    const std::vector<std::string>& args,
                                    const std::string *stdin = nullptr) = 0;

    /**
     * Executes the file cmd, passing the command line arguments
     * args. Calls the callbacks out_cb and err_cb on each line of the
     * commands stdout and stderr.
     */
    virtual bool each_line(const std::string& cmd,
                           std::vector<std::string> const& args,
                           std::function<bool(std::string&)> out_cb,
                           std::function<bool(std::string&)> err_cb) = 0;

    /**
     * Reads a file from the absolute path remote_path on the target and
     * returns its contents.
     */
    virtual result<std::string> read(const std::string& remote_path) = 0;

    /**
     * Writes the file remote_path on that target, putting content into
     * it. Care is taken to preserve the ownership and permissions of
     * remote_path if we are overwriting an existing file.
     */
    virtual result<void> write(const std::string& content,
                               const std::string& remote_path) = 0;
  };
  }
}
