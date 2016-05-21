#pragma once

#include <libral/provider.hpp>

namespace libral {
  /* A provider here consists of two separate classes, since we can't do
     all the metaprogramming magic that Ruby does. The two classes are

     - a subclass of provider, which is responsible for dealing with all
       resources of a certain kind (what would be class methods in a Ruby
       provider)

     - a subclass of resource, which represents an instance of whatever
       resources the corresponding provider manages
  */

  class mount_provider : public provider {
  public:

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

    mount_provider() : aug(nullptr) { };

    void prepare();
    void flush();
    std::vector<std::unique_ptr<resource>> instances();
  private:
    std::unique_ptr<aug::handle> aug;
  };

}
