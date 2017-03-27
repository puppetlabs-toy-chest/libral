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

  struct update {
    resource is;
    resource should;

    const std::string& name() const { return is.name(); }

    bool changed(const std::string& attr) const {
      return (should[attr] && is[attr] != should[attr]);
    }

    const value& operator[](const std::string& attr) const {
      if (should[attr])
        return should[attr];
      return is[attr];
    }

    bool present() const {
      // This makes 'ensure' and the value 'absent' very special
      return is.lookup<std::string>("ensure", "absent") == "present";
    }
  };

  using updates = std::vector<update>;
}
