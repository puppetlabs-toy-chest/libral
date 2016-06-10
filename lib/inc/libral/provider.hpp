#pragma once

#include <memory>
#include <vector>
#include <map>

#include <boost/optional.hpp>

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

  /* An individual thing that we manage */
  class resource {
  public:
    resource(const std::string name) : _name(name){ }
    const std::string& name() { return _name; }

    /* Return the current state of attribute ATTR */
    const value operator[](const std::string& key) const;
    value& operator[](const std::string& attr);
    void erase(const std::string attr) { _values.erase(attr); }
    attr_map::iterator attr_begin() { return _values.begin(); }
    attr_map::iterator attr_end() { return _values.end(); }
    /* Return the string value for KEY or DEFLT if that is not set
       or is boost::none */
    const std::string& lookup(const std::string& key,
                              const std::string& deflt) const;

    // @todo lutter 2016-05-16: noop for now
    virtual void destroy() {};
    virtual void update(const attr_map& should) {};
  protected:
    void update_values(const attr_map& should);
  private:
    std::string _name;
    attr_map _values;
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
