#pragma once

#include <string>
#include <map>

#include <libral/provider.hpp>

namespace libral {
  typedef std::map<std::string, std::string> attr_map;

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
}
