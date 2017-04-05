#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/compile.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/variable.h"

#include <memory>
#include <boost/lexical_cast.hpp>
#include <boost/nowide/iostream.hpp>

#include <leatherman/logging/logging.hpp>

#include <libral/libral.hpp>

#define E_PROVIDER_ERROR             (mrb_class_get(mrb, "ProviderError"))

struct _args {
  mrb_bool check_syntax : 1;
  mrb_bool verbose      : 1;
  int argc;
  char** argv;
};

namespace lib = libral;
namespace logging = leatherman::logging;

/* Global RAL instance, filled by main, used by prov_* */
std::shared_ptr<lib::ral> ral;

static void
usage(const char *name)
{
  static const char *const usage_msg[] = {
  "switches:",
  "-c           check syntax only",
  "-v           print version number, then run in verbose mode",
  "-l LEVEL     set the log level",
  "--verbose    run in verbose mode",
  "--version    print the version",
  "--copyright  print the copyright",
  NULL
  };
  const char *const *p = usage_msg;

  printf("Usage: %s [switches] tests.rb\n", name);
  while (*p)
    printf("  %s\n", *p++);
}

static int
parse_args(mrb_state *mrb, int argc, char **argv, struct _args *args)
{
  static const struct _args args_zero = { 0 };

  *args = args_zero;

  for (argc--,argv++; argc > 0; argc--,argv++) {
    char *item;
    if (argv[0][0] != '-') break;

    if (strlen(*argv) <= 1) {
      argc--; argv++;
      break;
    }

    item = argv[0] + 1;
    switch (*item++) {
    case 'c':
      args->check_syntax = TRUE;
      break;
    case 'v':
      if (!args->verbose) mrb_show_version(mrb);
      args->verbose = TRUE;
      break;
    case 'l':
      if (argc > 1) {
        argc--; argv++;
        logging::set_level(boost::lexical_cast<logging::log_level>(argv[0]));
        break;
      }
    case '-':
      if (strcmp((*argv) + 2, "version") == 0) {
        mrb_show_version(mrb);
        exit(EXIT_SUCCESS);
      }
      else if (strcmp((*argv) + 2, "verbose") == 0) {
        args->verbose = TRUE;
        break;
      }
      else if (strcmp((*argv) + 2, "copyright") == 0) {
        mrb_show_copyright(mrb);
        exit(EXIT_SUCCESS);
      }
    default:
      return EXIT_FAILURE;
    }
  }

  args->argv = (char **)mrb_realloc(mrb, args->argv, sizeof(char*) * (argc + 1));
  memcpy(args->argv, argv, (argc+1) * sizeof(char*));
  args->argc = argc;

  return EXIT_SUCCESS;
}

static void
cleanup(mrb_state *mrb, struct _args *args)
{
  mrb_free(mrb, args->argv);
  mrb_close(mrb);
}

static int
load_file(mrb_state *mrb, const char *fname, mrbc_context *ctx) {
  mrb_value v;
  FILE *fp;

  fp = fopen(fname, "r");
  if (fp == NULL) {
    printf("Cannot open %s\n", fname);
    return -1;
  }
  mrbc_filename(mrb, ctx, fname);
  ctx->capture_errors = 1;
  ctx->keep_lv = 1;
  ctx->no_optimize = 1;
  v = mrb_load_file_cxt(mrb, fp, ctx);
  if (mrb->exc) {
    if (!mrb_undef_p(v)) {
      mrb_print_error(mrb);
    }
    return -1;
  }

  return 0;
}

static void ral_type_free(mrb_state *mrb, void *type) {
  delete (lib::type*) type;
}

struct mrb_data_type ral_type = {
    "ral::type", ral_type_free
};

mrb_value
prov_init(mrb_state *mrb, mrb_value self) {
  char *name;

  mrb_get_args(mrb, "z", &name);

  auto type = ral->find_type(name);
  if (! type) {
    mrb_value n = mrb_str_new_cstr(mrb, name);
    mrb_raisef(mrb, E_PROVIDER_ERROR, "could not find provider '%S'", n);
  }

  mrb_data_init(self, type->release(), &ral_type);
  return self;
}

struct to_ruby_visitor : boost::static_visitor<mrb_value> {
  to_ruby_visitor(mrb_state *mrb) : _mrb(mrb) { }

  result_type operator()(const boost::none_t& n) const {
    return mrb_nil_value();
  }

  result_type operator()(const bool& b) const {
    return b ? mrb_true_value() : mrb_false_value();
  }

  result_type operator()(const std::string& s) const {
    return mrb_str_new_cstr(_mrb, s.c_str());
  }

  result_type operator()(const libral::array& ary) const {
    mrb_value rb_ary = mrb_ary_new(_mrb);
    for (auto s : ary) {
      mrb_ary_push(_mrb, rb_ary, mrb_str_new_cstr(_mrb, s.c_str()));
    }
    return rb_ary;
  }

