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
  result<prov::spec> user_provider::describe() {
    static const std::string desc =
#include "user.yaml"
      ;
    return prov::spec::read("user", desc);
  }

  result<bool> user_provider::suitable() {
    _cmd_useradd = command::create("useradd");
    _cmd_usermod = command::create("usermod");
    _cmd_userdel = command::create("userdel");
    return _cmd_useradd && _cmd_usermod && _cmd_userdel;;
  }

  void user_provider::flush() {
    return; // Noop, no batching
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

  std::unique_ptr<resource> user_provider::create(const std::string& name) {
    auto shared_this = std::static_pointer_cast<user_provider>(shared_from_this());
    return std::unique_ptr<resource>(new user_resource(shared_this, name, false));
  }

  result<std::vector<resource_uptr>> user_provider::instances() {
    std::vector<std::unique_ptr<resource>> result;
    auto shared_this = std::static_pointer_cast<user_provider>(shared_from_this());
    struct passwd *p = NULL;

    setpwent();
    while ((p = getpwent()) != NULL) {
      // FIXME: check errno according to getpwent(3)
      auto res = std::unique_ptr<resource>(new user_resource(shared_this, p->pw_name, true));
      (*res)["ensure"]  = "present";
      (*res)["comment"] = std::string(p->pw_gecos);
      (*res)["gid"]     = std::to_string(p->pw_gid);
      (*res)["home"]    = std::string(p->pw_dir);
      (*res)["shell"]   = std::string(p->pw_shell);
      (*res)["uid"]     = std::to_string(p->pw_uid);
      add_group_list(*res, p->pw_name, p->pw_gid);
      result.push_back(std::move(res));
    }
    endpwent();

    return std::move(result);
  }

  result<changes>
  user_provider::user_resource::update(const attr_map& should) {
    auto props = { "comment", "gid", "home", "shell", "uid" };

    auto& self = *this;
    auto res = result<changes>(changes());
    changes& chgs = res.ok();

    auto state = lookup<std::string>("ensure", "absent");
    auto ensure = should.lookup<std::string>("ensure", state);

    if (ensure != state) {
      chgs.push_back(change("ensure", ensure, state));
    }
    self["ensure"] = ensure;

    if (ensure == "present") {
      for (auto prop : props) {
        auto p = should.lookup<std::string>(prop);
        if (p && self[prop] != value(*p)) {
          chgs.push_back(change(prop, *p, self[prop]));
          self[prop] = *p;
        }
      }
      auto is_groups = lookup<array>("groups", array());
      auto groups = should.lookup<array>("groups", is_groups);
      auto is_groups_set = std::set<std::string>(is_groups.begin(),
                                                 is_groups.end());
      auto groups_set = std::set<std::string>(groups.begin(), groups.end());
      if (is_groups_set != groups_set) {
        chgs.add("groups", groups, is_groups);
        self["groups"] = groups;
      }

      if (!_exists || !chgs.empty()) {
        std::vector<std::string> args;
        if (self["comment"].is_present()) {
          args.push_back("-c");
          args.push_back(self["comment"].to_string());
        }
        if (self["gid"].is_present()) {
          args.push_back("-g");
          args.push_back(self["gid"].to_string());
        }
        if (self["home"].is_present()) {
          args.push_back("-d");
          args.push_back(self["home"].to_string());
        }
        if (self["shell"].is_present()) {
          args.push_back("-s");
          args.push_back(self["shell"].to_string());
        }
        if (self["uid"].is_present()) {
          args.push_back("-u");
          args.push_back(self["uid"].to_string());
        }
        if (groups_set != is_groups_set) {
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
        args.push_back(name());
        result<bool> run_result;
        if (_exists) {
          run_result = _prov->_cmd_usermod->run(args);
        } else {
          run_result = _prov->_cmd_useradd->run(args);
        }
        if (! run_result)
          return run_result.err();
      }
    } else if (ensure == "absent") {
      if (state != "absent") {
        auto run_result = _prov->_cmd_userdel->run({ "-r", name() });
        if (! run_result)
          return run_result.err();
      }
    } else if (ensure == "role") {
      return error("can not ensure=role with this provider");
    }
    return res;
  }
}
