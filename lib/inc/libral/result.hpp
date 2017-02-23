#pragma once

#include <ostream>
#include <memory>

#include <boost/optional.hpp>

namespace libral {
  /*
     A poor man's emulation of Rust's Result construct. The basic idea is
     that we want to avoid throwing exceptions and rather force our callers
     to be explicit about how they handle errors.

     One day we might be able to use std::expected but that's still
     experimental

     We would really like to use boost::variant here, but that runs into
     trouble along the lines of
     https://svn.boost.org/trac/boost/ticket/7120 so we do it the
     old-fashioned way with a lot more boilerplate code
  */

  // The basic error we use everywhere
  struct error {
    error(const std::string &det) : detail(det) {};
    std::string detail;
  };

  // Indication that something is not implemented
  struct not_implemented_error : public error {
    not_implemented_error() : error("not implemented") { }
  };

  /**
   * Immediately return if res is an error
   */
#define err_ret(res)                                         \
  do {                                                       \
    decltype(res) _x = (res);                                \
    if (_x.is_err()) return _x.err();                        \
  } while(0)

  /* A result is either an error or whatever we really wanted */
  template <class R>
  class result {
  private:
    enum class tag { err, ok };
  public:
    result() : result(R()) {};
    result(R& ok) : _tag(tag::ok), _ok(ok) {};
    result(const R& ok) : _tag(tag::ok), _ok(ok) {};
    result(R&& ok) : _tag(tag::ok), _ok(std::move(ok)) {};
    result(error& err) : _tag(tag::err), _err(err) {};
    result(const error& err) : _tag(tag::err), _err(err) {};
    result(error&& err) : _tag(tag::err), _err(std::move(err)) {};

    result(const result& other) { init(other); }
    result(result&& other) { init(std::move(other)); }

    result& operator=(const result& other) {
      assign(other);
      return *this;
    }

    result& operator=(result&& other) {
      assign(other);
      return *this;
    }

    result& operator=(const error& err) {
      assign(err);
      return *this;
    }

    ~result() {
      if (is_ok()) {
        _ok.~R();
      } else {
        _err.~error();
      }
    }

    /**
     * Returns true if the result is ok
     */
    operator bool() const {
      return _tag == tag::ok;
    }

    /**
     * Returns the ok value.
     *
     * @throws std::logic_error if the result is an error and not ok
     */
    R& ok() {
      if (is_ok()) {
        return this->_ok;
      } else {
        std::string msg = "attempt to get ok value from err: ";
        msg += _err.detail;
        throw std::logic_error(msg);
      }
    };

    /**
     * Returns the ok value.
     *
     * @throws std::logic_error if the result is an error and not ok
     */
    const R& ok() const {
      if (is_ok()) {
        return this->_ok;
      } else {
        std::string msg = "attempt to get ok value from err: ";
        msg += _err.detail;
        throw std::logic_error(msg);
      }
    };

    /**
     * Returns the err value.
     *
     * @throws std::logic_error if the result is ok and not an error
     */
    error& err() {
      if (is_err()) {
        return this->_err;
      } else {
        throw std::logic_error("tried to get error from ok result");
      }
    };

    /**
     * Returns the err value.
     *
     * @throws std::logic_error if the result is ok and not an error
     */
    const error& err() const {
      if (is_err()) {
        return this->_err;
      } else {
        throw std::logic_error("tried to get error from ok result");
      }
    };

    /**
     * Returns true if the result is ok
     */
    bool is_ok()  const { return _tag == tag::ok; }

    /**
     * Returns true if the result is an error
     */
    bool is_err() const { return _tag == tag::err; }

    /**
     * Returns true if the result is an error
     */
    bool operator!() const noexcept { return is_err(); }

    /**
     * Returns the ok value.
     *
     * @throws std::logic_error if the result is an error and not ok
     */
    R& operator*() { return ok(); }

