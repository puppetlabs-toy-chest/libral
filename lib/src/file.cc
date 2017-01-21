#include <libral/file.hpp>

#include <sstream>
#include <iomanip>
#include <iostream>

// We use stat/lstat here; those are not portable, but boost::filesystem
// doesn't give us owner/group; to make this portable, we will need to
// detect whether lstat is available and fall back to boost::filesystem
// otherwise
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/nowide/fstream.hpp>
#include "boost/system/error_code.hpp"
#include "boost/filesystem.hpp"

#include <leatherman/locale/locale.hpp>

namespace fs = boost::filesystem;
using ftype = fs::file_type;
using namespace leatherman::locale;

namespace libral {

  using fprov = file_provider;

  /* A map from ftype to the strings we use for the ensure
   * and type attributes
   *
   * This produces a broader range of ensure values than what puppet
   * supports, as puppet only has present, absent, file, directory, and
   * link
   *
   * It also produces better behavior as 'puppet resource' hangs on dev
   * nodes and pipes, and gives an error on sockets
   */
  struct fdesc {
    ftype       ftyp;
    std::string ensure;
    std::string typ;
  };

  static const std::string iso_8601_format = "%FT%T%z";

  // String constants for the ensure/state values
  static const std::string s_absent    = "absent";
  static const std::string s_present   = "present";
  static const std::string s_file      = "file";
  static const std::string s_directory = "directory";
  static const std::string s_link      = "link";

  static const std::array<fdesc, 11> ftype_ensure = {{
      // unknown must be the first entry
      // entries are searched in order so that for duplicate values,
      // the 'canonical' entry must come first
      { ftype::type_unknown,   s_absent,    "unknown" },
      { ftype::status_error,   s_absent,    "error" },
      { ftype::file_not_found, s_absent,    s_absent},
      { ftype::regular_file,   s_file,      s_file },
      { ftype::regular_file,   s_present,   s_file },
      { ftype::directory_file, s_directory, s_directory},
      { ftype::symlink_file,   s_link,      s_link},
      { ftype::block_file,     s_present,    "block"},
      { ftype::character_file, s_present,    "char"},
      { ftype::fifo_file,      s_present,    "fifo"},
      { ftype::socket_file,    s_present,    "socket"}
    }};

  /* Get a string representation for the given ftype */
  const fdesc& fdesc_from_ftype(const ftype& t) {
    for (auto it = ftype_ensure.begin(); it != ftype_ensure.end(); it++) {
      if (t == it->ftyp)
        return *it;
    }
    return ftype_ensure[0];
  }

  result<prov::spec> fprov::describe() {
    static const std::string desc =
#include "file.yaml"
      ;
    return prov::spec::read("file", desc);
  }

  std::vector<std::unique_ptr<resource>> fprov::instances() {
    std::vector<std::unique_ptr<resource>> result;

    return result;
  }

  std::unique_ptr<resource> fprov::create(const std::string& name) {
    auto shared_this = std::static_pointer_cast<file_provider>(shared_from_this());
    boost::system::error_code ec;
    // FIXME: we really want to call lexically_normal() on the path to get
    // rid of redundant .. etc. That function is in Boost 1.60, but not in
    // Boost 1.58 which pl-build-tools has on CentOS 6
    auto cname = fs::absolute(fs::path(name));
    auto ptr = new file_resource(shared_this, cname.native());
    return std::unique_ptr<resource>(ptr);
  }


  boost::optional<std::unique_ptr<resource>>
  fprov::find(const std::string &name) {
    auto result = create(name);
    auto& res = *result;

    load(res);

    return std::move(result);
  }

