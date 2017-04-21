#pragma once

#include <map>

#include <libral/value.hpp>

namespace libral {
  /** An individual thing that we manage

      The resource only has one set of attributes, whether this resource
      represents 'is' or 'should' state is therefore depending on the
      context in which the resource is used.

      A resource is little more than a map from attribute name to their
      value. The only difference with an ordinary map is that it also has a
      name.
  */
  class resource {
  protected:
    friend class provider;

    /**
     * Constructs a resource with a name and no attributes set.
     */
    resource(const std::string& name) : _name(name) { }

  public:
    /**
     * A unique pointer to a resource
     */
    using uptr = std::unique_ptr<resource>;

    /**
     * A map of attribute names to values
     */
    using attributes = std::map<std::string, value>;

    /**
     * Retrieves the name of this resource
     */
    const std::string& name() const { return _name; }

    /**
     * Returns the current state of attribute ATTR
     */
    const value& operator[](const std::string& key) const;

    /**
     * Returns the current state of attribute ATTR
     */
    value& operator[](const std::string& attr);

    /**
     * Erases the value of attribute ATTR
     */
    void erase(const std::string attr) { _attrs.erase(attr); }

    /**
     * Returns an iterator to the beginning of the attributes
     */
    attributes::iterator attr_begin() { return _attrs.begin(); }

    /**
     * Returns an iterator to the end of the attributes
     */
    attributes::iterator attr_end() { return _attrs.end(); }

    const attributes& attrs() const { return _attrs; }

    /**
     * Returns the value associated with KEY if it is of type T, and DEFLT
     * otherwise
     */
    template<typename T>
    const T& lookup(const std::string& key, const T& deflt) const;

    /**
     * Returns a pointer to the value associated with KEY if it is of type
     * T, and NULLPTR otherwise.
     */
    template<typename T>
    const T* lookup(const std::string& key) const;

  private:
    bool is_name(const std::string& key) const;

    std::string _name;
    attributes _attrs;
  };

  /**
   * Represents a desired update to one resource and bundles the is and the
   * should state of the resource, together with some convenience methods
   * to access various aspects of the is/should pair of resources.
   *
   * Attributes for the should resource will only be set if a change to
   * them has been requested (though it is entirely possible that the is
   * and should value of the attribute are identical) If an attribute is
   * not set in the should resource, it is certain that no change to it is
   * being requested.
   */
  struct update {
    /** The current "is" state of the resource */
    resource is;
    /** The desired "should" state of the resource */
    resource should;

    /**
     * Returns the name of the underlying resource.
     */
    const std::string& name() const { return is.name(); }

    /**
     * Return true if this update contains a change for the attribute
     * attr, i.e., if the should value for this attribute is set and
     * differs from the is value.
     */
    bool changed(const std::string& attr) const {
      return (should[attr] && is[attr] != should[attr]);
    }

    /**
     * Returns the should value for the attribute attr if it is set, and
     * returns the is value for that attribute otherwise. If neither is
     * set, returns a none value.
     */
    const value& operator[](const std::string& attr) const {
      if (should[attr])
        return should[attr];
      return is[attr];
    }

    /**
     * Returns true if the is value for 'ensure' is anything but "absent"
     */
    bool present() const {
      // This makes 'ensure' and the value 'absent' very special
      return is.lookup<std::string>("ensure", "absent") != "absent";
    }
  };

  using updates = std::vector<update>;
}
