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

    bool exists(const std::string &attr);
  protected:
    changes() {}
  private:
    friend class context;
    friend class provider;
    std::vector<change> _changes;
  };


  class context {
  public:
    context() { }

    /**
     * Returns a changes object that can record changes to the resource
     * called NAME
     */
    changes& changes_for(const std::string& name);
  private:
    std::map<std::string, changes> _changes;
  };
}
