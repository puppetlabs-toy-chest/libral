#include "libral.h"

// @todo lutter 2017-04-25: Something in ruby.h defines the '_' macro which
// collides with Leatherman's '_' macro. We need to figure that out and
// properly fix that.
#undef _

#include <memory>
#include <libral/ral.hpp>

VALUE rb_mLibral;
VALUE rb_cProvider, rb_cRal;
VALUE rb_cResource, rb_cChange, rb_cUpdate;
VALUE rb_eError, rb_eProviderError;

namespace lib = libral;

// Descriptor of storing a pointer to the ral instance
// in a RUby object
static void free_ral(void *vral) {
  delete (std::shared_ptr<lib::ral>*) vral;
}

static const rb_data_type_t ral_data_type = {
    "libral::ral",
    {nullptr, free_ral, nullptr},
    nullptr, nullptr,
    RUBY_TYPED_FREE_IMMEDIATELY,
};


static void free_type(void *type) {
  delete (lib::type*) type;
}

static const rb_data_type_t provider_data_type = {
    "libral::ral",
    {nullptr, free_type, nullptr},
    nullptr, nullptr,
    RUBY_TYPED_FREE_IMMEDIATELY,
};

struct to_ruby_visitor : boost::static_visitor<VALUE> {
  result_type operator()(const boost::none_t& n) const {
    return Qnil;
  }

  result_type operator()(const bool& b) const {
    return b ? Qtrue : Qfalse;
  }

  result_type operator()(const std::string& s) const {
    return rb_str_new_cstr(s.c_str());
  }

  result_type operator()(const libral::array& ary) const {
    VALUE rb_ary = rb_ary_new();
    for (const auto& s : ary) {
      rb_ary_push(rb_ary, rb_str_new_cstr(s.c_str()));
    }
    return rb_ary;
  }
};

VALUE
value_to_ruby(const lib::value &val) {
  auto visitor = to_ruby_visitor();
  return boost::apply_visitor(visitor, val);
}

lib::value
value_from_ruby(VALUE val) {
  // FIXME: properly convert ruby -> value
  if (TYPE(val) == T_STRING) {
    return lib::value(StringValueCStr(val));
  } else if (TYPE(val) == T_ARRAY) {
    std::vector<std::string> ary;
    for (int i=0; i < rb_array_len(val); i++) {
      VALUE elt = rb_ary_entry(val, i);
      if (TYPE(elt) == T_STRING) {
        ary.push_back(StringValueCStr(elt));
      } else {
        VALUE insp = rb_inspect(val);
        rb_raise(rb_eProviderError,
          "only array[string], but not %s can be converted to a libral::value",
                 StringValueCStr(insp));
      }
    }
    return libral::value(ary);
  }
  VALUE insp = rb_inspect(val);
  rb_raise(rb_eProviderError, "cannot convert %s to a libral::value",
           StringValueCStr(insp));
  // Won't get here
  return libral::value(boost::none);
}

/*
 * Turn a lib::resource into an instance of Resource in ruby
 */
VALUE
resource_to_ruby(const lib::resource& res) {
  VALUE rb_name = rb_str_new_cstr(res.name().c_str());
  VALUE rb_attrs = rb_hash_new();

  for (const auto& attr : res.attrs()) {
    VALUE key = rb_str_new_cstr(attr.first.c_str());
    VALUE val = value_to_ruby(attr.second);
    rb_hash_aset(rb_attrs, key, val);
  }

  VALUE args[2] = { rb_name, rb_attrs };
  return rb_class_new_instance(2, args, rb_cResource);
}

/*
 * Retrieves the pointer to lib::type* stored in Provider objects
 */
static lib::type*
get_type(VALUE rb_prov) {
  lib::type *type;
  TypedData_Get_Struct(rb_prov, lib::type, &provider_data_type, type);
  return type;
}

/*
 *  Provider#get
 *  prov_get(args) -> Array[Resource]
 *
 * +args+ can be strings or arrays of strings. It is assumed that each such
 * string is the name of a resource to get.
 * @return [Array[Resource]] a list of resources
 */
VALUE
libral_prov_get(int argc, VALUE *argv, VALUE rb_prov) {
  std::vector<std::string> names;

  for (int i=0; i < argc; i++) {
    if (TYPE(argv[i]) == T_ARRAY) {
      for (int e=0; e < rb_array_len(argv[i]); e++) {
        VALUE elt = rb_ary_entry(argv[i], e);
        names.push_back(StringValueCStr(elt));
      }
    } else if (TYPE(argv[i]) == T_STRING) {
      names.push_back(StringValueCStr(argv[i]));
    } else {
          rb_raise(rb_eTypeError,
                   "arguments to get must be strings or arrays of strings");
    }
  }

  lib::type *type = get_type(rb_prov);

  auto r = type->get(names);
  if (!r) {
    rb_raise(rb_eProviderError, "get failed: %s", r.err().detail.c_str());
  }

  VALUE ary = rb_ary_new_capa(r.ok().size());
  for (const auto& res : r.ok()) {
    rb_ary_push(ary, resource_to_ruby(res));
  }
  return ary;
}

/**
 * call-seq:
 *   prov_set(Array[Resource]) -> Array[Update]
 */
