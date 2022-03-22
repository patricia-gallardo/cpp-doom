#include <catch2/catch.hpp>

#include "z_zone.hpp"

TEST_CASE("Z_Init", "[z_zone]") {
  Z_Init();
  REQUIRE(Z_ZoneSize() == 0x2000000);
}

TEST_CASE("Z_Malloc(PU_STATIC)", "[z_zone]") {
  void * ptr = Z_Malloc(10, PU_STATIC, nullptr);
  REQUIRE(ptr != nullptr);
}

TEST_CASE("Z_Malloc(PU_LEVEL)", "[z_zone]") {
  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);
}

TEST_CASE("Z_Malloc(PU_LEVSPEC)", "[z_zone]") {
  void * ptr = Z_Malloc(10, PU_LEVSPEC, nullptr);
  REQUIRE(ptr != nullptr);
}

TEST_CASE("Z_Malloc(PU_LEVEL) and Z_Free", "[z_zone]") {
  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);
  Z_Free(ptr);
}

TEST_CASE("Z_Malloc(PU_LEVEL) and Z_Free and Z_Malloc(PU_LEVEL)", "[z_zone]") {
  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);
  Z_Free(ptr);
  void * ptr2 = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr2 != nullptr);
  REQUIRE(ptr == ptr2);
}
