#include <libral/augeas.hpp>

#include <boost/nowide/iostream.hpp>

namespace aug {
  node::node(handle& aug, const std::string path) : _aug(aug), _path(path) { }

  std::string node::operator[](const std::string& path) const {
    auto result = _aug.value(_path + "/" + path);
    _aug.check_error();
    return result;
  }

  handle::handle(const char *root,
                 const char *loadpath,
                 unsigned int flags)
    : _augeas(aug_init(root, loadpath, flags)) { }

  void handle::include(const std::string& lens, const std::string& glob) {
    aug_transform(_augeas, lens.c_str(), glob.c_str(), 0);
    check_error();
  }

  void handle::load(void) {
    aug_load(this->_augeas);
  }

  std::vector<node> handle::match(const std::string& pathx) {
    std::vector<node> result;
    char **matches;
    int r = aug_match(_augeas, pathx.c_str(), &matches);
    check_error();

    for (int i=0; i < r; i++) {
      result.push_back(node(*this, std::string(matches[i])));
      free(matches[i]);
    }
    free(matches);
    return result;
  }

  std::string handle::value(const std::string& pathx) const {
    const char *value;

    aug_get(_augeas, pathx.c_str(), &value);
    check_error();
    if (value == NULL) {
      boost::nowide::cout << "got NULL: " << pathx << std::endl;
    }
    return (value == NULL) ? "" : value;
  }

  static std::string to_string(const char *s) {
    if (s == NULL) {
      return "";
    } else {
      return std::string(s);
    }
  }

  void handle::check_error() const {
    if (aug_error(_augeas) != AUG_NOERROR) {
      // @todo lutter 2016-05-10: produce real error messages
      // these will be impossible ot comprehend
      auto msg = to_string(aug_error_message(_augeas)) +
        "\n" + to_string(aug_error_minor_message(_augeas)) +
        "\n" + to_string(aug_error_details(_augeas));
      throw error { msg };
    }
  }
}
