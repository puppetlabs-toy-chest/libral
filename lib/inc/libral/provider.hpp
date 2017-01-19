#pragma once

#include <memory>
#include <vector>
#include <map>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <libral/result.hpp>
#include <libral/value.hpp>

namespace libral {
  /* Nomenclature warning: a provider here is the thing that knows how to
     deal with a certain kind of resources, e.g., that knows how to list
     all users on a system.

     We call the representation of an individual user a 'resource'
  */

  /* A map of attribute names to values. */
  // FIXME: we really need to combine attr_map and resource, maybe make
  // resource a subclass of atr_map ?
  struct attr_map : std::map<std::string, value> {

    /**
     * Looks up the value associated with key. If no entry exists, returns
     * boost::none.
     */
    const value operator[](const std::string& key) const;
    value& operator[](const std::string& attr);

    /**
     * Looks up the value of a given attribute.
     * @param key The name of the attribute
     * @param deflt The value to use if there is no suitable value for key
     * @return Returns the value associated with key if one exists and has
     * type T and deflt otherwise.
     */
    template<typename T>
    const T& lookup(const std::string& key, const T& deflt) const;

    /**
     * Looks up the value of a given attribute.
     * @param key The name of the attribute
     * @return Returns the value associated with key if one exists and has
     * type T and nullptr otherwise.
     */
    template<typename T>
    const T* lookup(const std::string& key) const;
  };

  /* Record the change of attribute ATTR from WAS to IS */
  struct change {
    change(const std::string &a, const value &i, const value &w = boost::none)
      : attr(a), is(i), was(w) {};
    std::string attr;
    value is;
    value was;
  };

  /* A list of changes */
  struct changes : std::vector<change> {
    void add(const std::string &attr, const value &is,
             const value &was = boost::none);
    bool exists(const std::string &attr);
  };

  /**
   * Stream insertion operator for changes.
   * @param os The output stream to write the changes to.
   * @param chgs The changes to write.
   * @return Returns the given output stream.
   */
  std::ostream& operator<<(std::ostream& os, changes const& chgs);

  /* An individual thing that we manage */
  /* The resource only has one set of attributes, therefore it's not clear
     whether those represent 'should' or 'is' state; that information is
     contextual but might need to get incorporated into the notion of a
     resource. Maybe. */
  class resource {
  protected:
    /* Resources are created by one of the provider methods: create, find,
       or instances, but should not be instantiated directly. */
    resource(const std::string name) : _name(name){ }
  public:
    const std::string& name() { return _name; }

    /* Return the current state of attribute ATTR */
    const value operator[](const std::string& key) const;
    value& operator[](const std::string& attr);
    void erase(const std::string attr) { _attrs.erase(attr); }
    attr_map::iterator attr_begin() { return _attrs.begin(); }
    attr_map::iterator attr_end() { return _attrs.end(); }
    template<typename T>
    const T& lookup(const std::string& key, const T& deflt) const;
    template<typename T>
    const T* lookup(const std::string& key) const;

    /* Update this resource's properties to the values in SHOULD. Only the
     * attributes mentioned in SHOULD should be modified, all others need
     * to be left alone. It is safe to assume that the current attributes
     * of the resource represent the 'is' state.
     */
    virtual std::unique_ptr<result<changes>> update(const attr_map& should) = 0;

  private:
    std::string _name;
    attr_map _attrs;
  };


  /* Class provider, that knows how to manage lots of things that are
     accessed in the same way
  */
  class provider : public std::enable_shared_from_this<provider> {
  public:
    provider() { };

    /* Returns +true+ if the provider can be used on the system. Returns
       +false+ if the provider can not be used on the system.

       If a problem is encountered that should be considered an error and
       reported back to the user, return an error result. The provider will
       be considered not suitable in that case.
    */
    virtual result<bool> suitable() = 0;

    /* Perform all changes that may have been queued up by the resources
       created by this provider */
    virtual void flush() { };

    /* Create a new resource

       @todo lutter 2016-06-08: do we really need that ? Why not have a
       update(name, attrs) and let that sort out whether a resource needs
       to be created or not
     */
    virtual std::unique_ptr<resource> create(const std::string &name) = 0;

    virtual boost::optional<std::unique_ptr<resource>>
      find(const std::string &name);

    /* Retrieve all resources managed by this provider */
    virtual std::vector<std::unique_ptr<resource>> instances() = 0;
  };
}
