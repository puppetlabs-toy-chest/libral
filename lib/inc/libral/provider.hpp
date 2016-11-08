#pragma once

#include <memory>
#include <vector>
#include <map>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <libral/result.hpp>

namespace libral {
  /* Nomenclature warning: a provider here is the thing that knows how to
     deal with a certain kind of resources, e.g., that knows how to list
     all users on a system.

     We call the representation of an individual user a 'resource'
  */

  // This will eventually need to be a PCore value
  typedef boost::optional<std::string> value;
  // This should probably be its own class
  typedef std::map<std::string, value> attr_map;

  /* Record the change of attribute ATTR from WAS to IS */
  struct change {
    change(const std::string &a, const value &i, const value &w = boost::none)
      : attr(a), is(i), was(w) {};
    std::string attr;
    value is;
    value was;
  };

  // Will probably become its own class sooner or later
  typedef std::vector<change> changes;

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
    /* Return the string value for KEY or DEFLT if that is not set
       or is boost::none */
    const std::string& lookup(const std::string& key,
                              const std::string& deflt) const;

    /* Update this resource's properties to the values in SHOULD. Only the
     * attributes mentioned in SHOULD should be modified, all others need
     * to be left alone. It is safe to assume that the current attributes
     * of the resource represent the 'is' state.
     */
    virtual std::unique_ptr<result<changes>> update(const attr_map& should) = 0;
  protected:
    void set_attrs(const attr_map& should);
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

    /* Return +true+ if the provider can be used on the system */
    virtual bool suitable() = 0;

    /* Prepare using this provider; this is called once for each provider
       class */
    virtual void prepare() { };

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
