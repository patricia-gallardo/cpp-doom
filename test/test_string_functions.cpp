#include <catch2/catch.hpp>

#include "m_misc.hpp"

TEST_CASE("Two words", "[strings]") {
  char *      actual   = M_StringJoin("Hello", " World");
  std::string expected = "Hello World";
  REQUIRE(expected == actual);
  free(actual);
}

TEST_CASE("Three words", "[strings]") {
  char *      actual   = M_StringJoin("Hello, ", "Hi, ", "World");
  std::string expected = "Hello, Hi, World";
  REQUIRE(expected == actual);
  free(actual);
}

TEST_CASE("Path seperator", "[strings]") {
  char *      actual   = M_StringJoin("First", DIR_SEPARATOR_S, "Second");
  std::string expected = "First" DIR_SEPARATOR_S "Second";
  REQUIRE(expected == actual);
  free(actual);
}
