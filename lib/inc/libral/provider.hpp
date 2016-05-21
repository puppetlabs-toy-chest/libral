#pragma once

#include <memory>
#include <vector>
#include <map>

#include <libral/augeas.hpp>

namespace libral {
  /* Nomenclature warning: a provider here is the thing that knows how to
     deal with a certain kind of resources, e.g., that knows how to list
     all users on a system.

     We call the representation of an individual user a 'resource'
  */

  typedef std::map<std::string, std::string> attr_map;


  /* An individual thing that we manage */
  class resource {
  public:
    resource(const std::string name) : _name(name){ }
    const std::string& name() { return _name; }

    /* Return the current state of attribute ATTR */
    const std::string& operator[](const std::string& attr) const;
    std::string& operator[](const std::string& attr);
    attr_map::iterator attr_begin() { return _values.begin(); }
    attr_map::iterator attr_end() { return _values.end(); }

    // @todo lutter 2016-05-16: noop for now
    virtual void destroy() {};
    virtual void update(const attr_map& should) {};
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

    /* Prepare using this provider; this is called once for each provider
       class */
    virtual void prepare() { };
    /* Perform all changes that may have been queued up by the resources
       created by this provider */
    virtual void flush() { };

    /* Create a new resource */
    //virtual std::unique_ptr<resource> create(const std::string &name);

    //virtual boost::optional<std::unique_ptr<resource>>
    //  find(const std::string &name);

    /* Retrieve all resources managed by this provider */
    virtual std::vector<std::unique_ptr<resource>> instances() = 0;
  };
}
