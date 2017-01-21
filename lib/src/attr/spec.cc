#include <libral/attr/spec.hpp>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <leatherman/logging/logging.hpp>

#include <libral/result.hpp>

using namespace leatherman::locale;

namespace libral { namespace attr {

  result<kind> kind::create(const std::string& tag_str) {
    if (tag_str == "r") {
      return kind(tag::r);
    } else if (tag_str == "w") {
      return kind(tag::w);
    } else if (tag_str == "rw") {
      return kind(tag::rw);
    } else {
      return error(_("unknown kind {1}", tag_str));
    }
  }

  result<value> string_type::read_string(const std::string& s) const {
    return value(s);
  }

  result<value> boolean_type::read_string(const std::string& s) const {
    return value(boost::iequals(s, "true"));
  }

  result<value> array_type::read_string(const std::string& s) const {
    return not_implemented_error();
  }

  result<value> enum_type::read_string(const std::string& s) const {
    if (std::find(_options.cbegin(), _options.cend(), s) != _options.cend()) {
      return value(s);
    } else {
      return error(_("value '{1}' is not a legal value for this enum", s));
    }
  }

  /*
   * attr::spec
   */
  struct read_string_visitor : boost::static_visitor<result<value>> {
    read_string_visitor(const std::string& s): _s(s) { };

    template <typename T>
    result_type operator()(const T& type) const {
      return type.read_string(_s);
    }
  private:
    const std::string& _s;
  };

  result<value> spec::read_string(const std::string& s) const {
    return boost::apply_visitor(read_string_visitor(s), _data_type);
  }

  result<spec> spec::create(const std::string& name,
                            const std::string& desc,
                            const std::string& type_str,
                            const std::string& kind_str) {

    // FIXME: we would like to use std::regex here. But that is busted in
    // gcc 4.8, and we are stuck on that for CentOS 6
    static const boost::regex array_rx("array\\s*\\[\\s*string\\s*\\]\\s*");
    static const std::string s_enum = "enum";

    auto kind = kind::create(kind_str);
    if (!kind)
      return kind.err();

    std::string ts = type_str;
    boost::trim(ts);

    data_type dt;
    if (ts == "boolean") {
      dt = boolean_type();
    } else if (ts == "string") {
      dt = string_type();
    } else if (boost::regex_match(ts, array_rx)) {
      dt = array_type();
    } else if (boost::starts_with(ts, s_enum)) {
      /* Take 'enum[<option> (, <option>)*] apart */
      auto option_start = std::find(ts.cbegin(), ts.cend(), '[');
      auto option_end = std::find(option_start+1, ts.cend(), ']');
      if (option_start+1 >= ts.cend() || option_end == ts.cend()) {
        return error(_("bad enum type: must be 'enum[<option> (,<option>)*]' but is {1}", type_str));
      }
      std::vector<std::string> options;
      // FIXME: is there any way to get a slice, rather than a copy ?
      auto options_str = std::string(option_start + 1, option_end);
      boost::split(options, options_str, boost::is_any_of(","));
      if (options.empty()) {
        return error(_("bad enum type: must be 'enum[<option> (,<option>)*]' but {1} lists no options", type_str));
      }
      for (auto& o : options) {
        boost::trim(o);
        if (o.empty()) {
          return error(_("bad enum type: one option is the empty string"));
        }
        for (auto p = o.begin(); p != o.end(); p++) {
          if (! std::islower(*p) && ! std::isdigit(*p)) {
            return error(_("bad option '{1}' in enum type: options must be lower case characters or digits", o));
          }
        }
      }
      dt = enum_type(std::move(options));
    } else {
      return error(_("unknown type '{1}'", type_str));
    }
    return spec(name, desc, dt, *kind);
  }
} }
