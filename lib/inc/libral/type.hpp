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

    /**
     * Sets the resources to the desired state indicated in \p should. For
     * each resource, only attributes that might need to change have to be
     * filled in. Before performing the change, each of the resources is
     * looked up using \p get, and both the 'is' and the 'should' state are
     * passed to the provider.
     *
     * Returns an \p error if any change fails. If all changes succeed,
     * returns the updates, i.e. the pairs of is/should state that was
     * passed to the provider, and the changes that were actually performed
     * as reported by the provider.
     */
    result<std::vector<std::pair<update, changes>>>
    set(const std::vector<resource>& shoulds);

    /**
     * Looks up resources. The \p names indicate which resources the
     * provider should look up. To look up all resources, pass an empty
     * vector.
     *
     * Returns an error if the lookup failed. On success, returns a list of
     * resources that is guaranteed to at least contain the resources
     * mentioned in \p names but may contain more than that.
     */
    result<std::vector<resource>>
    get(const std::vector<std::string>& names);

    /* Turn a string into a value. Return an error message if that is not
       possible */
    result<value> parse(const std::string &name, const std::string &v);

    provider& prov() { return *_prov; };
    const provider& prov() const { return *_prov; };

    // Only needed in tests
    const std::shared_ptr<provider> prov_ptr() const { return _prov; }
  private:
    std::shared_ptr<provider> _prov;
  };
}