  // Load file attributes into res from whatever is on disk
  void fprov::load(resource &res) {
    /* Look up file and fill in attributes */
    boost::system::error_code ec;
    auto st = fs::symlink_status(res.name(), ec);
    if (ec) {
      // FIXME: Be a little more discrimnating here; for example, if we get
      // EACCES, should we really act as if the file doesn't exist ?
      res["ensure"] = s_absent;
      return;
    }

    res["ensure"] = fdesc_from_ftype(st.type()).ensure;
    res["type"] = fdesc_from_ftype(st.type()).typ;

    if (st.type() == ftype::symlink_file) {
      res["target"] = fs::read_symlink(res.name()).native();
    }

    {
      // Turn mode into an octal string
      std::ostringstream os;
      os.fill('0');
      os.width(st.permissions() > fs::perms::all_all ? 5 : 4);
      os << std::oct << st.permissions();
      res["mode"] = os.str();
    }

    {
      // Get mtime; we lose the subsecond bits of mtime
      auto mtime = fs::last_write_time(res.name());
      res["mtime"] = time_as_iso_string(&mtime);
      // At some point, we should make it possible to set the type of
      // checksum to compute. As we do not support other ways to specify
      // content than literally through a 'content' attribute, this is
      // kinda pointless for now
      res["checksum_value"] = std::to_string(mtime);
      res["checksum"] = "mtime";
    }

    find_from_stat(res);
  }

  /* Fill in attributes we can only get via stat(2) and not from
     boost::filesystem, most notably owner, group, and ctime */
  void fprov::find_from_stat(resource &res) {
    struct stat buf;
    int r = lstat(res.name().c_str(), &buf);
    if (r < 0) {
      std::cout << "errno" << errno << std::endl;
      return;
    }

    res["ctime"] = time_as_iso_string(&buf.st_ctime);
    res["owner"] = std::to_string(buf.st_uid);
    res["group"] = std::to_string(buf.st_gid);
  }

  /*
   * The logic for updating a file is pretty convoluted, because we need to
   * take several situations into account; in particular, we need to cater
   * for:
   * - file exists and has the right type
   * - file exists but has the wrong type (e.g., link -> directory)
   * - file does not exist
   *
   * This wonderful ASCII table tries to clarify what's going on. The
   * columns indicate the current state of the file, the rows the desired
   * state expressed in ensure. Before we enter the logic behind this
   * table, we make sure neither current state nor ensure are
   * 'present'. The current state always indicates the specific type of
   * file if it exists (or is 'absent'). When 'ensure' is 'present', we
   * either set it to the current state, or if the state is 'absent', we
   * default it to 'file'.
   *
   *             absent       file        directory        link
   * ----------+----------+-----------+---------------+--------------+
   *           |          |           |               |              |
   *   absent  |  ---     |  remove   |   remove      |  remove      |
   *           |          |           |               |              |
   * ----------+----------+-----------+---------------+--------------+
   *           | touch    |           |   remove      |    remove    |
   *    file   | -> file  | metadata  | ->absent      | -> absent    |
   *           |          | content   |               |              |
   * ----------+----------+-----------+---------------+--------------+
   *           | mkdir    |   remove  |               |              |
   * directory | -> dir   |-> absent  |  metadata     |  remove      |
   *           |          |           |               |  -> absent   |
   * ----------+----------+-----------+---------------+--------------+
   *           |          |  remove   |  remove       |  target      |
   *  link     | -> link  |-> absent  |  -> absent    |  metadata    |
   *           |          |           |               |              |
   * ----------+----------+-----------+---------------+--------------+
   *
   * - 'remove' means either unlink (file/link) or rmdir/rm -rf (directory,
   *   depending on value of 'force'
   * - the notation '-> absent' means go to the 'absent' column in the
   *   same row
   * - 'metadata', 'content', 'target' are short for 'update_metadata' etc.
   */
  fprov::change_result_uptr
  fprov::file_resource::update(const attr_map& should) {
    auto& is = *this;
    auto res = result<changes>::make_unique();

    auto state = lookup<std::string>("ensure", s_absent);
    if (state == s_present) {
      state = lookup<std::string>("type", s_file);
    }
    auto ensure = should.lookup<std::string>("ensure", state);

    // Change ensure == "present" to something else, either "file" when we
    // need to create it, or the type it is right now
    if (ensure == s_present) {
      if (state == s_absent) {
        ensure = s_file;
      } else {
        ensure = state;
      }
    }

    auto force = (should.lookup<std::string>("force", "false") == "true");

    // See if we would have to mutate a directory to something else without
    // the force flag
    if (state == s_directory && state != ensure && !force) {
      *res = error(_("Cannot change file '{1}' from directory to {2}",
                     name(), ensure));
      return res;
    }

    // Any sort of change os the file type is always 'remove old' followed
    // by doing the same as if the file was absent
    if (ensure != state) {
      if (state != s_absent) {
        remove(*res, state, force);
        if (!(*res)) {
          return res;
        }
      }

      changes& chgs = res->ok();
      is["ensure"] = ensure;
      chgs.add("ensure", ensure, state);
    }

    if (ensure == s_file) {
      if (state == s_absent) {
        create_file(*res);
      }
      update_metadata(*res, should);
      update_content(*res, should);
    } else if (ensure == s_directory) {
      if (state == s_absent) {
        create_directory(*res);
      }
      update_metadata(*res, should);
    } else if (ensure == s_link) {
      auto target = should.lookup<std::string>("target");
      if (!target) {
        *res = error(_("ensure for '{1}' is 'link', but target is not set",
                       name()));
        return res;
      }
      update_target(*res, state, *target);
      update_metadata(*res, should);
    } else if (ensure == s_absent) {
      remove(*res, state, force);
    } else {
      *res = error(_("Illegal ensure value '{1}'", ensure));
    }

    if (*res && res->ok().size() > 0) {
      // If we made any changes, just restat all metadata
      _prov->load(is);
    }
    return res;
  }

