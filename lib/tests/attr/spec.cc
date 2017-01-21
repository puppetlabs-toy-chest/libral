#include <catch.hpp>
#include <libral/attr/spec.hpp>

namespace libral { namespace attr {
  SCENARIO("attr::spec::create") {
    SECTION("creates string type") {
      auto spec = spec::create("test", "Test desc", "string", "r");
      REQUIRE(spec);
      REQUIRE(spec->name() == "test");
      REQUIRE(spec->desc() == "Test desc");
      REQUIRE(spec->get_data_type().type() == typeid(string_type));
      REQUIRE(spec->get_kind().get_tag() == kind::tag::r);
    }

    SECTION("creates boolean type") {
      auto spec = spec::create("test", "Test desc", "boolean", "w");
      REQUIRE(spec);
      REQUIRE(spec->name() == "test");
      REQUIRE(spec->desc() == "Test desc");
      REQUIRE(spec->get_data_type().type() == typeid(boolean_type));
      REQUIRE(spec->get_kind().get_tag() == kind::tag::w);
    }

    SECTION("creates a array[string] type") {
      // Varying amounts of space
      std::vector<std::string> array_texts =
        { "array[string]", " array[string]", "\tarray\t[ string  ]  " };
      for (auto array_text : array_texts) {
        SECTION("from '" + array_text + "'") {
          auto spec = spec::create("test", "Test desc", array_text, "rw");
          REQUIRE(spec);
          REQUIRE(spec->name() == "test");
          REQUIRE(spec->desc() == "Test desc");
          REQUIRE(spec->get_data_type().type() == typeid(array_type));
          REQUIRE(spec->get_kind().get_tag() == kind::tag::rw);
        }
      }
    }

    SECTION("creates an enum type") {
      // No options
      auto spec = spec::create("test", "Test desc", "enum", "r");
      REQUIRE(!spec);
      // Missing brakcets
      spec = spec::create("test", "Test desc", "enum yes,no]", "r");
      REQUIRE(!spec);
      spec = spec::create("test", "Test desc", "enum[yes,no", "r");
      REQUIRE(!spec);

      // Empty options
      spec = spec::create("test", "Test desc", "enum[ ]", "r");
      REQUIRE(!spec);
      spec = spec::create("test", "Test desc", "enum[, ]", "r");
      REQUIRE(!spec);
      spec = spec::create("test", "Test desc", "enum[yes, ]", "r");
      REQUIRE(!spec);

      // Valid enums
      std::vector<std::string> enum_texts =
        { "enum[yes,no]", "enum[yes, no]", "  enum\t [yes,no]",
          "enum[  yes,   no\t]  \t" };
      for (auto enum_text : enum_texts) {
        SECTION("from '" + enum_text + "'") {
          auto spec = spec::create("test", "Test desc", enum_text, "r");
          REQUIRE(spec);
          REQUIRE(spec->name() == "test");
          REQUIRE(spec->desc() == "Test desc");
          auto& data_type = spec->get_data_type();
          REQUIRE(data_type.type() == typeid(enum_type));
          auto& enum_t = boost::get<enum_type>(data_type);
          std::vector<std::string> opts = { "yes", "no" };
          REQUIRE(enum_t.options() == opts);
          REQUIRE(spec->get_kind().get_tag() == kind::tag::r);
        }
      }
    }

    SECTION("rejects gibberish type and kind") {
      auto spec = spec::create("test", "Test desc", "gibberish", "r");
      REQUIRE(!spec);

      spec = spec::create("test", "Test desc", "string", "rwx");
      REQUIRE(!spec);
    }
  }
} }
