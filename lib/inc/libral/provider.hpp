#pragma once

#include <memory>
#include <vector>

#include <libral/augeas.hpp>

namespace libral {
  /* Nomenclature warning: a provider here is the thing that knows how to
     deal with a certain kind of resources, e.g., that knows how to list
     all users on a system.

     We call the representation of an individual user a 'resource'
   */

  class resource;

  class provider : public std::enable_shared_from_this<provider> {
  public:
    provider() { };

    /* Prepare using this provider; this is called once for each provider
       class */
    virtual void prepare() { };
    /* Perform all changes that may have been queued up by the resources
       created by this provider */
    virtual void flush() { };
    /* Retrieve all resources managed by this provider */
    virtual std::vector<std::unique_ptr<resource>> instances() = 0;
  };

  class mount_provider : public provider {
  public:
    mount_provider() : aug(nullptr) { };

    void prepare();
    void flush();
    std::vector<std::unique_ptr<resource>> instances();
  private:
    std::unique_ptr<aug::handle> aug;
  };
}