VALUE
libral_prov_set(VALUE rb_prov, VALUE rb_resource_ary) {
  lib::type *type = get_type(rb_prov);

  if (TYPE(rb_resource_ary) != T_ARRAY) {
    rb_raise(rb_eTypeError,
             "the resources argument must be an array");
  }

  std::vector<lib::resource> rsrcs;
  for (int i = 0; i < rb_array_len(rb_resource_ary); i++) {
    VALUE rb_res = rb_ary_entry(rb_resource_ary, i);
    VALUE name = rb_iv_get(rb_res, "@name");
    VALUE should = rb_iv_get(rb_res, "@attrs");

    auto res = type->prov().create(StringValueCStr(name));
    /* should.keys.each { |key| res[key] = should[key] } */
    // mRuby has rb_hash_keys, but CRuby does not so we simulate it. Could
    // also do this with rb_hash_foreach; this is the lazy variant
    VALUE keys = rb_funcall(should, rb_intern("keys"), 0);
    for (int e = 0; e < rb_array_len(keys); e++) {
      VALUE key = rb_ary_entry(keys, e);
      VALUE val = rb_hash_aref(should, key);
      res[StringValueCStr(key)] = value_from_ruby(val);
    }
    rsrcs.push_back(res);
  }

  auto r = type->set(rsrcs);
  if (!r) {
    rb_raise(rb_eProviderError, "%s", r.err().detail.c_str());
  }

  /* Construct the result array */
  VALUE ary = rb_ary_new();
  for (const auto& pair : r.ok()) {
    auto& upd = pair.first;

    VALUE rb_changes = rb_ary_new();
    for(const auto& chg : pair.second) {
      VALUE attr = rb_str_new_cstr(chg.attr.c_str());
      VALUE is = value_to_ruby(chg.is);
      VALUE was = value_to_ruby(chg.was);

      VALUE change_args[3] = { attr, is, was };
      VALUE rb_change = rb_class_new_instance(3, change_args, rb_cChange);
      rb_ary_push(rb_changes, rb_change);
    }

    VALUE rb_is = resource_to_ruby(upd.is);
    VALUE rb_should = resource_to_ruby(upd.should);
    VALUE update_args[3] = { rb_is, rb_should, rb_changes };
    VALUE rb_update = rb_class_new_instance(3, update_args, rb_cUpdate);
    rb_ary_push(ary, rb_update);
  }
  return ary;
}

VALUE libral_prov_name(VALUE rb_prov) {
  lib::type *type = get_type(rb_prov);

  auto qname = type->qname();
  return rb_str_new(qname.c_str(), qname.length());
}

VALUE libral_ral_provider(VALUE rb_ral, VALUE rb_name) {
  std::shared_ptr<lib::ral> *ral;

  TypedData_Get_Struct(rb_ral, std::shared_ptr<lib::ral>, &ral_data_type, ral);

  const char *name = StringValueCStr(rb_name);
  auto type = (*ral)->find_type(name);
  if (! type) {
    rb_raise(rb_eProviderError, "Failed to find provider '%s'", name);
  }

  return TypedData_Wrap_Struct(rb_cProvider, &provider_data_type,
                               type->release());
}

/**
 *
 *   module Libral
 *     def self.open; end
 *   end
 */
VALUE libral_open(VALUE libral) {
  // create returns a std::shared_ptr allocated on the stack
  auto ral_stack = lib::ral::create({ });
  // make a std::shared_ptr on the heap and remember that one
  auto ral = new std::shared_ptr<lib::ral>(ral_stack);
  return TypedData_Wrap_Struct(rb_cRal, &ral_data_type, ral);
}

void Init_libral(void) {
  rb_mLibral = rb_define_module("Libral");

  rb_define_singleton_method(rb_mLibral, "open",
                             reinterpret_cast<VALUE(*)(...)>(libral_open), 0);

  // classes Libral::Resource, Libral::Change, Libral::Update
  // the actual code for the classes is in Ruby
  rb_cResource = rb_define_class_under(rb_mLibral, "Resource", rb_cObject);
  rb_cChange   = rb_define_class_under(rb_mLibral, "Change", rb_cObject);
  rb_cUpdate   = rb_define_class_under(rb_mLibral, "Update", rb_cObject);

  //   class Error < RuntimeError; end
  rb_eError = rb_define_class_under(rb_mLibral, "Error", rb_eRuntimeError);
  // class ProviderError < Error; end
  rb_eProviderError =
    rb_define_class_under(rb_mLibral, "ProviderError", rb_eError);

  rb_cRal = rb_define_class_under(rb_mLibral, "Ral", rb_cObject);
  rb_define_method(rb_cRal, "provider",
                   reinterpret_cast<VALUE(*)(...)>(libral_ral_provider), 1);

  rb_cProvider = rb_define_class_under(rb_mLibral, "Provider", rb_cObject);
  rb_define_method(rb_cProvider, "name",
                   reinterpret_cast<VALUE(*)(...)>(libral_prov_name), 0);
  rb_define_method(rb_cProvider, "set",
                   reinterpret_cast<VALUE(*)(...)>(libral_prov_set), 1);
  rb_define_method(rb_cProvider, "get",
                   reinterpret_cast<VALUE(*)(...)>(libral_prov_get), -1);
}
