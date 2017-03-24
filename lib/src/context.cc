#include <libral/context.hpp>

namespace libral {

  /*
   * implementation for changes
   */

  void changes::add(const std::string &attr, const value &is,
                    const value &was) {
    push_back(change(attr, is, was));
  }

  bool changes::add(const std::string &attr, const update &upd) {
    bool changed = upd.changed(attr);
    if (changed) {
      add(attr, upd.should[attr], upd.is[attr]);
    }
    return changed;
  }

  bool changes::add(const std::vector<std::string> &attrs, const update &upd) {
    bool changed = false;
    for (auto attr : attrs) {
      if (add(attr, upd))
        changed = true;
    }
    return changed;
  }

  bool changes::exists(const std::string &attr) {
    for (auto ch : (*this)) {
      if (ch.attr == attr)
        return true;
    }
    return false;
  }

  std::ostream& operator<<(std::ostream& os, changes const& chgs) {
    for (auto chg: chgs) {
      auto was = chg.was.to_string();
      auto is = chg.is.to_string();
      os << chg.attr << "(" << was << "->" << is << ")" << std::endl;
    }
    return os;
  }


  /*
   * implementation for context
   */
  changes& context::changes_for(const std::string& name) {
    // We do a bit of a dance here since we do not want the
    // changes constructor to be public
    auto it = _changes.find(name);
    if (it == _changes.end()) {
      return _changes.emplace(std::make_pair(name, changes())).first->second;
    } else {
      return it->second;
    }
  }


}
