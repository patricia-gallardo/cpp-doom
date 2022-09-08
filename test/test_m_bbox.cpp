#include <catch.hpp>
#include <limits>

#include "m_bbox.hpp"

// bounding box checks

TEST_CASE("init", "[bounding_box]") {
  bounding_box_t box;
  REQUIRE(box.get(box_e::top) == 0);
  REQUIRE(box.get(box_e::bottom) == 0);
  REQUIRE(box.get(box_e::left) == 0);
  REQUIRE(box.get(box_e::right) == 0);
}

TEST_CASE("set", "[bounding_box]") {
  bounding_box_t box;
  box.set(box_e::top, 42);
  REQUIRE(box.get(box_e::top) == 42);
  box.set(box_e::bottom, 42);
  REQUIRE(box.get(box_e::bottom) == 42);
  box.set(box_e::left, 42);
  REQUIRE(box.get(box_e::left) == 42);
  box.set(box_e::right, 42);
  REQUIRE(box.get(box_e::right) == 42);
}

TEST_CASE("set_vertex", "[bounding_box]") {
  bounding_box_t box;
  REQUIRE(box.get(box_e::top) == 0);
  REQUIRE(box.get(box_e::bottom) == 0);
  REQUIRE(box.get(box_e::left) == 0);
  REQUIRE(box.get(box_e::right) == 0);

  box.set_vertex(1, 1, -1, -1);

  REQUIRE(box.get(box_e::top) == 1);
  REQUIRE(box.get(box_e::bottom) == -1);
  REQUIRE(box.get(box_e::left) == -1);
  REQUIRE(box.get(box_e::right) == 1);
}

TEST_CASE("clear", "[bounding_box]") {
  bounding_box_t box;
  box.set(box_e::top, 42);
  box.set(box_e::bottom, 42);
  box.set(box_e::left, 42);
  box.set(box_e::right, 42);
  box.clear();

  REQUIRE(box.get(box_e::top) == std::numeric_limits<int32_t>::min());
  REQUIRE(box.get(box_e::bottom) == std::numeric_limits<int32_t>::max());
  REQUIRE(box.get(box_e::left) == std::numeric_limits<int32_t>::max());
  REQUIRE(box.get(box_e::right) == std::numeric_limits<int32_t>::min());
}

TEST_CASE("add", "[bounding_box]") {
  bounding_box_t box;
  REQUIRE(box.get(box_e::top) == 0);
  REQUIRE(box.get(box_e::bottom) == 0);
  REQUIRE(box.get(box_e::left) == 0);
  REQUIRE(box.get(box_e::right) == 0);

  box.add(1, 1);

  REQUIRE(box.get(box_e::top) == 1);
  REQUIRE(box.get(box_e::bottom) == 0);
  REQUIRE(box.get(box_e::left) == 0);
  REQUIRE(box.get(box_e::right) == 1);

  box.add(-1, -1);

  REQUIRE(box.get(box_e::top) == 1);
  REQUIRE(box.get(box_e::bottom) == -1);
  REQUIRE(box.get(box_e::left) == -1);
  REQUIRE(box.get(box_e::right) == 1);
}

TEST_CASE("is_inside", "[bounding_box]") {
  {
    bounding_box_t box1;
    box1.set_vertex(1, 1, -1, -1);

    bounding_box_t box2;
    box2.set_vertex(2, 2, -2, -2);

    REQUIRE(box1.is_outside(box2) == false);
  }
  {
    bounding_box_t box1;
    box1.set_vertex(2, 2, 1, 1);

    bounding_box_t box2;
    box2.set_vertex(-1, -1, -2, -2);

    REQUIRE(box1.is_outside(box2) == true);
  }
  {
    bounding_box_t box1;
    box1.set_vertex(1, 1, 0, 0);

    bounding_box_t box2;
    box2.set_vertex(0, 0, -1, -1);

    REQUIRE(box1.is_outside(box2) == true);
  }
}
