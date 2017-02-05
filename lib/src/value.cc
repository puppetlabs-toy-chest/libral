#include <libral/value.hpp>

#include <boost/lexical_cast.hpp>

namespace libral {

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
  };

  std::string value::to_string() const {
    return boost::apply_visitor(to_string_visitor(), *this);
  }

  /* operator==, operator!= */
  struct equality_visitor : boost::static_visitor<bool> {
    /**
     * Compares two strings.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true if the strings are equal (case insensitive) or false if not.
     */
    template <typename T>
    result_type operator()(T const& left,T const& right) const {
      return left == right;
    }

    /**
     * Compares two different value types.
     * @tparam T The left hand type.
     * @tparam U The right hand type.
     * @return Always returns false since two values of different types cannot be equal.
     */
    template <typename T, typename U>
    result_type operator()(T const&, U const&) const {
      // Not the same type
      return false;
    }
  };


  bool operator==(value const& left, value const& right) {
    return boost::apply_visitor(equality_visitor(), left, right);
  }


  bool operator!=(value const& left, value const& right)  {
    return ! boost::apply_visitor(equality_visitor(), left, right);
  }

  /* operator<< */
  std::ostream& operator<<(std::ostream& os, value const& val) {
    os << val.to_string();
    return os;
  }
}
