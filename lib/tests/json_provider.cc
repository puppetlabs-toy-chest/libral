#include <catch.hpp>
#include <libral/ral.hpp>
#include <iostream>
#include <memory>

#include <boost/optional/optional_io.hpp>
#include "boost/filesystem.hpp"

#include "fixtures.hpp"

namespace libral {
  SCENARIO("json_provider harness") {
    auto aral = ral::create({ TEST_DATA_DIR });
    auto json_type = *aral->find_type("json");

    SECTION("behaves with invalid error messages") {
      auto all = json_type->get();
      REQUIRE(all.is_ok());

      for(const auto &r0 : all.ok()) {
        auto res = json_type->find(r0.name());
        REQUIRE(res.is_err());

        // Every error message needs to contain the provider name
        REQUIRE_THAT(res.err().detail, Catch::Contains("[json::json]"));
        // The resources tell us in "message" if there are specific things
        // we should look for in the error message
        auto msg = r0.lookup<std::string>("message");
        if (msg) {
          REQUIRE_THAT(res.err().detail, Catch::Contains(*msg));
        }
      }

      // Now tickle the same error via an update
      for(const auto &r0 : all.ok()) {
        // The resource names for set need to be prefixed with 'set_' so we
        // do not trip the provider's get which happens before set is
        // called
        auto should = json_type->prov().create("set_" + r0.name());
        should["ensure"] = "absent";
        auto res = json_type->set(should);
        REQUIRE(res.is_err());
        REQUIRE_THAT(res.err().detail, Catch::Contains("[json::json]"));
        auto msg = r0.lookup<std::string>("message");
        if (msg) {
          REQUIRE_THAT(res.err().detail, Catch::Contains(*msg));
        }
      }
    }

  }
}
