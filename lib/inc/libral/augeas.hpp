#pragma once

#include <stdexcept>
#include <vector>

#include <augeas.h>

namespace aug {
  class error : public std::runtime_error {
  public:
    explicit error(std::string const& msg) : std::runtime_error(msg) {}
  };

  class handle;

  class node {
  public:
    node(handle& aug, const std::string path);
    const std::string& path() { return _path; }
    std::string operator[](const std::string& path) const;
  private:
    handle& _aug;
    const std::string _path;
  };

  class handle {
  public:
    handle(const char *root,
           const char *loadpath,
           unsigned int flags);

    /* Load all files matching the shell glob GLOB with LENS */
    void include(const std::string& lens, const std::string& glob);

    /* Load all the files we set up with INCLUDE */
    void load(void);

    std::vector<node> match(const std::string& pathx);

    /* Get the value associated with the node matching PATHX. If there is
       one, returns that value. If there is no node matching PATHX or the
       value is NULL, returns an empty string
    */
    std::string value(const std::string& pathx) const;

    /* Check _augeas for errors and throw an aug::error if there
       is one */
    void check_error() const;

  private:
    ::augeas* _augeas;
  };
}
