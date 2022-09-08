#include <catch.hpp>

#include "m_mapmath.hpp"

// map math utils checks

TEST_CASE("approx_distance", "[mapmath]") {
  REQUIRE(map::approx_distance(0, 0) == 0);
  REQUIRE(map::approx_distance(1, 0) == 1);
  REQUIRE(map::approx_distance(0, 1) == 1);
  REQUIRE(map::approx_distance(1, 1) == 2);
  REQUIRE(map::approx_distance(10, 20) == 25);
  REQUIRE(map::approx_distance(20, 10) == 25);
  REQUIRE(map::approx_distance(-10, 10) == 15);
  REQUIRE(map::approx_distance(0, 5000) == 5000);
  REQUIRE(map::approx_distance(-5000, 5000) == 7500);
}

TEST_CASE("point_on_line_side", "[mapmath]") {
  REQUIRE(map::point_on_line_side(0, 0, 0, 0, 0, 0) == false);
  REQUIRE(map::point_on_line_side(0, 0, 0, 1, 1, 0) == true);
  REQUIRE(map::point_on_line_side(10, 0, 0, -1, 1, 0) == true);
  //cppdoom add more tests
}