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

TEST_CASE("Replace one", "[strings]") {
  std::string haystack    = "Now I know I’ve got a heart, ’cause it’s breaking.";
  std::string needle      = "heart";
  std::string replacement = "build";
  char *      actual      = M_StringReplace(haystack.c_str(), needle.c_str(), replacement.c_str());
  std::string expected    = "Now I know I’ve got a build, ’cause it’s breaking.";
  REQUIRE(expected == actual);
  free(actual);
}

TEST_CASE("Replace two", "[strings]") {
  std::string haystack    = "Put ‘em up! Put ‘em up!";
  std::string needle      = "up";
  std::string replacement = "down";
  char *      actual      = M_StringReplace(haystack.c_str(), needle.c_str(), replacement.c_str());
  std::string expected    = "Put ‘em down! Put ‘em down!";
  REQUIRE(expected == actual);
  free(actual);
}

TEST_CASE("Replace start", "[strings]") {
  std::string haystack    = "Here Scarecrow, wanna play ball?";
  std::string needle      = "Here";
  std::string replacement = "Hey";
  char *      actual      = M_StringReplace(haystack.c_str(), needle.c_str(), replacement.c_str());
  std::string expected    = "Hey Scarecrow, wanna play ball?";
  REQUIRE(expected == actual);
  free(actual);
}

TEST_CASE("Replace end", "[strings]") {
  std::string haystack    = "Don’t be silly, Toto. Scarecrows don’t talk.";
  std::string needle      = "talk.";
  std::string replacement = "bite.";
  char *      actual      = M_StringReplace(haystack.c_str(), needle.c_str(), replacement.c_str());
  std::string expected    = "Don’t be silly, Toto. Scarecrows don’t bite.";
  REQUIRE(expected == actual);
  free(actual);
}