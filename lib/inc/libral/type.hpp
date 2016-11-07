#pragma once

#include <string>
#include <vector>
#include <memory>
#include <boost/optional.hpp>

#include "provider.hpp"

namespace libral {
  /* type is really a misnomer for this class; it's not really the type you
     are used from Puppet, since it's not concerned (yet?) with validation
     against a schema etc. Rather, it's mostly a very simple manager for
     providers */
  class type {
  public:
    type(const std::string name, std::shared_ptr<provider> prov)
      : _name(name), _prov(prov) { }
    const std::string& name(void) const { return _name; }
    boost::optional<std::unique_ptr<resource>> find(const std::string &name);
    std::vector<std::unique_ptr<resource>> instances(void);
    // @todo lutter 2016-06-08: some error indication might be nice
    std::pair<std::unique_ptr<resource>, std::unique_ptr<result<changes>>>
    update(const std::string& name, const attr_map& attrs);
    void flush() { _prov->flush(); }
  private:
    std::string _name;
    std::shared_ptr<provider> _prov;
  };
}
