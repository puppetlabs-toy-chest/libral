#include <libral/libral.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/args.hpp>
#include <boost/filesystem.hpp>
#include <leatherman/logging/logging.hpp>
#include <leatherman/util/environment.hpp>

#include <iomanip>

#include <config.hpp>

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

using namespace std;
using namespace leatherman::logging;
namespace lib = libral;
namespace po = boost::program_options;
using namespace leatherman::locale;
namespace fs = boost::filesystem;

namespace color {
  // Very poor man's output coloring. Call init() to fill these
  // with color escape sequences
  std::string cyan, green, yellow, red, magenta, blue, reset;

  void init() {
    if (isatty(1)) {
      cyan = "\33[0;36m";
      green = "\33[0;32m";
      yellow = "\33[0;33m";
      red = "\33[0;31m";
      magenta = "\33[0;35m";
      blue = "\33[0;34m";

      reset = "\33[0m";
    }
  }
}

void help(po::options_description& desc)
{
  const static std::string help1 =
R"txt(Usage: ralsh [OPTION]... [TYPE [NAME [ATTRIBUTE=VALUE] ... ] ]
Print resources managed by libral and modify them.

The positional arguments make ralsh behave in the following way:
  ralsh           : list all the types that libral knows about.
  ralsh TYPE      : list all instances of TYPE
  ralsh TYPE NAME : list just TYPE[NAME]
  ralsh TYPE NAME ATTRIBUTE=VALUE ... :
                    modify TYPE[NAME] by setting the provided attributes
                    to the corresponding values. Print the resulting resource
                    and a list of the changes that were made.

Options:
)txt";
  boost::nowide::cout << help1 << desc << endl;
}

static void print_resource_attr(const std::string& name,
                                const lib::value& v,
                                uint maxlen) {
  cout << "  " << color::green << left << setw(maxlen) << name << color::reset
       << " => '" << v << "'," << endl;
}

static void print_resource(lib::type& type, lib::resource& res) {
  cout << color::blue << type.name() << color::reset << " { '"
       << color::blue << res.name() << color::reset << "':" << endl;
  uint maxlen = 0;
  for (auto a = res.attr_begin(); a != res.attr_end(); ++a) {
    if (a->first.length() > maxlen) maxlen = a->first.length();
  }
  auto ens = res.lookup<std::string>("ensure");
  if (ens) {
    print_resource_attr("ensure", *ens, maxlen);
  }
  for (auto a = res.attr_begin(); a != res.attr_end(); ++a) {
    if (a->first == "ensure")
      continue;
    print_resource_attr(a->first, a->second, maxlen);
  }
  cout << "}" << endl;
}

static void print_update(lib::type& type, lib::resource& res,
                         const lib::result<lib::changes>& rslt) {
  if (rslt) {
    print_resource(type, res);
    cout << rslt.ok() << endl;
  } else {
    cout << color::red << _("failed: {1}", rslt.err().detail) << color::reset << endl;
  }
}

static void print_attr_explanation(const std::string& name,
                                   const lib::attr::spec& attr,
                                   uint maxlen) {
  cout << "  " << color::green << left << setw(maxlen)
       << name << color::reset
       << " : " << attr.desc() << endl;
  cout << "  " << left << setw(maxlen) << " "
       << " . kind = " << color::blue << attr.get_kind()
                       << color::reset << endl;
  cout << "  " << left << setw(maxlen) << " "
       << " . type = " << color::blue << attr.get_data_type()
                       << color::reset << endl;
}

static void print_explanation(lib::type& type) {
  auto& prov = type.prov();
  auto& spec = prov.spec();
  if (!spec) {
    cerr << _("internal error: failed to get metadata for {1}", type.name())
         << endl;
    return;
  }

  uint maxlen = 0;
  for (auto a = spec->attr_begin(); a != spec->attr_end(); ++a) {
    if (a->first.length() > maxlen) maxlen = a->first.length();
  }

  cout << _("Type {1} (from {2})", type.name(), prov.source()) << endl;
  if (auto attr = spec->attr("name")) {
    print_attr_explanation("name", *attr, maxlen);
  }
  if (auto attr = spec->attr("ensure")) {
    print_attr_explanation("ensure", *attr, maxlen);
  }
  for (auto attr = spec->attr_begin(); attr != spec->attr_end(); attr++) {
    if (attr->first == "name" || attr->first == "ensure") {
      continue;
    }
    print_attr_explanation(attr->first, attr->second, maxlen);
  }
}

static std::string absolute_path(const std::string& path) {
  return fs::absolute(fs::path(path)).native();
}