    /**
     * Returns the const ok value.
     *
     * @throws std::logic_error if the result is an error and not ok
     */
    const R& operator*() const { return ok(); }

    /**
     * Returns a pointer to the ok value.
     *
     * @throws std::logic_error if the result is an error and not ok
     */
    R* operator->() { return &ok(); }

    /**
     * Returns a const pointer to the ok value.
     *
     * @throws std::logic_error if the result is an error and not ok
     */
    const R* operator->() const { return &ok(); }

    static std::unique_ptr<result<R>> make_unique() {
      return std::unique_ptr<result<R>>(new result<R>(R()));
    };

    static std::unique_ptr<result<R>> make_unique(const error& e) {
      return std::unique_ptr<result<R>>(new result<R>(e));
    };
  private:

    /* Assign by copy */
    void assign(const error& err) {
      if (_tag == tag::ok) {
        _ok.~R();
        new (&_err) error(err);
      } else {
        _err = err;
      }
      _tag = tag::err;
    }

    void assign(const R& ok) {
      if (_tag == tag::ok) {
        _ok = ok;
      } else {
        _err.~error();
        new (&_ok) R(ok);
      }
      _tag = tag::ok;
    }

    void assign(const result& other) {
      if (other._tag == tag::ok) {
        assign(other._ok);
      } else {
        assign(other._err);
      }
    }

    void init(const result& other) {
      _tag = other._tag;
      if (_tag == tag::ok) {
        new (&_ok) R(other._ok);
      } else {
        new (&_err) error(other._err);
      }
    }

    /* Assign by move */
    void assign(error&& err) {
      if (_tag == tag::ok) {
        _ok.~R();
        new (&_err) error(std::move(err));
      } else {
        _err = std::move(err);
      }
      _tag = tag::err;
    }

    void assign(R&& ok) {
      if (_tag == tag::ok) {
        _ok = std::move(ok);
      } else {
        _err.~error();
        new (&_ok) R(std::move(ok));
      }
      _tag = tag::ok;
    }

    void assign(result&& other) {
      if (other._tag == tag::ok) {
        assign(std::move(other._ok));
      } else {
        assign(std::move(other._err));
      }
    }

    void init(result&& other) {
      _tag = other._tag;
      if (_tag == tag::ok) {
        new (&_ok) R(std::move(other._ok));
      } else {
        new (&_err) error(std::move(other._err));
      }
    }

    tag _tag;
    union {
      error _err;
      R     _ok;
    };
  };

  template<typename T>
  std::ostream& operator<<(std::ostream& os, result<T> const& res) {
    if (res.is_ok()) {
      os << "tag:ok";
    } else if (res.is_err()) {
      os << "tag:err " << (*res.err()).detail;
    } else {
      os << "tag:???";
    }
    return os;
  }

  /* Specialication for void */
  template <>
  class result<void> {
  public:
    result() : _err(boost::none) {};
    result(error& err) : _err(err) {};
    result(const error& err) : _err(err) {};
    result(error&& err) : _err(std::move(err)) {};

    /**
     * Returns true if the result is ok
     */
    operator bool() const { return (bool) _err; }

    /**
     * Returns the err value.
     *
     * @throws std::logic_error if the result is ok and not an error
     */
    error& err() {
      if (is_err()) {
        return *_err;
      } else {
        throw std::logic_error("tried to get error from ok result");
      }
    };

    /**
     * Returns the err value.
     *
     * @throws std::logic_error if the result is ok and not an error
     */
    const error& err() const {
      if (is_err()) {
        return *_err;
      } else {
        throw std::logic_error("tried to get error from ok result");
      }
    };

    /**
     * Returns true if the result is ok
     */
    bool is_ok()  const { return ! _err; }

    /**
     * Returns true if the result is an error
     */
    bool is_err() const { return (bool) _err; }

    /**
     * Returns true if the result is an error
     */
    bool operator!() const noexcept { return is_err(); }

  private:
    boost::optional<error> _err;
  };
}
