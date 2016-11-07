#include <catch.hpp>
#include <libral/version.h>
#include <libral/libral.hpp>


/*
  Missing tests:

  Simple providers
  - x.prov describe fails
  - describe returns invalid YAML
  - no meta key
  - no invoke
  - no type name

 */
SCENARIO("version() returns the version") {
    REQUIRE(libral::version() == LIBRAL_VERSION_WITH_COMMIT);
}
