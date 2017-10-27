#pragma once

#include <libral/result.hpp>

extern "C" {
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/string.h>
}

namespace libral {

  /**
   * Simple wrapper to make accessing MRuby routines a bit more memory and
   * type safe.
   */
  struct mruby {
    mruby(mrb_state* mrb0)
      : mrb(std::shared_ptr<mrb_state>(mrb0,
                                       [](mrb_state *mrb) { mrb_close(mrb); }))
    { }

    using value = result<mrb_value>;

    struct RClass *module_get(const std::string& name) {
      return mrb_module_get(mrb.get(), name.c_str());
    }

    value str_new(const std::string& s) {
      return check(mrb_str_new(mrb.get(), s.c_str(), s.length()));
    }

    value funcall(mrb_value obj, const std::string& name, mrb_value arg) {
      return check(mrb_funcall(mrb.get(), obj, name.c_str(), 1, arg));
    }

    value funcall(struct RClass *rc, const std::string& name, mrb_value arg) {
      return check(mrb_funcall(mrb.get(), mrb_obj_value(rc), name.c_str(), 1, arg));
    }

    value funcall(struct RClass *rc, const std::string& name,
                  const std::string& arg) {
      mrb_value s = mrb_str_new(mrb.get(), arg.c_str(), arg.length());
      if (mrb->exc != nullptr) {
        return error(_("Failed to make string"));
      }
      return check(mrb_funcall(mrb.get(), mrb_obj_value(rc), name.c_str(), 1, s));
    }

    void p(mrb_value v) { mrb_p(mrb.get(), v); }

    mrb_int ary_len(mrb_value ary) {
      return mrb_ary_len(mrb.get(), ary);
    }

    mrb_value hash_get(mrb_value hash, mrb_value key) {
      return mrb_hash_get(mrb.get(), hash, key);
    }

    mrb_value hash_get(mrb_value hash, const std::string& key) {
      mrb_value k = mrb_str_new(mrb.get(), key.c_str(), key.length());
      return mrb_hash_get(mrb.get(), hash, k);
    }

    mrb_value hash_keys(mrb_value hash) {
      return mrb_hash_keys(mrb.get(), hash);
    }

    bool bool_p(mrb_value v) {
      // Surprisingly involved - can't find a simpler way to check
      // whether v is the actual boolean value true or false
      struct RClass *rc = mrb_class(mrb.get(), v);
      return (rc == mrb->false_class || rc == mrb->true_class);
    }

    /**
     * Returns the string value at key in hash. Returns dflt when hash is
     * not a Hash, or does not contain a string at key
     */
    std::string hash_get_string(mrb_value hash, const std::string& key,
                                const std::string& dflt = "") {
      if (! mrb_hash_p(hash)) {
        return dflt;
      }
      mrb_value k = mrb_str_new(mrb.get(), key.c_str(), key.length());
      mrb_value v = mrb_hash_get(mrb.get(), hash, k);
      if (mrb_nil_p(v) || ! mrb_string_p(v)) {
        return dflt;
      } else {
        return std::string(RSTRING_PTR(v));
      }
    }

    std::string as_string(mrb_value str, const std::string& dflt = "") {
      if (mrb_string_p(str)) {
        return std::string(RSTRING_PTR(str));
      } else {
        return dflt;
      }
    }

    value check(const mrb_value& v) {
      if (has_exc()) {
        return exc_as_error();
      }
      return v;
    }

    bool has_exc() {
      return (mrb->exc != nullptr);
    }

    error exc_as_error() {
      if (mrb->exc != nullptr) {
        mrb_value val = mrb_funcall(mrb.get(), mrb_obj_value(mrb->exc),
                                    "message", 0);
        mrb->exc = nullptr;
        return error(std::string(RSTRING_PTR(val)));
      } else {
        return error("internal error: exc_as_error called even though there was no exception");
      }
    }

    static result<mruby> open() {
      mrb_state *mrb = mrb_open();
      if (mrb == NULL) {
        return error(_("Failed to open MRuby"));
      }
      return mruby(mrb);
    }

    std::shared_ptr<mrb_state> mrb;
  };
}
