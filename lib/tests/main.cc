#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <boost/nowide/iostream.hpp>

#include <leatherman/logging/logging.hpp>
#include <leatherman/util/environment.hpp>

#include "fixtures.hpp"

using namespace leatherman::logging;
namespace util = leatherman::util;

int main(int argc, char **argv)
{
  setup_logging(boost::nowide::cerr);
  set_level(log_level::none);

  // Modify PATH so that our mruby is on it
  std::string path;
  util::environment::get("PATH", path);
  util::environment::set("PATH", TEST_BIN_DIR ":" + path);

  return Catch::Session().run(argc, argv);
}
