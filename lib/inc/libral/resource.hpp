#pragma once

#include <string>
#include <map>

#include <libral/provider.hpp>

namespace libral {
  typedef std::map<std::string, std::string> attr_map;

  class resource {
  public:
    resource(const std::string name) { _values["name"] = name; }
    const std::string& name() { return _values["name"]; }

    /* Return the current state of attribute ATTR */
    const std::string& operator[](const std::string& attr) const;
    std::string& operator[](const std::string& attr);
    attr_map::iterator attr_begin() { return _values.begin(); }
    attr_map::iterator attr_end() { return _values.end(); }

    // @todo lutter 2016-05-16: noop for now
    virtual void destroy() {};
    virtual void modify(const attr_map& should) {};
  private:
    attr_map _values;
  };

  class mount_resource : public resource {
  public:
    mount_resource(std::shared_ptr<mount_provider>& prov, const aug::node& base)
      : resource(base["file"]), _prov(prov), _base(base) { extract_base(); }
  private:
    // Copy values from _base into _values
    void extract_base();

    std::shared_ptr<mount_provider> _prov;
    aug::node                       _base;
  };
}
