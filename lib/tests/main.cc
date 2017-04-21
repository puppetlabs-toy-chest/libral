#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <boost/nowide/iostream.hpp>

#include <leatherman/logging/logging.hpp>

using namespace leatherman::logging;

int main(int argc, char **argv)
{
  setup_logging(boost::nowide::cerr);
  set_level(log_level::none);

  return Catch::Session().run(argc, argv);
}