int main(int argc, char **argv) {
  try {
    // Fix args on Windows to be UTF-8
    boost::nowide::args arg_utf8(argc, argv);

    // Setup logging
    setup_logging(boost::nowide::cerr);

    color::init();

    po::options_description command_line_options("");
    command_line_options.add_options()
      ("explain,e", "print an explanation of TYPE, which must be provided")
      ("help,h", "produce help message")
      ("include,I", po::value<std::vector<std::string>>(), "search directory '$arg/providers' for providers.")
      ("log-level,l", po::value<log_level>()->default_value(log_level::warning, "warn"), "Set logging level.\nSupported levels are: none, trace, debug, info, warn, error, and fatal.")
      ("version", "print the version and exit");

    po::options_description all_options(command_line_options);

    /* Positional options */
    all_options.add_options()
      ("type", po::value<std::string>())
      ("name", po::value<std::string>())
      ("attr-value", po::value<std::vector<std::string>>());

    po::positional_options_description positional_options;
    positional_options.add("type", 1);
    positional_options.add("name", 1);
    positional_options.add("attr-value", -1);

    po::variables_map vm;

    try {
      po::store(po::command_line_parser(argc, argv).
                options(all_options).
                positional(positional_options).run(), vm);

      if (vm.count("help")) {
        help(command_line_options);
        return EXIT_SUCCESS;
      }

      po::notify(vm);
    } catch (exception& ex) {
      colorize(boost::nowide::cerr, log_level::error);
      boost::nowide::cerr << "error: " << ex.what() << "\n" << endl;
      colorize(boost::nowide::cerr);
      boost::nowide::cerr << "try 'ralsh -h' for more information." << endl;
      return EXIT_FAILURE;
    }

    // Get the logging level
    auto lvl = vm["log-level"].as<log_level>();
    set_level(lvl);

    if (vm.count("version")) {
      boost::nowide::cout << libral::version() << endl;
      return EXIT_SUCCESS;
    }

    bool explain = vm.count("explain");

    // Figure out our include path
    std::vector<std::string> data_dirs;
    std::string env_data_dir;
    if (leatherman::util::environment::get("RALSH_DATA_DIR", env_data_dir)) {
      data_dirs.push_back(absolute_path(env_data_dir));
    }
    if (vm.count("include")) {
      for (auto dir : vm["include"].as<std::vector<std::string>>()) {
        data_dirs.push_back(absolute_path(dir));
      }
    }
    data_dirs.push_back(absolute_path(LIBRAL_DATA_DIR));

    // Do the actual work
    auto ral = lib::ral::create(data_dirs);

    if (vm.count("type")) {
      // We have a type name
      auto type_name = vm["type"].as<std::string>();
      auto opt_type = ral->find_type(type_name);
      if (opt_type == boost::none) {
        boost::nowide::cout << color::red
                            << _("unknown type: '{1}'", type_name)
                            << color::reset << endl;
        boost::nowide::cout << _("run 'ralsh' to see a list of all types")
                            << color::reset << endl;
        return EXIT_FAILURE;
      }

      auto& type = *opt_type;

      if (explain) {
        print_explanation(*type);
        return EXIT_SUCCESS;
      }

      if (vm.count("name")) {
        // We have a resource name
        auto name = vm["name"].as<std::string>();
        if (vm.count("attr-value")) {
          // We have attributes, modify resource
          auto av = vm["attr-value"].as<std::vector<std::string>>();
          lib::attr_map attrs;

          attrs["name"] = name;
          for (auto arg = av.begin(); arg != av.end(); arg++) {
            auto found = arg->find("=");
            if (found != string::npos) {
              auto attr = arg->substr(0, found);
              auto value = type->parse(attr, arg->substr(found+1));
              if (value) {
                attrs[attr] = value.ok();
              } else {
                boost::nowide::cerr << color::red <<
                  _("failed to read attribute {1}: {2}", attr,
                    value.err().detail) << color::reset << endl;
                boost::nowide::cerr << _("run 'ralsh -e {1}' to get a list of attributes and valid values", type->name()) << endl;
                return EXIT_FAILURE;
              }
            }
          }
          auto res = type->update(name, attrs);
          if (!res) {
            boost::nowide::cerr << color::red <<
              _("failed to update {1}: {2}", name,
                res.err().detail) << color::reset << endl;
            return EXIT_FAILURE;
          } else {
            type->flush();
            print_update(*type, *(res->first), res->second);
          }
        } else {
          // No attributes, dump the resource
          auto inst = type->find(name);
          if (!inst) {
            boost::nowide::cerr << color::red <<
              _("failed to find {1}: {2}", name,
                inst.err().detail) << color::reset << endl;
            return EXIT_FAILURE;
          }
          if (*inst) {
            print_resource(*type, ***inst);
          }
        }
      } else {
        // No resource name, dump all resources of the type
        auto insts = type->instances();
        if (!insts) {
            boost::nowide::cerr << color::red <<
              _("failed to list {1}: {2}", type->name(),
                insts.err().detail) << color::reset << endl;
            return EXIT_FAILURE;
        }
        for (auto inst = insts->begin(); inst != insts->end(); ++inst) {
          print_resource(*type, **inst);
        }
      }
    } else {
      if (explain) {
        boost::nowide::cout << color::red << _("please provide a type")
                            << color::reset << endl;
        boost::nowide::cout << _("run 'ralsh' to see a list of all types")
                            << color::reset << endl;
        return EXIT_FAILURE;
      }
      // No type given, list known types
      auto types = ral->types();
      for (auto t = types.begin(); t != types.end(); ++t) {
        boost::nowide::cout << (*t)->name() << endl;
      }
    }
  } catch (domain_error& ex) {
    colorize(boost::nowide::cerr, log_level::fatal);
    boost::nowide::cerr << _("unhandled exception: {1}\n", ex.what()) << endl;
    colorize(boost::nowide::cerr);
    return EXIT_FAILURE;
  }

  return error_has_been_logged() ? EXIT_FAILURE : EXIT_SUCCESS;
}
