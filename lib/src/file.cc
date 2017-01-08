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

#include "boost/system/error_code.hpp"
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;
using ftype = fs::file_type;

namespace libral {

  using prov = file_provider;

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

  static const std::array<fdesc, 11> ftype_ensure = {{
      // unknown must be the first entry
      // entries are searched in order so that for duplicate values,
      // the 'canonical' entry must come first
      { ftype::type_unknown,   "absent",    "unknown" },
      { ftype::status_error,   "absent",    "error" },
      { ftype::file_not_found, "absent",    "absent"},
      { ftype::regular_file,   "file",      "file" },
      { ftype::regular_file,   "present",   "file" },
      { ftype::directory_file, "directory", "directory"},
      { ftype::symlink_file,   "link",      "link"},
      { ftype::block_file,     "present",    "block"},
      { ftype::character_file, "present",    "char"},
      { ftype::fifo_file,      "present",    "fifo"},
      { ftype::socket_file,    "present",    "socket"}
    }};

  /* Get a string representation for the given ftype */
  const fdesc& fdesc_from_ftype(const ftype& t) {
    for (auto it = ftype_ensure.begin(); it != ftype_ensure.end(); it++) {
      if (t == it->ftyp)
        return *it;
    }
    return ftype_ensure[0];
  }

  std::vector<std::unique_ptr<resource>> prov::instances() {
    std::vector<std::unique_ptr<resource>> result;

    return result;
  }

  std::unique_ptr<resource> prov::create(const std::string& name) {
    auto shared_this = std::static_pointer_cast<file_provider>(shared_from_this());
    auto cname = fs::canonical(name);
    auto res = new file_resource(shared_this, cname.native());
    return std::unique_ptr<resource>(res);
  }


  boost::optional<std::unique_ptr<resource>>
  prov::find(const std::string &name) {
    auto result = create(name);
    auto& res = *result;

    /* Look up file and fill in attributes */
    boost::system::error_code ec;
    auto st = fs::symlink_status(name, ec);
    if (ec) {
      // FIXME: Be a little more discrimnating here; for example, if we get
      // EACCES, should we really act as if the file doesn't exist ?
      res["ensure"] = "absent";
      return std::move(result);
    }

    res["ensure"] = fdesc_from_ftype(st.type()).ensure;
    res["type"] = fdesc_from_ftype(st.type()).typ;

    if (st.type() == ftype::symlink_file) {
      res["target"] = fs::read_symlink(name).native();
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
      auto mtime = fs::last_write_time(name);
      res["mtime"] = time_as_iso_string(&mtime);
      // At some point, we should make it possible to set the type of
      // checksum to compute. As we do not support other ways to specify
      // content than literally through a 'content' attribute, this is
      // kinda pointless for now
      res["checksum_value"] = std::to_string(mtime);
      res["checksum"] = "mtime";
    }

    find_from_stat(res);

    return std::move(result);
  }

  /* Fill in attributes we can only get via stat(2) and not from
     boost::filesystem, most notably owner, group, and ctime */
  void prov::find_from_stat(resource &res) {
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

  std::unique_ptr<result<changes>>
  prov::file_resource::update(const attr_map& should) {
    auto r = new result<changes>(not_implemented_error());
    return std::unique_ptr<result<changes>>(r);
  }

  std::string prov::time_as_iso_string(std::time_t *time) {
    char buf[100];
    strftime(buf, sizeof(buf), iso_8601_format.c_str(), std::localtime(time));
    return std::string(buf);
  }
}