  mrb_state *_mrb;
};

mrb_value
value_to_ruby(mrb_state* mrb, const lib::value &val) {
  auto visitor = to_ruby_visitor(mrb);
  return boost::apply_visitor(visitor, val);
  // FIXME: properly convert value -> ruby
}

lib::value
value_from_ruby(mrb_state *mrb, mrb_value val) {
  // FIXME: properly convert ruby -> value
  if (mrb_string_p(val)) {
    return lib::value(mrb_str_to_cstr(mrb, val));
  } else if (mrb_array_p(val)) {
    std::vector<std::string> ary;
    for (int i=0; i < mrb_ary_ptr(val)->len; i++) {
      mrb_value elt = mrb_ary_ref(mrb, val, i);
      if (mrb_string_p(elt)) {
        ary.push_back(mrb_str_to_cstr(mrb, elt));
      } else {
        mrb_raisef(mrb, E_PROVIDER_ERROR,
          "only array[string], but not %S can be converted to a libral::value",
          mrb_inspect(mrb, val));
      }
    }
    return libral::value(ary);
  }
  mrb_raisef(mrb, E_PROVIDER_ERROR, "cannot convert %S to a libral::value",
             mrb_inspect(mrb, val));
  // Won't get here
  return libral::value(boost::none);
}

/*
 * Turn a lib::resource into an instance of Resource in ruby
 */
mrb_value
resource_to_ruby(mrb_state *mrb, const lib::resource& res) {
  mrb_value key, val;

  RClass *c_res = mrb_class_get(mrb, "Resource");
  mrb_value rb_name = mrb_str_new_cstr(mrb, res.name().c_str());
  mrb_value rb_attrs = mrb_hash_new(mrb);

  for (const auto& attr : res.attrs()) {
    key = mrb_str_new_cstr(mrb, attr.first.c_str());
    val = value_to_ruby(mrb, attr.second);
    mrb_hash_set(mrb, rb_attrs, key, val);
  }

  mrb_value args[2] = { rb_name, rb_attrs };
  return mrb_obj_new(mrb, c_res, 2, args);
}

/**
 *  prov_get(names: Array[String]) -> Array[Hash]
 *
 * Each Hash represents a resource and maps the attr name to its value
 */
mrb_value
prov_get(mrb_state *mrb, mrb_value self) {
  std::vector<std::string> names;
  mrb_value ary;

  mrb_get_args(mrb, "A", &ary);
  for (int i=0; i < mrb_ary_ptr(ary)->len; i++) {
    mrb_value elt = mrb_ary_ref(mrb, ary, i);
    names.push_back(mrb_str_to_cstr(mrb, elt));
  }

  lib::type *type = (lib::type *) mrb_data_get_ptr(mrb, self, &ral_type);
  auto r = type->get(names);
  if (!r) {
    mrb_value e = mrb_str_new_cstr(mrb, r.err().detail.c_str());
    mrb_raisef(mrb, E_PROVIDER_ERROR, "get failed: %S", e);
  }

  ary = mrb_ary_new(mrb);
  for (const auto& res : r.ok()) {
    mrb_ary_push(mrb, ary, resource_to_ruby(mrb, res));
  }
  return ary;
}

/**
 * prov_set(Array[{ :name : String, :should : Hash }]) -> updates
 *
 * updates is an
 *   Array[{ :is => resource, :should => resource, :changes => changes  }]
 *   where resource is a hash representing the resource in the same
 *   form that get() uses
 *
 *   changes is an Array[{ :attr => String, :is => value, :was => value }]
 */
