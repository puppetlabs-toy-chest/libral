#pragma once

#include <stdexcept>
#include <vector>
#include <memory>
#include <boost/optional.hpp>

#include <augeas.h>

namespace aug {
  class error : public std::runtime_error {
  public:
    explicit error(std::string const& msg) : std::runtime_error(msg) {}
  };

  class handle;

  class node {
  public:
    node(std::shared_ptr<handle> aug, const std::string path);
    const std::string& path() const { return _path; }
    boost::optional<std::string> operator[](const std::string& path) const;

    /* Set the value of the child PATH to VALUE */
    void set(const std::string& path, const std::string& value);
    /* Set the value of child PATH to VALUE if it is a string; if VALUE is
       boost::none, set it to DEFLT */
    void set(const std::string& path,
             const boost::optional<std::string>& value,
             const std::string& deflt);
    /* Set the value of child PATH to VALUE if it is a string; do not do
       anything if VALUE is boost::none */
    void set_maybe(const std::string& path,
                   const boost::optional<std::string>& value);

    /* Delete this node and all its children */
    void rm();
    /* Delete the children of this node, but leave the node alone */
    void erase();
    /* Set the value for this node to NULL */
    void clear();
  private:
    std::string append(const std::string& p2) const;

    std::shared_ptr<handle> _aug;
    const std::string _path;
  };

  class handle : public std::enable_shared_from_this<handle> {
  public:
    handle(const std::string& loadpath, unsigned int flags);

    /* Load all files matching the shell glob GLOB with LENS */
    void include(const std::string& lens, const std::string& glob);

    /* Load all the files we set up with INCLUDE */
    void load(void);

    /* Save changes back to disk */
    void save(void);

    std::vector<node> match(const std::string& pathx);

    /* Set the value of the node at PATH to VALUE. As a sideeffect, might
       create the node */
    void set(const std::string& path, const std::string& value);

    /* Set the value of the node at PATH to NULL. As a sideeffect, might
       create the node */
    void clear(const std::string& path);

    void rm(const std::string& path);

    /* Get the value associated with the node matching PATHX. If there is
       one, returns that value. If there is no node matching PATHX or the
       value is NULL, returns an empty string
    */
    boost::optional<std::string> value(const std::string& pathx) const;

    /* Make a new node representing PATH in the tree. This by itself does
       not cause any changes to the tree. In particular, it does not create
       the node at PATH if it doesn't exist yet. */
    node make_node(const std::string& path);

    /* Check _augeas for errors and throw an aug::error if there
       is one */
    void check_error() const;

  private:
    ::augeas* _augeas;
    /* This is a kludge around the requirement that for shared_from_this to
       work a shared_ptr to this must already exist. See
       http://en.cppreference.com/w/cpp/memory/enable_shared_from_this/shared_from_this
    */
    std::shared_ptr<handle> _shared_this;
  };
}
