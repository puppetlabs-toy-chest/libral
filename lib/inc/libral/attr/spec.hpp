#pragma once

#include <libral/result.hpp>
#include <libral/value.hpp>
#include <vector>
#include <ostream>

namespace libral { namespace attr {

  /**
   * The kind of a provider attribute: read, write, or read-write
   */
  class kind {
  public:
    enum class tag { r, w, rw };

    kind(tag t) : _tag(t) { };
    tag get_tag() const { return _tag; }
    bool is(tag t) const { return _tag == t; }
    static result<kind> create(const std::string& tag_str);
  private:
    tag _tag;
  };

  std::ostream& operator<<(std::ostream& os, kind const& k);

  /**
   * The types we currently support for attributes. Their main point is to
   * read a string and convert it into a value
   */
  struct string_type {
    result<value> read_string(const std::string& s) const;
  };

  std::ostream& operator<<(std::ostream& os, string_type const& st);

  struct boolean_type {
    result<value> read_string(const std::string& s) const;
  };

  std::ostream& operator<<(std::ostream& os, boolean_type const& bt);

  /* Read an array in the form '[s1, s2, s3]'. No support for quoting
     currently */
  struct array_type {
    result<value> read_string(const std::string& s) const;
  };

  std::ostream& operator<<(std::ostream& os, array_type const& at);

  class enum_type {
  public:
    enum_type(std::vector<std::string>&& options)
      : _options(options) { };
    result<value> read_string(const std::string& s) const;
    const std::vector<std::string>& options() const { return _options; }
  private:
    std::vector<std::string> _options;
  };

  std::ostream& operator<<(std::ostream& os, enum_type const& et);

  /**
   * A generic data type, encompassing all data types we support
   */
  using data_type = boost::variant<
    string_type,
    boolean_type,
    array_type,
    enum_type
  >;

  /**
   * Represents the specification of an individual attribute
   */
  class spec {
  public:
    /**
     * Read string s and return the corresponding value
     */
    result<value> read_string(const std::string& s) const;

    const std::string& name() const { return _name; }
    const std::string& desc() const { return _desc; }
    const data_type& get_data_type() const { return _data_type; }
    const kind& get_kind() const { return _kind; }

    /**
     * Create a spec from a textual description
     *
     * This function parses textual descriptions of type and kind, and
     * either returns an attribute spec, or an indication of the error it
     * encountered
     */
    static result<spec> create(const std::string& name,
                               const std::string& desc,
                               const std::string& type,
                               const std::string& kind);
  private:
    /**
     * Constructs an attr_spec.
     * @param name The name of the attribute
     * @param desc The description of the attribute
     * @param data_type The data type of the attribute
     * @param kind The kind (r/w/rw) of the attribute
     */
    spec(const std::string& name,
         const std::string& desc,
         data_type& data_type,
         kind& kind)
      : _name(name), _desc(desc), _data_type(data_type), _kind(kind) { };

    std::string _name;
    std::string _desc;
    data_type   _data_type;
    kind        _kind;
  };

} }  // namespace libral::attr
