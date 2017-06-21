#pragma once

#include <string>
#include <map>

#include <libral/value.hpp>
#include <libral/resource.hpp>

namespace libral {

  /* Record the change of attribute ATTR from WAS to IS */
  struct change {
    change(const std::string &a, const value &i, const value &w = boost::none)
      : attr(a), is(i), was(w) {};
    std::string attr;
    value is;
    value was;
  };

  /* A list of changes */
  class changes : public std::vector<change> {
  public:
    void add(const std::string &attr, const value &is,
             const value &was = boost::none);

    /**
     * Adds a change entry for ATTR if UPD.IS[ATTR] has a different value
     * from UPD.SHOULD[ATTR]. Returns true if they differ, and false if
     * they are the same.
     */
    bool add(const std::string &attr, const update &upd);

    /**
     * Adds a change entry for each of the ATTRS where UPD.IS has a
     * different value from UPD.SHOULD. Returns true if any change was
     * detected, and false if none of the is and should values for ATTRS
     * differ.
     */
    bool add(const std::vector<std::string> &attrs, const update &upd);

    /**
     * Adds changes for all attributes where UPD.IS differs from UPD.SHOULD
     * and for which no changes have been recorded yet. Returns true if any
     * changes were detected.
     */
    bool maybe_add(const update& upd);

    bool exists(const std::string &attr);
  protected:
    changes() {}
  private:
    friend class context;
    std::vector<change> _changes;
  };

  // Forward declaration to avoid include loops
  class provider;

  class context {
  public:
    context(const std::shared_ptr<provider>& prov) : _prov(prov) { }

    /**
     * Logs a line which may start with a log level prefix '<LEVEL>:'.
     * Possible levels are DEBUG, INFO, WARN, or ERROR (case
     * insensitively). If the line starts with a level, the rest of the
     * line is logged at that level. If it does not start with a level, it
     * is logged at level WARN.
     */
    void log_line(const std::string& line);

    template <typename... TArgs>
    void log_debug(std::string const& fmt, TArgs... args) {
      log_debug(leatherman::locale::format(fmt, std::forward<TArgs>(args)...));
    }

    void log_debug(std::string const& msg);

    /**
     * Returns a changes object that can record changes to the resource
     * called NAME
     */
    changes& changes_for(const std::string& name);

    /**
     * Returns true if changes_for has been called before with the same
     * name.
     */
    bool have_changes(const std::string& name);

    libral::error error(const std::string& msg) const;

    /**
     * For any entry in names for which we do not have a resource in rsrcs,
     * add one with ensure set to 'absent'
     */
    void add_absent(std::vector<resource>& rsrcs,
                    const std::vector<std::string>& names);
  private:
    const std::shared_ptr<provider> _prov;
    std::map<std::string, changes> _changes;
  };
}
