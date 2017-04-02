#pragma once

#include <string>
#include <vector>
#include <memory>
#include <boost/optional.hpp>

#include <libral/provider.hpp>

namespace libral {
  /* type is really a misnomer for this class; it's not really the type you
     are used from Puppet, since it's not concerned (yet?) with validation
     against a schema etc. Rather, it's mostly a very simple manager for
     providers */
  class type {
  public:
    type(std::shared_ptr<provider> prov) : _prov(prov) { }

    const std::string& qname(void) const { return _prov->spec()->qname(); }
    const std::string& type_name(void) const
      { return _prov->spec()->type_name(); }

    result<boost::optional<resource>> find(const std::string &name);
    result<std::vector<resource>> instances(void);
    result<std::pair<update, changes>> set(const resource& should);

    /* Turn a string into a value. Return an error message if that is not
       possible */
    result<value> parse(const std::string &name, const std::string &v);

    provider& prov() { return *_prov; };
    const provider& prov() const { return *_prov; };

    // Only needed in tests
    const std::shared_ptr<provider> prov_ptr() const { return _prov; }
  private:
    std::string _name;
    std::shared_ptr<provider> _prov;
  };
}
