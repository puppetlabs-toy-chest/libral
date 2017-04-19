#include <libral/user.hpp>

#include <leatherman/locale/locale.hpp>

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <stdlib.h>

#ifndef HAVE_UNSIGNED_GID_T
typedef gid_t getgroups_t;
#else
// Group list argument to getgrouplist() is a signed int (not gid_t) on OS X
typedef int getgroups_t;
#endif

using namespace leatherman::locale;

namespace libral {
  result<prov::spec> user_provider::describe(environment &env) {
    static const std::string desc =
#include "user.yaml"
      ;

    _cmd_useradd = env.command("useradd");
    _cmd_usermod = env.command("usermod");
    _cmd_userdel = env.command("userdel");

    auto suitable = _cmd_useradd && _cmd_usermod && _cmd_userdel;

    return env.parse_spec("user", desc, suitable);
  }

  /* Find all the groups user belongs to and add it to rsrc["groups"] */
  static result<bool> add_group_list(resource& rsrc,
                                     const char* user, gid_t group) {
    int ngroups = 0;
    getgroups_t *groups = nullptr;
    getgrouplist(user, group, nullptr, &ngroups);
    groups = new getgroups_t[ngroups];
    getgrouplist(user, group, groups, &ngroups);
    auto group_names = array();
    for (int i=0; i < ngroups; i++) {
      struct group *gr = getgrgid(groups[i]);
      if (gr != nullptr) {
        group_names.push_back(gr->gr_name);
      }
    }
    delete[] groups;
    rsrc["groups"] = std::move(group_names);
    return true;
  }

  result<std::vector<resource>>
  user_provider::get(context &ctx,
                     const std::vector<std::string>& names,
                     const resource::attributes& config) {
    std::vector<resource> result;
    struct passwd *p = NULL;

    setpwent();
    while ((p = getpwent()) != NULL) {
      // FIXME: check errno according to getpwent(3)
      auto res = create(p->pw_name);
      res["ensure"]  = "present";
      res["comment"] = std::string(p->pw_gecos);
      res["gid"]     = std::to_string(p->pw_gid);
      res["home"]    = std::string(p->pw_dir);
      res["shell"]   = std::string(p->pw_shell);
      res["uid"]     = std::to_string(p->pw_uid);
      add_group_list(res, p->pw_name, p->pw_gid);
      result.push_back(std::move(res));
    }
    endpwent();

    ctx.add_absent(result, names);
    return std::move(result);
  }

  result<void>
  user_provider::set(context &ctx, const updates& upds) {
    for (auto& u : upds) {
      auto res = set(ctx, u);
      if (!res)
        return res.err();
    }
    return result<void>();
  }

  result<void>
  user_provider::set(context &ctx, const update &upd) {
    static const std::vector<std::string> props =
      { "comment", "gid", "home", "shell", "uid" };
    static const std::map<std::string, std::string> opts =
      {{"comment", "-c"},
       {"gid", "-g"},
       {"home", "-d"},
       {"shell", "-s"},
       {"uid", "-u"}};

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
        auto p = should.lookup<std::string>(prop);
        if (should[prop] && is[prop] != should[prop]) {
          chgs.push_back(change(prop, *p, is[prop]));
          args.push_back(opts.at(prop));
          args.push_back(should[prop].to_string());
        }
      }

      /* Figure out if any supplementary groups were changed */
      auto is_groups = is.lookup<array>("groups", array());
      auto groups = should.lookup<array>("groups", is_groups);
      auto is_groups_set = std::set<std::string>(is_groups.begin(),
                                                 is_groups.end());
      auto groups_set = std::set<std::string>(groups.begin(), groups.end());
      if (is_groups_set != groups_set) {
        chgs.add("groups", groups, is_groups);
        args.push_back("-G");
        std::stringstream buf;
        bool first = true;
        for (auto g : groups) {
          if (first) {
            first = false;
          } else {
            buf << ",";
          }
          buf << g;
        }
        args.push_back(buf.str());
      }

      args.push_back(upd.name());
      if (! chgs.empty()) {
        result<void> run_result;
        if (state == "present") {
          run_result = _cmd_usermod->run(args);
        } else {
          run_result = _cmd_useradd->run(args);
        }
        if (! run_result)
          return run_result;
      }
    } else if (ensure == "absent") {
      if (state != "absent") {
        auto run_result = _cmd_userdel->run({ "-r", upd.name() });
        if (! run_result)
          return run_result.err();
      }
    } else if (ensure == "role") {
      return error("can not ensure=role with this provider");
    }
    return result<void>();
  }
}
