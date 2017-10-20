#include <libral/libral.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/args.hpp>
#include <boost/filesystem.hpp>
#include <leatherman/logging/logging.hpp>
#include <leatherman/util/environment.hpp>

#include <libral/emitter/puppet_emitter.hpp>
#include <libral/emitter/json_emitter.hpp>
#include <libral/emitter/quiet_emitter.hpp>

#include <stdint.h>

#include <iomanip>

#include <config.hpp>

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#ifdef HAS_SUGGEST_OVERRIDE
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop


// We distinguish between failure to find something (EXIT_FAILURE) and
// other errors (EXIT_ERROR)
const static int EXIT_ERROR = 2;

using namespace std;
using namespace leatherman::logging;
namespace lib = libral;
namespace po = boost::program_options;
using namespace leatherman::locale;
namespace fs = boost::filesystem;
namespace util = leatherman::util;

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
  const static std::string help2 =
R"txt(Exit status:
  0 on success
  1 when no resource was found
  2 when an error happened
)txt";
  boost::nowide::cout << help1 << desc << endl;
  boost::nowide::cout << help2 << endl;
}

static void print_attr_explanation(const std::string& name,
                                   const lib::attr::spec& attr,
                                   uint16_t maxlen) {
  cout << "  " << color::green << left << setw(maxlen)
       << name << color::reset
       << " : " << attr.desc() << endl;
  cout << "  " << left << setw(maxlen) << " "
       << " . kind = " << color::blue << attr.kind()
                       << color::reset << endl;
  cout << "  " << left << setw(maxlen) << " "
       << " . type = " << color::blue << attr.data_type()
                       << color::reset << endl;
}

static void print_explanation(lib::provider& prov) {
  auto& spec = prov.spec();
  if (!spec) {
    cerr << _("internal error: failed to get metadata for {1}", prov.qname())
         << endl;
    return;
  }

  uint16_t maxlen = 0;
  for (auto a = spec->attr_begin(); a != spec->attr_end(); ++a) {
    if (a->first.length() > maxlen) maxlen = a->first.length();
  }

  cout << color::magenta
       << _("Provider {1}", spec->qname()) << endl
       << "  " << _("source: {1}", prov.source())
       << color::reset << endl;
  cout << spec->desc() << endl;
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
      ("target,t", po::value<std::string>(),
       "run commands on target TARGET (ssh only)")
      ("help,h", "produce help message")
      ("include,I", po::value<std::vector<std::string>>(), "search directory '$arg/providers' for providers.")
      ("log-level,l", po::value<log_level>()->default_value(log_level::warning, "warn"), "Set logging level.\nSupported levels are: none, trace, debug, info, warn, error, and fatal.")
      ("json,j", "produce JSON output")
      ("quiet,q", "suppress all normal output")
      ("absent,a", "consider resources with ensure=absent as missing")
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
      return EXIT_ERROR;
    }

    // Get the logging level
    auto lvl = vm["log-level"].as<log_level>();
    set_level(lvl);

    if (vm.count("version")) {
      boost::nowide::cout << libral::version() << endl;
      return EXIT_SUCCESS;
    }

    bool explain = vm.count("explain");
    bool err_on_absent = vm.count("absent");

    if (explain && vm.count("json")) {
      boost::nowide::cerr << "error: " << "you can not specify --json and --explain at the same time" << endl;
      boost::nowide::cerr << "error: " << "running 'ralsh --json' will contain explanations for all providers" << endl;
    }
    // Figure out our include path
    std::vector<std::string> data_dirs;
    if (vm.count("include")) {
      data_dirs = vm["include"].as<std::vector<std::string>>();
    }

    // Do the actual work
    auto ral = lib::ral::create(data_dirs);
    if (vm.count("target")) {
      auto target = vm["target"].as<std::string>();
      auto res = ral->connect(target);
      if (res.is_err()) {
        boost::nowide::cerr << color::red
                            << _("failed to connect to {1}", target)
                            << endl
                            << res.err().detail
                            << color::reset << endl;
        return EXIT_ERROR;
      }
    }
    std::unique_ptr<lib::emitter> emp;
    if (vm.count("quiet")) {
      emp = std::unique_ptr<lib::emitter>(new lib::quiet_emitter());
    } else if (vm.count("json")) {
      emp = std::unique_ptr<lib::emitter>(new lib::json_emitter());
    } else {
      emp = std::unique_ptr<lib::emitter>(new lib::puppet_emitter());
    }
    lib::emitter& em = *emp;

    if (vm.count("type")) {
      // We have a type name
      auto type_name = vm["type"].as<std::string>();
      auto opt_prov = ral->find_provider(type_name);
      if (opt_prov == boost::none) {
        boost::nowide::cout << color::red
                            << _("unknown provider: '{1}'", type_name)
                            << color::reset << endl;
        boost::nowide::cout << _("run 'ralsh' to see a list of all providers")
                            << color::reset << endl;
        return EXIT_ERROR;
      }

      auto& prov = **opt_prov;

      if (explain) {
        print_explanation(prov);
        return EXIT_SUCCESS;
      }

      if (vm.count("name")) {
        // We have a resource name
        auto name = vm["name"].as<std::string>();
        if (vm.count("attr-value")) {
          // We have attributes, modify resource
          auto av = vm["attr-value"].as<std::vector<std::string>>();
          lib::resource should = prov.create(name);

          for (const auto& arg : av) {
            auto found = arg.find("=");
            if (found != string::npos) {
              auto attr = arg.substr(0, found);
              auto value = prov.parse(attr, arg.substr(found+1));
              if (value) {
                should[attr] = value.ok();
              } else {
                boost::nowide::cerr << color::red <<
                  _("failed to read attribute {1}: {2}", attr,
                    value.err().detail) << color::reset << endl;
                boost::nowide::cerr << _("run 'ralsh -e {1}' to get a list of attributes and valid values", prov.qname()) << endl;
                return EXIT_ERROR;
              }
            }
          }

          auto res = prov.set({ should });
          em.print_set(prov, res);
          if (!res) {
            return EXIT_ERROR;
          }
        } else {
          // No attributes, dump the resource
          auto inst = prov.find(name);
          em.print_find(prov, inst);
          if (!inst) {
            return EXIT_ERROR;
          }
          if (! inst.ok() ||
              (err_on_absent && (*inst.ok())["ensure"] == lib::value("absent"))) {
            return EXIT_FAILURE;
          }
        }
      } else {
        // No resource name, dump all resources of the provider
        auto insts = prov.get();
        em.print_list(prov, insts);
        if (!insts) {
            return EXIT_ERROR;
        }
      }
    } else {
      if (explain) {
        boost::nowide::cout << color::red << _("please provide a type")
                            << color::reset << endl;
        boost::nowide::cout << _("run 'ralsh' to see a list of all types")
                            << color::reset << endl;
        return EXIT_ERROR;
      }
      // No type given, list known providers
      auto provs = ral->providers();
      em.print_providers(provs);
    }
  } catch (domain_error& ex) {
    colorize(boost::nowide::cerr, log_level::fatal);
    boost::nowide::cerr << _("unhandled exception: {1}\n", ex.what()) << endl;
    colorize(boost::nowide::cerr);
    return EXIT_ERROR;
  }

  return error_has_been_logged() ? EXIT_ERROR : EXIT_SUCCESS;
}
