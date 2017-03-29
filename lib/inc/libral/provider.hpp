#pragma once

#include <vector>
#include <map>
#include <memory>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <libral/result.hpp>
#include <libral/value.hpp>
#include <libral/resource.hpp>
#include <libral/context.hpp>
#include <libral/environment.hpp>
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
  class provider {
  public:
    provider() { };
    virtual ~provider() = default;

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
     * Gets the provider ready for use. Calls describe() to kick off any
     * provider-specific initialization.
     */
    result<void> prepare(environment &env);

    /**
     * Constructs and returns the provider spec. The \p env object can be
     * used to interact with the runtime environment. This method must also
     * do any provider-internal initialization and check whether the
     * provider is suitable for this system.
     */
    virtual result<prov::spec> describe(environment &env) = 0;
  private:
    /* This gets only intitialized when we call prepare, and we therefore
     * must allow for it to be none for a while
     */
    boost::optional<prov::spec> _spec;
  };
}
