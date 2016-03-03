#include <catch.hpp>
#include <libral/version.h>
#include <libral/libral.hpp>

SCENARIO("version() returns the version") {
    REQUIRE(libral::version() == LIBRAL_VERSION_WITH_COMMIT);
}
