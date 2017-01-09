#include <catch.hpp>
#include <libral/file.hpp>

#include <boost/optional/optional_io.hpp>

namespace libral {
  auto prov = file_provider("");

  SCENARIO("instances() returns an empty vector") {
    REQUIRE(prov.instances().empty());
  }
  SCENARIO("suitable() is always true") {
    auto s = prov.suitable();
    REQUIRE((s.is_ok() && *(s.ok())));
  }
  SCENARIO("find()") {
    SECTION("returns boost::none for nonexistent file") {
      auto rsc = prov.find("/tmp/not_there");
      REQUIRE(rsc == boost::none);
    }
  }
}
