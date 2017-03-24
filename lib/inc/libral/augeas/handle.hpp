#pragma once

#include <vector>
#include <memory>
#include <string>
#include <boost/optional.hpp>

#include <augeas.h>

namespace libral { namespace augeas {

  class node;

  class handle : public std::enable_shared_from_this<handle> {
  public:
    ~handle() { aug_close(_augeas); }

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

    static std::shared_ptr<handle> make(const std::string& loadpath,
                                        unsigned int flags) {
      return std::shared_ptr<handle>(new handle(loadpath, flags));
    }

  private:
    handle(const std::string& loadpath, unsigned int flags)
      : _augeas(aug_init(NULL, loadpath.c_str(), flags)) { };

    ::augeas* _augeas;
  };

  } }
