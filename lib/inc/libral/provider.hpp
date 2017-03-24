#pragma once

#include <memory>
#include <vector>
#include <map>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <libral/result.hpp>
#include <libral/value.hpp>
#include <libral/resource.hpp>
#include <libral/context.hpp>
#include <libral/prov/spec.hpp>

namespace libral {
  /**
   * Stream insertion operator for changes.
   * @param os The output stream to write the changes to.
   * @param chgs The changes to write.
   * @return Returns the given output stream.
   */
  std::ostream& operator<<(std::ostream& os, changes const& chgs);

  /* Class provider, that knows how to manage lots of things that are
     accessed in the same way
  */
  class provider : public std::enable_shared_from_this<provider> {
  public:
    provider() { };
    virtual ~provider() = default;

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

    /**
     * Retrieve the resources managed by this provider. At least the
     * resources mentioned in NAMES must be included in the returned
     * vector, though more may be returned.
     *
     * The CONFIG can contain parameters that influence how the provider
     * retrieves resources and where it looks for them. Config parameters
     * that were used to retrieve a resource must be reflected in the
     * returned resource.
     */
    // IOW, this needs to be true:
    // auto rsrcs = get(names, config);
    // for (auto r : rsrcs) {
    //   assert(r == get( { r.name() }, r.attributes())
    // }
    virtual result<std::vector<resource>>
    get(context &ctx,
        const std::vector<std::string>& names,
        const resource::attributes& config) = 0;

    virtual result<void>
    set(context &ctx, const updates& upds) = 0;

    /**
     * Reads the string representation v for attribute name and returns the
     * corresponding value. If v is not a valid string for name's type,
     * returns an error indicating the problem.
     */
    result<value> parse(const std::string& name, const std::string& v);

    /**
     * Return the provider specification
     */
    const boost::optional<prov::spec>& spec() const { return _spec; };

    /**
     * Return a human-readable string that indicates where this
     * provider came from.
     */
    virtual const std::string& source() const;

    /**
     * Creates a new resource object. This does not make any changes on the
     *  system, it only hides the mechanics of instantiating a resource
     *  object.
     */
    resource create(const std::string &name) const;

    /**
     * Creates a resource reflecting the state after the update has been
     * made. Attributes mentioned in UPD.SHOULD take precedence over
     * attributes mentioned in UPD.IS
     */
    resource create(const update &upd, const changes& chgs) const;
  protected:
    friend class ral;

    /**
     * Sets up internal data structures and is called after suitable() is
     */
    result<bool> prepare();

    /**
     * Returns the provider spec for this provider.
     */
    virtual result<prov::spec> describe() = 0;
  private:
    /* This gets only intitialized when we call prepare, and we therefore
     * must allow for it to be none for a while
     */
    boost::optional<prov::spec> _spec;
  };
}
