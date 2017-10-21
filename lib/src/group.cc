#include <libral/group.hpp>

#include <leatherman/locale/locale.hpp>

#include <sys/types.h>
#include <grp.h>

using namespace leatherman::locale;

namespace libral {
  result<prov::spec> group_provider::describe(environment &env) {
    static const std::string desc =
#include "group.yaml"
      ;

    auto suitable = env.is_local();

    if (suitable) {
      _cmd_groupadd = env.command("groupadd");
      _cmd_groupmod = env.command("groupmod");
      _cmd_groupdel = env.command("groupdel");
      suitable = suitable &&
        _cmd_groupadd && _cmd_groupmod && _cmd_groupdel;
    }

    return env.parse_spec("group", desc, suitable);
  }

  result<std::vector<resource>>
  group_provider::get(context &ctx,
                      const std::vector<std::string>& names,
                      const resource::attributes& config) {
    std::vector<resource> res;
    struct group *g = NULL;

    setgrent();
    while ((g = getgrent()) != NULL) {
      // FIXME: check errno according to getgrent(3)
      auto group = create(g->gr_name);
      group["ensure"]  = "present";
      group["gid"]     = std::to_string(g->gr_gid);
      if (*g->gr_mem != nullptr) {
        array members;
        for (auto mem = g->gr_mem; *mem != nullptr; mem++) {
          members.push_back(std::string(*mem));
        }
        group["members"] = std::move(members);
      }
      res.push_back(std::move(group));
    }
    endgrent();

    ctx.add_absent(res, names);
    return std::move(res);
  }

  result<void>
  group_provider::set(context &ctx, const updates& upds) {
    for (auto& upd : upds) {
      auto res = set(ctx, upd);
      if (!res)
        return res.err();
    }
    return result<void>();
  }

  result<void>
  group_provider::set(context &ctx, const update &upd) {
    static const std::vector<std::string> props = { "gid" };
    static const std::map<std::string, std::string> opts =
      {{"gid", "-g"}};

    changes& chgs = ctx.changes_for(upd.name());
    auto& is = upd.is;
    auto& should = upd.should;

    auto state = is.lookup<std::string>("ensure", "absent");
    auto ensure = should.lookup<std::string>("ensure", state);

    if (ensure != state) {
      chgs.push_back(change("ensure", ensure, state));
    }

    if (ensure == "present") {
      std::vector<std::string> args;
      for (auto prop : props) {
        if (chgs.add(prop, upd)) {
          args.push_back(opts.at(prop));
          args.push_back(should[prop].to_string());
        }
      }

      args.push_back(upd.name());
      if (! chgs.empty()) {
        result<void> run_result;
        if (state == "present") {
          run_result = _cmd_groupmod->run(args);
        } else {
          run_result = _cmd_groupadd->run(args);
        }
        if (! run_result)
          return run_result;
      }
    } else if (ensure == "absent") {
      if (state != "absent") {
        auto run_result = _cmd_groupdel->run({ upd.name() });
        if (! run_result)
          return run_result.err();
      }
    }
    return result<void>();
  }
}
