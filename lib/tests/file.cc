#include <catch.hpp>
#include <libral/file.hpp>
#include <iostream>
#include <memory>

#include <boost/optional/optional_io.hpp>
#include "boost/filesystem.hpp"

#include "fixtures.hpp"

namespace fs = boost::filesystem;

namespace libral {
  // We need to create a shared_ptr here because provider uses
  // shared_from_this and that only works if a shared_ptr already exists
  file_provider prv;

  resource::attributes config;

  SCENARIO("instances() returns an empty vector") {
    libral::context ctx;

    REQUIRE(! prv.get(ctx, {}, config));
  }

  SCENARIO("suitable() is always true") {
    auto s = prv.suitable();
    REQUIRE((s.is_ok() && s.ok()));
  }

  SCENARIO("find()") {
    libral::context ctx;

    SECTION("returns resource for nonexistent file") {
      auto res = prv.get(ctx, { "/tmp/not_there" }, config);

      REQUIRE(res.is_ok());
      REQUIRE(res.ok().size() == 1);
      auto& rsc = res.ok().front();
      REQUIRE(rsc["ensure"] == value("absent"));
    }

    SECTION("finds an existing file") {
      auto tmp = temp_file("");

      auto res = prv.get(ctx, { tmp.get_file_name() }, config);

      REQUIRE(res.is_ok());
      REQUIRE(res.ok().size() == 1);
      auto& rsc = res.ok().front();
      REQUIRE(rsc["ensure"] == value("file"));
      auto full_name = fs::canonical(tmp.get_file_name());
      REQUIRE(rsc.name() == full_name);
    }
  }

  SCENARIO("update()") {
    libral::context ctx;

    SECTION("create a new file") {
      auto tmp = unique_fixture_path();
      try {
        auto res = prv.get(ctx, { tmp.native() }, config);
        REQUIRE(res.is_ok());
        REQUIRE(res.ok().size() == 1);
        auto& rsc = res.ok().front();
        REQUIRE(rsc["ensure"] == value("absent"));

        resource should = prv.create(tmp.native());
        should["ensure"] = "present";
        should["mode"] = "0660";
        should["content"] = "stuff";

        {
          updates upds = { { .is = rsc, .should = should } };
          auto res = prv.set(ctx, upds);
          REQUIRE(res.is_ok());
          auto& chgs = ctx.changes_for(rsc.name());
          REQUIRE(chgs.exists("ensure"));
          REQUIRE(chgs.exists("mode"));

          auto changed = prv.create(upds.front(), chgs);
          REQUIRE(changed["ensure"] == value("file"));
          REQUIRE(changed["mode"] == value("0660"));
          REQUIRE(changed["content"] == value("stuff"));
        }
        fs::remove(tmp);
      } catch(...) {
          fs::remove(tmp);
          throw;
      }
    }

    SECTION("mutates file type") {
      // Here are all the possible file type changes; we should totally
      // test them:
      //     AA AF FF FD DF FA AD DD DL LD DA AL LF FL LL LA
      // Right now, we have tests for AF FL LD
      auto tmp = temp_file("original");
      auto res = prv.get(ctx, { tmp.get_file_name() }, config);
      REQUIRE(res.is_ok());
      REQUIRE(res.ok().size() == 1);
      auto& rsc = res.ok().front();

      SECTION("file -> link") {
        auto should = prv.create(rsc.name());
        should["ensure"] = "link";
        should["target"] = "/tmp";

        updates upds = { { .is = rsc, .should = should } };
        auto res = prv.set(ctx, upds);
        if (!res) {
          std::cout << res.err().detail << std::endl;
        }
        REQUIRE(res.is_ok());
        auto& chgs = ctx.changes_for(rsc.name());
        REQUIRE(chgs.size() == 2);
        REQUIRE(chgs.exists("ensure"));
        REQUIRE(chgs.exists("target"));

        auto changed = prv.create(upds.front(), chgs);
        REQUIRE(changed["ensure"] == value("link"));
        REQUIRE(changed["target"] == value("/tmp"));
      }

      SECTION("change link target") {
        auto should = prv.create(rsc.name());
        should["ensure"] = "link";
        should["target"] = "/var/tmp";

        updates upds = { { .is = rsc, .should = should } };
        auto res = prv.set(ctx, upds);
        REQUIRE(res.is_ok());
        auto& chgs = ctx.changes_for(rsc.name());
        REQUIRE(chgs.size() == 2);
        REQUIRE(chgs.exists("target"));

        auto changed = prv.create(upds.front(), chgs);
        REQUIRE(changed["ensure"] == value("link"));
        REQUIRE(changed["target"] == value("/var/tmp"));
      }

      SECTION("link -> directory") {
        auto should = prv.create(rsc.name());
        should["ensure"] = "directory";

        updates upds = { { .is = rsc, .should = should } };
        auto res = prv.set(ctx, upds);
        REQUIRE(res.is_ok());
        auto& chgs = ctx.changes_for(rsc.name());
        REQUIRE(chgs.size() == 1);
        REQUIRE(chgs.exists("ensure"));

        auto changed = prv.create(upds.front(), chgs);
        REQUIRE(changed["ensure"] == value("directory"));
      }
    }
  }
}
