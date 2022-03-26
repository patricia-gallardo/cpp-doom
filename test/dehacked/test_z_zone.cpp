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

// Some compilers don't define __SANITIZE_ADDRESS__
#if defined(__has_feature)
# if __has_feature(address_sanitizer)
#  if !defined(__SANITIZE_ADDRESS__)
#   define __SANITIZE_ADDRESS__ // NOLINT
#  endif // !defined(__SANITIZE_ADDRESS__)
# endif // __has_feature(address_sanitizer)
#endif // __has_feature

#ifndef __SANITIZE_ADDRESS__
struct memblock_t {
  int                 size; // including the header and possibly tiny fragments
  void **             user;
  int                 tag; // PU_FREE if this is free
  int                 id;  // should be ZONEID
  struct memblock_t * next;
  struct memblock_t * prev;
};

long what;

memblock_t fake{
  .size = 0,
  .user = nullptr,
  .tag = PU_FREE,
  .id = 0,
  .next = nullptr, // where
  .prev = nullptr
};

TEST_CASE("Z_ZOverwrite header", "[z_zone]") {

  void * guard = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(guard != nullptr);

  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);

  void * guard2 = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(guard2 != nullptr);

  what = 0x42424242;

  // Corrupt header
  auto * byte_ptr = reinterpret_cast<uint8_t *>(ptr);
  auto * header   = reinterpret_cast<memblock_t *>(byte_ptr - sizeof(memblock_t));
  REQUIRE(header->tag == PU_LEVEL);
  header->prev      = &fake;
  header->next      = reinterpret_cast<memblock_t *>(&what);
  // Corruption done

  // Verify state
  REQUIRE(header->prev->next == nullptr);

  Z_Free(ptr);

  // Check if successful
  REQUIRE(header->prev->next != nullptr);
  REQUIRE(*(reinterpret_cast<long*>(header->prev->next)) == 0x42424242);
}
#endif
