#pragma once

#include <libral/provider.hpp>
#include <libral/augeas.hpp>

namespace libral {
  class host_provider : public provider {
  public:
    result<std::vector<resource>>
    get(context &ctx, const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void> set(context &ctx, const updates& upds) override;

  protected:
    result<prov::spec> describe(environment& env) override;

  private:
    result<augeas::node> base(const update &upd);
    result<resource> make(const std::string& name,
                          const augeas::node& base, const std::string& ens);
    result<void> update_base(const update &upd);
    result<void> set(context &ctx, const update &upd);

    std::shared_ptr<augeas::handle> _aug;
  };
}