  static result<uid_t> owner_to_uid(const std::string& owner) {
    // FIXME: look up with getpwnam
    try {
      return std::stoi(owner);
    } catch (const std::exception& e) {
      return result<uid_t>(error(_("failed to convert '{1}' into a uid [{2}]",
                                   owner, e.what())));
    }
  }

  static result<gid_t> group_to_gid(const std::string& group) {
    // FIXME: look up with getgrnam
    try {
      return std::stoi(group);
    } catch (const std::exception& e) {
      return result<uid_t>(error(_("failed to convert '{1}' into a gid [{2}]",
                                   group, e.what())));
    }
  }

  /*
   * Update owner, group, mode as needed
   */
  void fprov::file_resource::update_metadata(result<changes>& res,
                                            const attr_map& should) {
    if (!res)
      return;

    auto& is = *this;
    changes& chgs = *res;
    fs::path p = name();

    check(chgs, should, { "owner", "group", "mode" });

    if (chgs.exists("owner") || chgs.exists("group")) {
      uid_t uid = -1;
      gid_t gid = -1;
      auto owner = should.lookup<std::string>("owner");
      auto group = should.lookup<std::string>("group");
      if (owner) {
        auto uid_res = owner_to_uid(*owner);
        if (!uid_res) {
          res = uid_res.err();
          return;
        }
        uid = *uid_res;
      }
      if (group) {
        auto gid_res = group_to_gid(*group);
        if (!gid_res) {
          res = gid_res.err();
          return;
        }
        gid = *gid_res;
      }

      errno = 0;
      int r = lchown(name().c_str(), uid, gid);
      if (r < 0) {
        res = error(_("cannot set owner/group [lchown, errno={1}]: {2}",
                      errno, strerror(errno)));
        return;
      }
      if (owner) {
        is["owner"] = should["owner"];
      }
      if (group) {
        is["group"] = should["group"];
      }
    }
    if (chgs.exists("mode")) {
      auto s = should.lookup<std::string>("mode");
      fs::perms mode;

      if (!s) {
        // This should not happen - how did we figure out that we needed to
        // change mode ?
        res = error(_("internal error: should['mode'] is not a string"));
        return;
      }
      try {
        std::size_t pos;
        mode = static_cast<fs::perms>(std::stoi(*s, &pos, 8));
        if (pos != s->length()) {
          res = error(_("mode '{1}' is not a valid octal number", *s));
          return;
        }
      } catch (const std::exception& e) {
        res = error(_("failed to parse mode '{1}': {2}", *s, e.what()));
        return;
      }

      if (mode & ~fs::perms_mask) {
        // FIXME: convert perms_mask to octal
        res = error(_("mode '{1}' sets bits outside of mask {2}",
                      *s, fs::perms_mask));
        return;
      }
      boost::system::error_code ec;
      permissions(p, mode, ec);
      if (ec) {
        res = error(_("failed to change mode to '{1}': {2}",
                      *s, ec.message()));
        return;
      }
      is["mode"] = should["mode"];
    }
  }

