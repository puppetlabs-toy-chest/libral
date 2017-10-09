#pragma once

#include <vector>
#include <memory>
#include <string>
#include <boost/optional.hpp>

#include <augeas.h>

#include <libral/result.hpp>

namespace libral { namespace augeas {

  class node;

  class handle : public std::enable_shared_from_this<handle> {
  public:
    ~handle() { aug_close(_augeas); }

    /**
     * Marks files matching the shell glob \p glob for processing with \p lens.
     * Matching files will only be loaded after calling load() */
    result<void>
    include(const std::string& lens, const std::string& glob);

    /* Load all the files we set up with INCLUDE */
    result<void> load(void);

    /* Save changes back to disk */
    result<void> save(void);

    /**
     * Matches the given path expression against the tree and returns all
     * the nodes matching it. */
    result<std::vector<node>>
    match(const std::string& pathx);

    /**
     * Set the value of the node at PATH to VALUE. As a sideeffect, might
     * create the node */
    result<void>
    set(const std::string& path, const std::string& value);

    /* Set the value of the node at PATH to NULL. As a sideeffect, might
       create the node */
    result<void>
    clear(const std::string& path);

    result<void>
    rm(const std::string& path);

    /* Get the value associated with the node matching PATHX. If there is
       one, returns that value. If there is no node matching PATHX or the
       value is NULL, returns an empty string
    */
    result<boost::optional<std::string>>
    get(const std::string& pathx) const;

    /**
     * Makes a new node representing PATH in the tree. This by itself does
     * not cause any changes to the tree. In particular, it does not create
     * the node at PATH if it doesn't exist yet. */
    node make_node(const std::string& path);

    /**
     * Makes a new node underneath PATH with a unique sequence number. This
     * is useful for things like appending lines to /etc/hosts or
     * /etc/fstab where each line is numbered. This by itself does
     * not cause any changes to the tree. In particular, it does not create
     * the node at PATH if it doesn't exist yet.*/
    node make_node_seq_next(const std::string& path);

    static std::shared_ptr<handle> make(unsigned int flags) {
      return std::shared_ptr<handle>(new handle(flags));
    }

  private:
    handle(unsigned int flags)
      : _augeas(aug_init(NULL, NULL, flags)) { };

    /* Checks _augeas for errors and return an error if there is one. */
    result<void> check_error() const;

    ::augeas* _augeas;
    /* make_node_seq_next uses this to make sure we create unique sequence
       numbers for nodes that need it */
    int       _seq;
  };

  } }
