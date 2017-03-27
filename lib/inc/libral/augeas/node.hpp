#pragma once

#include <libral/augeas/handle.hpp>

#include <vector>
#include <memory>
#include <string>
#include <boost/optional.hpp>

#include <libral/value.hpp>

namespace libral { namespace augeas {

  class handle;

  class node {
  public:
    node(std::shared_ptr<handle> aug, const std::string& path);
    const std::string& path() const { return _path; }

    /**
     * Returns the value of the node \p path. If \p path is relative, it is
     * taken as relative to this node.
     */
    result<boost::optional<std::string>>
    operator[](const std::string& path) const;

    /**
     * Returns the value for the node's path
     */
    result<boost::optional<std::string>>
    get() const { return _aug->get(path()); }

    /**
     * Set the value of the child PATH to v.to_string() unless v is none,
     * in which case no set is performed
     */
    result<void>
    set(const std::string& path, const value& v);

    /** Sets the value of child PATH to v.to_string() unless v is none; in
     * that case set it to DEFLT
     */
    result<void>
    set(const std::string& path, const value& v, const std::string& deflt);

    /**
     * Deletes this node and all its children.
     */
    result<void> rm();

    /**
     * Deletes the children of this node, but leaves the node alone
     */
    result<void> erase();

    /**
     * Sets the value for this node to NULL
     */
    result<void> clear();
  private:
    std::string append(const std::string& p2) const;

    std::shared_ptr<handle> _aug;
    std::string _path;
  };

} }
