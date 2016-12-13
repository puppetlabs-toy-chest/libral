#pragma once

#include <boost/variant.hpp>

namespace libral {
  /*
     A poor man's emulation of Rust's Result construct. The basic idea is
     that we want to avoid throwing exceptions and rather force our callers
     to be explicit about how they handle errors.

     One day we might be able to use std::expected but that's still
     experimental
  */

  // The basic error we use everywhere
  struct error {
    error(const std::string &det) : detail(det) {};
    std::string detail;
  };

  /* A result is either an error or whatever we really wanted */
  template <class R>
  class result : boost::variant<error, R> {
    typedef boost::variant<error, R> base;
  public:
    result(R& ok) : base(ok) {};
    result(const R& ok) : base(ok) {};
    result(R&& ok) : base(std::move(ok)) {};
    result(error& err) : base(err) {};
    result(const error& err) : base(err) {};
    result(error&& err) : base(std::move(err)) {};

    boost::optional<R&> ok() {
      if (is_ok()) {
        return boost::get<R>(*this);
      } else {
        return boost::none;
      }
    };

    boost::optional<const R&> ok() const {
      if (is_ok()) {
        return boost::get<R>(*this);
      } else {
        return boost::none;
      }
    };

    boost::optional<error&> err() {
      if (is_err()) {
        return boost::get<error>(*this);
      } else {
        return boost::none;
      }
    };

    boost::optional<const error&> err() const {
      if (is_err()) {
        return boost::get<error>(*this);
      } else {
        return boost::none;
      }
    };

    bool is_ok()  const { return base::which() == 1; }
    bool is_err() const { return base::which() == 0; }

    static std::unique_ptr<result<R>> make_unique() {
      return std::unique_ptr<result<R>>(new result<R>(R()));
    };

    static std::unique_ptr<result<R>> make_unique(const error& e) {
      return std::unique_ptr<result<R>>(new result<R>(e));
    };
  };
}
