#include <libral/attr/spec.hpp>

#include <algorithm>

#include <boost/algorithm/string.hpp>

#include <leatherman/logging/logging.hpp>

#include <libral/result.hpp>

using namespace leatherman::locale;
namespace json = leatherman::json_container;
using json_container = json::JsonContainer;
using json_keys = std::vector<json::JsonContainerKey>;

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

  std::ostream& operator<<(std::ostream& os, kind const& k) {
    if (k.is(kind::tag::r)) {
      os << "r";
    } else if (k.is(kind::tag::w)) {
      os << "w";
    } else if (k.is(kind::tag::rw)) {
      os << "rw";
    } else {
      throw std::logic_error("internal error: unknown kind");
    }
    return os;
  }

  result<value> string_type::read_string(const std::string& s) const {
    return value(s);
  }

  std::ostream& operator<<(std::ostream& os, string_type const& st) {
    os << "string";
    return os;
  }

  result<value> boolean_type::read_string(const std::string& s) const {
    bool t = boost::iequals(s, "true");
    bool f = boost::iequals(s, "false");
    if (!t && !f) {
      return error(_("invalid boolean: must be either 'true' or 'false'"));
    }
    return value(t);
  }

  std::ostream& operator<<(std::ostream& os, boolean_type const& bt) {
    os << "boolean";
    return os;
  }

  result<value> array_type::read_string(const std::string& s) const {
    array ary;
    if (! s.empty()) {
      boost::split(ary, s, boost::is_any_of(","));
      for (auto& elt : ary) {
        boost::trim(elt);
        if (elt.empty()) {
          return error(_("bad array: entries in '{1}' can not be blank", s));
        }
      }
    }
    return value(ary);
  }

  std::ostream& operator<<(std::ostream& os, array_type const& at) {
    os << "array[string]";
    return os;
  }

  result<value> enum_type::read_string(const std::string& s) const {
    if (std::find(_options.cbegin(), _options.cend(), s) != _options.cend()) {
      return value(s);
    } else {
      return error(_("value '{1}' is not a legal value for this enum", s));
    }
  }

  std::ostream& operator<<(std::ostream& os, enum_type const& et) {
    os << "enum[";
    auto first = true;
    for (auto o : et.options()) {
      if (!first) {
        os << ", ";
      }
      first = false;
      os << o;
    }
    os << "]";
    return os;
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


  struct from_json_visitor : boost::static_visitor<result<value>> {
    from_json_visitor(const json_container& json, const json_keys& key)
      : _json(json), _key(key) { };

    // FIXME: this needs to be specialized by the data_type of the
    // attribute, i.e., we want to make sure we don't read a value into,
    // e.g., an enum attribute, that is not part of the enum
    template <typename T>
    result_type operator()(const T& type) const {
      switch (_json.type(_key)) {
      case json::Null:
        return value(boost::none);
        break;
      case json::Array:
        return value(_json.get<std::vector<std::string>>(_key));
        break;
      case json::String:
        return value(_json.get<std::string>(_key));
        break;
      case json::Int:
        return value(_json.get<int>(_key));
        break;
      case json::Bool:
        return value(_json.get<bool>(_key));
        break;
      default:
        std::ostringstream os;
        bool first = true;
        for (const auto& k : _key) {
          if (!first)
            os << ".";
          first = false;
          os << k;
        }
        return error(_("can not convert data type for {1}", os.str()));
      }
    }

  private:
    const json_container& _json;
    const json_keys& _key;
  };

  result<value>
  spec::from_json(const json_container& json,
                  const json_keys& key) const {
    auto visitor = from_json_visitor(json, key);
    return boost::apply_visitor(visitor, _data_type);
  }


  result<spec> spec::create(const std::string& name,
                            const std::string& desc,
                            const std::string& type_str,
                            const std::string& kind_str) {

    static const std::string s_string_array = "array[string]";
    static const std::string s_enum = "enum";

    auto kind = kind::create(kind_str);
    if (!kind)
      return kind.err();

    std::string ts = type_str;
    ts.erase(std::remove_if(ts.begin(), ts.end(),
                            [](unsigned char x){return std::isspace(x);}),
             ts.end());

    attr::data_type dt;
    if (ts == "boolean") {
      dt = boolean_type();
    } else if (ts == "string") {
      dt = string_type();
    } else if (ts == s_string_array) {
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
