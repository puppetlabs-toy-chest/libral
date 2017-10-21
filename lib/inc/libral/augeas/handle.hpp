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
    using callback = std::function<void(::augeas *)>;

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

    void print(const std::string& path);

    /**
     * Returns a handle to a new augeas instance which will load lenses
     * from loadpath, a colon separated list of directories. The reader and
     * writer callbacks are used when the tree needs to be loaded and
     * saved, making it possible to process files that are not on the local
     * system; if they are not given, aug_load and aug_save are used for
     * that.
     *
     * The reader must place file contents into the augeas tree, which
     * should generally happen at /files/$path, e.g. using
     * aug_text_store. The writer needs to turn the interesting parts of
     * the tree back into text and store it on disk.
     *
     * Both should report errors by setting at least
     * /augeas/files/$path/error and /augeas/files/$path/error/message to a
     * machine-readable and a human readable indication of the
     * error. Additional entries can be set in accordance to what augeas
     * itself does when it encounters errors.
     *
     * When a reader or a writer are provided, the augeas root is set to
     * /dev/null to make sure that there is no chance of inadvertently
     * modifying the local filesystem through aug_load and aug_save.
     *
     * The reader should take into account that aug_load by default makes
     * sure that all input text ends with a '\n' (it appends one if there
     * is none) and that lenses generally expect that newline to be there,
     * and otherwise might fail on non-newline terminated input.
     */
    static std::shared_ptr<handle> make(const std::string& loadpath,
                                        const callback& reader = nullptr,
                                        const callback& writer = nullptr) {
      return std::shared_ptr<handle>(new handle(loadpath, reader, writer));
    }

  private:
    handle(const std::string& loadpath,
           const callback& reader,
           const callback &writer);

    /* Checks _augeas for errors and return an error if there is one. */
    result<void> check_error() const;

    ::augeas* _augeas;
    /* make_node_seq_next uses this to make sure we create unique sequence
       numbers for nodes that need it */
    int       _seq;
    callback  _reader;
    callback  _writer;
  };

  } }