mrb_value
prov_set(mrb_state *mrb, mrb_value self) {
  mrb_value ary;
  mrb_sym name_sym    = mrb_intern_cstr(mrb, "@name");
  mrb_sym attrs_sym  = mrb_intern_cstr(mrb, "@attrs");

  RClass *c_change = mrb_class_get(mrb, "Change");
  RClass *c_update = mrb_class_get(mrb, "Update");

  mrb_get_args(mrb, "A", &ary);

  lib::type *type = (lib::type *) mrb_data_get_ptr(mrb, self, &ral_type);
  std::vector<lib::resource> rsrcs;
  for (int i = 0; i < mrb_ary_ptr(ary)->len; i++) {
    /* Destructure { :name => name, :attrs => attrs } */
    mrb_value rb_res = mrb_ary_ref(mrb, ary, i);
    mrb_value name = mrb_iv_get(mrb, rb_res, name_sym);
    mrb_value should = mrb_iv_get(mrb, rb_res, attrs_sym);

    auto res = type->prov().create(mrb_str_to_cstr(mrb, name));
    /* should.keys.each { |key| res[key] = should[key] } */
    mrb_value keys = mrb_hash_keys(mrb, should);
    for (int e = 0; e < mrb_ary_ptr(keys)->len; e++) {
      mrb_value key = mrb_ary_ref(mrb, keys, e);
      mrb_value val = mrb_hash_get(mrb, should, key);
      res[mrb_str_to_cstr(mrb, key)] = value_from_ruby(mrb, val);
    }
    rsrcs.push_back(res);
  }

  auto r = type->set(rsrcs);
  if (!r) {
    mrb_value e = mrb_str_new_cstr(mrb, r.err().detail.c_str());
    mrb_raisef(mrb, E_PROVIDER_ERROR, "%S", e);
  }

  /* Construct the result array */
  ary = mrb_ary_new(mrb);
  for (const auto& pair : r.ok()) {
    auto& upd = pair.first;

    mrb_value rb_changes = mrb_ary_new(mrb);
    for(const auto& chg : pair.second) {
      mrb_value rb_change;
      mrb_value attr = mrb_str_new_cstr(mrb, chg.attr.c_str());
      mrb_value is = value_to_ruby(mrb, chg.is);
      mrb_value was = value_to_ruby(mrb, chg.was);

      mrb_value change_args[3] = { attr, is, was };
      rb_change = mrb_obj_new(mrb, c_change, 3, change_args);
      mrb_ary_push(mrb, rb_changes, rb_change);
    }

    mrb_value rb_is = resource_to_ruby(mrb, upd.is);
    mrb_value rb_should = resource_to_ruby(mrb, upd.should);
    mrb_value update_args[3] = { rb_is, rb_should, rb_changes };
    mrb_value rb_update = mrb_obj_new(mrb, c_update, 3, update_args);
    mrb_ary_push(mrb, ary, rb_update);
  }
  return ary;
}

static void
libral_init(mrb_state *mrb) {
  struct RClass* c_prov = mrb_define_class(mrb, "Provider", mrb->object_class);

  mrb_define_class(mrb, "ProviderError", E_RUNTIME_ERROR);

  MRB_SET_INSTANCE_TT(c_prov, MRB_TT_DATA);

  mrb_define_method(mrb, c_prov, "initialize", prov_init, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, c_prov, "set", prov_set, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, c_prov, "get", prov_get, MRB_ARGS_REQ(1));
}

static void
run_mtest(mrb_state *mrb) {
  /* The equivalent of MTest::Unit.new.run */
  struct RClass *c_mtest = mrb_module_get(mrb, "MTest");
  struct RClass *c_unit = mrb_class_get_under(mrb, c_mtest, "Unit");
  mrb_value rb_unit = mrb_obj_new(mrb, c_unit, 0, NULL);
  mrb_funcall(mrb, rb_unit, "run", 0);
}

int
main(int argc, char **argv)
{
  mrb_state *mrb = mrb_open();
  int ret = -1;
  int i;
  struct _args args;
  mrb_value ARGV;
  mrbc_context *c, *c2;
  mrb_sym zero_sym;

  logging::setup_logging(boost::nowide::cerr);

  if (mrb == NULL) {
    fputs("Invalid mrb_state, exiting mruby\n", stderr);
    return EXIT_FAILURE;
  }

  ret = parse_args(mrb, argc, argv, &args);
  if (ret == EXIT_FAILURE || args.argc != 1) {
    cleanup(mrb, &args);
    usage(argv[0]);
    return ret;
  }

  ARGV = mrb_ary_new_capa(mrb, args.argc);
  for (i = 0; i < args.argc; i++) {
    char* utf8 = mrb_utf8_from_locale(args.argv[i], -1);
    if (utf8) {
      mrb_ary_push(mrb, ARGV, mrb_str_new_cstr(mrb, utf8));
      mrb_utf8_free(utf8);
    }
  }
  mrb_define_global_const(mrb, "ARGV", ARGV);
  mrb_define_global_const(mrb, "VERBOSE",
                          args.verbose ? mrb_true_value() : mrb_false_value());
  c = mrbc_context_new(mrb);
  if (args.check_syntax)
    c->no_exec = TRUE;

  c2 = mrbc_context_new(mrb);
  if (args.check_syntax)
    c2->no_exec = TRUE;

  /* Create RAL */
  // FIXME: parse -I arguments
  ral = lib::ral::create({ });
  libral_init(mrb);

  /* Set $0 */
  zero_sym = mrb_intern_lit(mrb, "$0");
  mrb_gv_set(mrb, zero_sym, mrb_str_new_cstr(mrb, "ral-check"));

  /* Load program */
  auto rb_library = ral->find_in_data_dirs("mrblib/ral_check.rb");
  if (! rb_library) {
    fputs("Could not find test harness mrblib/ral-check.rb in data dirs\n",
          stderr);
    ret = -1;
  } else {
    ret = load_file(mrb, rb_library->c_str(), c2);
    if (ret == 0) {
      ret = load_file(mrb, args.argv[0], c);
    }
  }

  /* Run the tests */
  if (ret == 0) {
    run_mtest(mrb);
  }

  mrbc_context_free(mrb, c);
  cleanup(mrb, &args);

  return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
