#include <libral/augeas.hpp>

#include <sstream>

#include <boost/nowide/iostream.hpp>

#include <leatherman/locale/locale.hpp>

using namespace leatherman::locale;

namespace libral { namespace augeas {

  handle::handle(const callback& reader, const callback &writer)
    : _reader(reader), _writer(writer) {
    // We do not report errors from aug_init. That's bad. Very bad.

    // If we have a reader or writer, make sure we can't possibly read or
    // write ourselves with aug_load or aug_save
    const char *root = (_reader != nullptr || _writer != nullptr)
      ? "/dev/null" : NULL;
    _augeas = aug_init(root, nullptr, AUG_NO_MODL_AUTOLOAD);

    // Set up default reader and writer
    if (_reader == nullptr) {
      _reader = [this](::augeas *aug) { aug_load(aug); };
    }
    if (_writer == nullptr) {
      _writer = [this](::augeas *aug) { aug_save(aug); };
    }
  };

  result<void>
  handle::include(const std::string& lens, const std::string& glob) {
    aug_transform(_augeas, lens.c_str(), glob.c_str(), 0);
    return check_error();
  }

  result<void> handle::load(void) {
    _reader(this->_augeas);
    return check_error();
  }

  result<void> handle::save(void) {
    _writer(this->_augeas);

    auto r = check_error();
    if (!r) return r;

    auto matches = match("/augeas/files//error");
    if (!matches) return matches.err();

    if (matches.ok().size() > 0) {
      static const std::string augeas_files = "/augeas/files";

      std::ostringstream os;
      bool first = true;
      for (const auto &e : matches.ok()) {
        auto line     = e["line"];
        auto char_pos = e["char"];
        auto lens     = e["lens"];
        auto last     = e["lens/last_matched"];
        auto next     = e["lens/next_not_matched"];
        auto msg      = e["message"];
        auto path     = e["path"];
        auto kind     = *e.get();

        auto filename = e.path();
        {
          // Strip off '/augeas/files' at the beginning and '/error' at the end
          filename.erase(0, augeas_files.size());
          auto pos = filename.rfind("/error");
          if (pos != std::string::npos) {
            filename.erase(pos);
          }
        }

        if (!first)
          os << std::endl;
        first = false;

        if (line && line.ok()) {
          os << _("Error in {1}:{2}.{3} ({4})",
                  filename, **line, **char_pos, *kind) << std::endl;
        } else if (path && path.ok()) {
          os << _("Error in {1} at node {2} ({3})", filename, **path, *kind)
             << std::endl;
        } else {
          os << _("Error in {1} ({2})", filename, *kind) << std::endl;
        }

        if (msg && msg.ok())
          os << "  " << *msg.ok() << std::endl;
        if (lens && lens.ok())
          os << _("  Lens: {1}", *lens.ok()) << std::endl;
        if (last && last.ok())
          os << _("    Last matched: {1}\n", *last.ok()) << std::endl;
        if (next && next.ok())
          os << _("    Next (no match): {1}\n", *next.ok()) << std::endl;
      }
      return error(os.str());
    }
    return result<void>();
  }

  result<std::vector<node>>
  handle::match(const std::string& pathx) {
    std::vector<node> nodes;
    char **matches;

    auto count = aug_match(_augeas, pathx.c_str(), &matches);
    auto r = check_error();
    if (!r) return r.err();

    for (int i=0; i < count; i++) {
      nodes.push_back(make_node(std::string(matches[i])));
      free(matches[i]);
    }
    free(matches);
    return nodes;
  }

  result<void>
  handle::set(const std::string& path, const std::string& value) {
    aug_set(_augeas, path.c_str(), value.c_str());
    return check_error();
  }

  result<void> handle::clear(const std::string& path) {
    aug_set(_augeas, path.c_str(), NULL);
    return check_error();
  }

  result<void> handle::rm(const std::string& path) {
    aug_rm(_augeas, path.c_str());
    return check_error();
  }

  result<boost::optional<std::string>>
  handle::get(const std::string& pathx) const {
    const char *value;

    aug_get(_augeas, pathx.c_str(), &value);
    auto r = check_error();
    if (!r) return r.err();

    if (value == NULL) {
      return result<boost::optional<std::string>>(boost::none);
    } else {
      return boost::optional<std::string>(std::string(value));
    }
  }

  node handle::make_node(const std::string& path) {
    return node(shared_from_this(), path);
  }

  node handle::make_node_seq_next(const std::string& path) {
    std::ostringstream os;
    os << path;
    if (path.back() != '/')
      os << '/';
    os << "0" << ++_seq;
    return make_node(os.str());
  }

  result<void> handle::check_error() const {
    if (aug_error(_augeas) != AUG_NOERROR) {
      std::ostringstream os;
      const char *msg    = aug_error_message(_augeas);
      const char *minor  = aug_error_minor_message(_augeas);
      const char *detail = aug_error_details(_augeas);

      os << msg;
      if (minor != nullptr)
        os << ": " << minor;
      if (detail != nullptr)
        os << ": " << detail;
      os << std::endl;
      return error(os.str());
    }
    return result<void>();
  }

  void handle::print(const std::string& path) {
    aug_print(_augeas, stdout, path.c_str());
  }

} }
