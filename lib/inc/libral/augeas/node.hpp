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
    node(std::shared_ptr<handle> aug, const std::string path);
    const std::string& path() const { return _path; }
    boost::optional<std::string> operator[](const std::string& path) const;

    /**
     * Set the value of the child PATH to v.to_string() unless v is none,
     * in which case no set is performed
     */
    void set(const std::string& path, const value& v);

    /** Sets the value of child PATH to v.to_string() unless v is none; in
     * that case set it to DEFLT
     */
    void set(const std::string& path,
             const value& v,
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

} }