  void fprov::file_resource::update_content(result<changes>& res,
                                           const attr_map& should) {
    if (!res)
      return;

    auto content = should.lookup<std::string>("content");
    if (!content) {
      // either, 'content' was not set, which is ok, or its value is something
      // other than a string
      if (should.count("content") > 0) {
        // not sure how that could actually happen
        res = error(_("illegal value for 'content': it must be a string"));
      }
      return;
    }

    // This way of reading/checking/writing file contents will of course
    // not work for large files ...
    boost::nowide::ifstream ifs(name());
    std::stringstream buf;
    if (!ifs.is_open()) {
      res = error(_("Could not open file '{1}' for reading", name()));
      return;
    }
    buf << ifs.rdbuf();
    ifs.close();

    if (buf.str() != *content) {
      boost::nowide::ofstream ofs(name());
      if (!ofs.is_open()) {
        res = error(_("Could not create file '{1}'", name()));
        return;
      }
      ofs << *content;
      ofs.close();

      auto& is = *this;
      changes& chgs = *res;
      is["content"] = *content;
      // FIXME: add some hash of the contents here rather than the real
      // values (???)
      chgs.add("content", *content, buf.str());
    }
  }

  void fprov::file_resource::update_target(result<changes>& res,
                                          std::string& state,
                                          const std::string& target) {
    if (!res)
      return;

    if (state != s_absent) {
      remove(res, state, false);
      if (!res)
        return;
      state = s_absent;
    }

    boost::system::error_code ec;
    fs::create_symlink(target, name(), ec);
    if (ec) {
      res = error(_("failed to create symlink '{1}' with target '{2}': {3}",
                    name(), target, ec.message()));
      return;
    }
    state = s_link;

    auto& is = *this;
    res.ok().add("target", target, is["target"]);
    is["target"] = target;
  }

  // Remove the current file (precise type in 'state') and set 'state' to
  // 'absent'
  void fprov::file_resource::remove(result<changes>& res,
                                   std::string& state,
                                   bool force) {
    if (!res || state == s_absent)
      return;

    if (state == s_directory && !force) {
      res = error(_("file '{1}' is a directory. Can not remove that since force is not set to true"));
      return;
    }
    if (state == s_directory || state == s_file || state == s_link) {
      boost::system::error_code ec;
      fs::remove_all(name(), ec);
      if (ec) {
        res = error(_("failed to remove '{1}': {2}", name(), ec.message()));
        return;
      }
    } else {
      res = error(_("can not remove a '{1}' - unknown file type", state));
      return;
    }
    state = s_absent;
  }

  // Create an empty file
  void fprov::file_resource::create_file(result<changes>& res) {
    if (!res)
      return;
    boost::nowide::ofstream ofs(name());
    if (!ofs.is_open()) {
      res = error(_("Could not create file '{1}'", name()));
    }
    ofs.close();
  }

  void fprov::file_resource::create_directory(result<changes>& res) {
    if (!res)
      return;
    boost::system::error_code ec;
    fs::create_directory(name(), ec);
    if (ec) {
      res = error(_("failed to create directory '{1}': {2}",
                    name(), ec.message()));
    }
  }

  std::string fprov::time_as_iso_string(std::time_t *time) {
    char buf[100];
    strftime(buf, sizeof(buf), iso_8601_format.c_str(), std::localtime(time));
    return std::string(buf);
  }
}
