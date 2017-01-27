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
  auto ptr = std::make_shared<file_provider>("");
  auto& prv = *ptr;

  SCENARIO("instances() returns an empty vector") {
    REQUIRE(prv.instances().empty());
  }

  SCENARIO("suitable() is always true") {
    auto s = prv.suitable();
    REQUIRE((s.is_ok() && s.ok()));
  }

  SCENARIO("find()") {
    SECTION("returns resource for nonexistent file") {
      auto res = prv.find("/tmp/not_there");

      REQUIRE(*res);
      auto& rsc = **res;
      REQUIRE(rsc["ensure"] == value("absent"));
    }

    SECTION("finds an existing file") {
      auto tmp = temp_file("");

      auto res = prv.find(tmp.get_file_name());

      REQUIRE(*res);
      auto& rsc = **res;
      REQUIRE(rsc["ensure"] == value("file"));
      auto full_name = fs::canonical(tmp.get_file_name());
      REQUIRE(rsc.name() == full_name);
    }
  }

  SCENARIO("update()") {
    SECTION("create a new file") {
      auto tmp = unique_fixture_path();
      try {
        auto res = prv.find(tmp.native());
        REQUIRE(*res);
        auto& rsc = **res;
        REQUIRE(rsc["ensure"] == value("absent"));

        auto attrs = attr_map();
        attrs["ensure"] = "present";
        attrs["mode"] = "0660";
        attrs["content"] = "stuff";
        auto upd = rsc.update(attrs);
        REQUIRE(upd);
        auto& chgs = upd.ok();
        REQUIRE(chgs.exists("ensure"));
        REQUIRE(chgs.exists("mode"));
        REQUIRE(rsc["ensure"] == value("file"));
        REQUIRE(rsc["mode"] == value("0660"));
        REQUIRE(rsc["content"] == value("stuff"));

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
      auto res = prv.find(tmp.get_file_name());
      REQUIRE(*res);
      auto& rsc = **res;

      // mutate file to link
      {
        auto attrs = attr_map();
        attrs["ensure"] = "link";
        attrs["target"] = "/tmp";
        auto upd = rsc.update(attrs);
        REQUIRE(upd);
        auto& chgs = upd.ok();
        REQUIRE(chgs.size() == 2);
        REQUIRE(chgs.exists("ensure"));
        REQUIRE(chgs.exists("target"));
        REQUIRE(rsc["ensure"] == value("link"));
        REQUIRE(rsc["target"] == value("/tmp"));
      }

      // change link target
      {
        auto attrs = attr_map();
        attrs.clear();
        attrs["target"] = "/var/tmp";
        auto upd = rsc.update(attrs);
        REQUIRE(upd);
        auto& chgs = upd.ok();
        REQUIRE(chgs.size() == 1);
        REQUIRE(chgs.exists("target"));
        REQUIRE(rsc["ensure"] == value("link"));
        REQUIRE(rsc["target"] == value("/var/tmp"));
      }

      // change link to directory
      {
        auto attrs = attr_map();
        attrs["ensure"] = "directory";
        auto upd = rsc.update(attrs);
        REQUIRE(upd);
        auto& chgs = upd.ok();
        REQUIRE(chgs.size() == 1);
        REQUIRE(chgs.exists("ensure"));
        REQUIRE(rsc["ensure"] == value("directory"));
      }
    }
  }
}
