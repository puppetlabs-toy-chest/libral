#pragma once

#include <libral/provider.hpp>

#include <libral/command.hpp>

namespace libral {
  /* Group provider for posix systems using groupadd/groupmod/groupdel from
     util-linux
  */
  class group_provider : public provider {
  public:
    result<std::vector<resource>>
    get(context &ctx, const std::vector<std::string>& names,
        const resource::attributes& config) override;

    result<void>
    set(context &ctx, const updates& upds) override;

  protected:
    result<prov::spec> describe(environment &env) override;
  private:
    result<void> set(context &ctx, const update &upd);

    command::uptr                 _cmd_groupadd;
    command::uptr                 _cmd_groupmod;
    command::uptr                 _cmd_groupdel;
    std::string                   _data_dir;
  };

}
