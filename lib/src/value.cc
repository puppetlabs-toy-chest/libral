#include <libral/value.hpp>

#include <boost/lexical_cast.hpp>

#include <sstream>

namespace json = leatherman::json_container;
using json_container = json::JsonContainer;
using json_keys = std::vector<json::JsonContainerKey>;

namespace libral {

  const value value::none = value();

  value& value::operator=(char const* string)
  {
    value_base::operator=(std::string(string));
    return *this;
  }

  value::value(char const* string) :
    value_base(std::string(string)) { }

  /* to_string */

  struct to_string_visitor : boost::static_visitor<std::string> {
    result_type operator()(const boost::none_t& n) const {
      return "(none)";
    }

    result_type operator()(const bool& b) const {
      return b ? "true" : "false";
    }

    result_type operator()(const std::string& s) const {
      return s;
    }

    result_type operator()(const array& ary) const {
      std::stringstream buf;
      bool first = true;
      buf << "[";
      for (auto s : ary) {
        if (! first)
          buf << ", ";
        first = false;
        buf << "'" << s << "'";
      }
      buf << "]";
      return buf.str();
    }
  };

  std::string value::to_string() const {
    return boost::apply_visitor(to_string_visitor(), *this);
  }

  struct to_json_visitor : boost::static_visitor<void> {
    to_json_visitor(json_container& js, const json_keys& key)
      : _js(js), _key(key) { }

    void operator()(const boost::none_t& n) const {
      // FIXME: how do you set something to JSON 'null' ?
      _js.set<std::string>(_key, std::string("null"));
    }

    void operator()(const bool& b) const {
      _js.set<bool>(_key, b);
    }

    void operator()(const std::string& s) const {
      _js.set<std::string>(_key, s);
    }

    void operator()(const array& ary) const {
      _js.set<array>(_key, ary);
    }

    json_container& _js;
    const json_keys& _key;
  };

  void value::to_json(json_container& js, const json_keys& key) const {
    auto visitor = to_json_visitor(js, key);
    boost::apply_visitor(visitor, *this);
  }

  /* operator==, operator!= */
  struct equality_visitor : boost::static_visitor<bool> {
    // For some reason, leaving this to the templated operator leads to
    // infinite recursion, at least with gcc 6.3.1 when left and right are
    // value::none
    result_type
    operator()(boost::none_t const& left, boost::none_t const& right) const {
      return true;
    }

    template <typename T, typename U>
    result_type operator()(T const&, U const&) const {
      // Not the same type
      return false;
    }

    template <typename T>
    result_type operator()(T const& left,T const& right) const {
      return left == right;
    }
  };

  bool operator==(value const& left, value const& right) {
    //return (static_cast<value_base>(left) == static_cast<value_base>(right));
    return boost::apply_visitor(equality_visitor(), left, right);
  }

  bool operator!=(value const& left, value const& right)  {
    //return (static_cast<value_base>(left) != static_cast<value_base>(right));
    return ! boost::apply_visitor(equality_visitor(), left, right);
  }

  struct to_quoted_string_visitor : boost::static_visitor<std::string> {
    result_type operator()(const boost::none_t& n) const {
      return "(none)";
    }

    result_type operator()(const bool& b) const {
      return b ? "true" : "false";
    }

    result_type operator()(const std::string& s) const {
      return "'" + s + "'";
    }

    result_type operator()(const array& ary) const {
      std::stringstream buf;
      bool first = true;
      buf << "[";
      for (auto s : ary) {
        if (! first)
          buf << ", ";
        first = false;
        // s is a std::string, not a value, so we need to quote it
        buf << "'" << s << "'";
      }
      buf << "]";
      return buf.str();
    }
  };

  /* operator<< */
  std::ostream& operator<<(std::ostream& os, value const& val) {
    os << boost::apply_visitor(to_quoted_string_visitor(), val);
    return os;
  }
}
