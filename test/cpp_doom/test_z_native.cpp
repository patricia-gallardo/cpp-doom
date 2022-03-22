#include <catch2/catch.hpp>

#include "z_zone.hpp"

TEST_CASE("Z_Init", "[z_native]") {
  Z_Init();
}

TEST_CASE("Z_Malloc(PU_STATIC)", "[z_native]") {
  void * ptr = Z_Malloc(10, PU_STATIC, nullptr);
  REQUIRE(ptr != nullptr);
}

TEST_CASE("Z_Malloc(PU_LEVEL)", "[z_native]") {
  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);
}

TEST_CASE("Z_Malloc(PU_LEVSPEC)", "[z_native]") {
  void * ptr = Z_Malloc(10, PU_LEVSPEC, nullptr);
  REQUIRE(ptr != nullptr);
}

TEST_CASE("Z_Malloc(PU_LEVEL) and Z_Free", "[z_native]") {
  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);
  Z_Free(ptr);
}

TEST_CASE("Z_Malloc(PU_LEVEL) and Z_Free and Z_Malloc(PU_LEVEL)", "[z_native]") {
  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);
  Z_Free(ptr);
  void * ptr2 = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr2 != nullptr);
  REQUIRE(ptr == ptr2);
}

struct memblock_t {
  int          id; // = ZONEID
  int          tag;
  int          size;
  void **      user;
  memblock_t * prev;
  memblock_t * next;
};

long * where;
long   what;

TEST_CASE("Z_ZOverwrite header", "[z_native]") {

  void * guard = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(guard != nullptr);

  void * ptr = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(ptr != nullptr);

  void * guard2 = Z_Malloc(10, PU_LEVEL, nullptr);
  REQUIRE(guard2 != nullptr);

  // Corrupt header
  where = nullptr;
  what  = 0x42424242;

  auto * byte_ptr = reinterpret_cast<uint8_t *>(ptr);
  auto * header   = reinterpret_cast<memblock_t *>(byte_ptr - sizeof(memblock_t));
  REQUIRE(header->tag == PU_LEVEL);
  long * what_ptr   = &what;
  long ** where_ptr = &where;

  auto distance = reinterpret_cast<uint8_t*>(&(header->next)) - reinterpret_cast<uint8_t*>(header);
  if constexpr (sizeof(void *) == 8)
    REQUIRE(distance == 32);
  else
    REQUIRE(distance == 20);

  uint8_t * byte_where_ptr = reinterpret_cast<uint8_t*>(where_ptr);
  uint8_t * adjusted_byte_where_ptr = byte_where_ptr - distance;

  header->prev      = reinterpret_cast<memblock_t *>(adjusted_byte_where_ptr);
  header->next      = reinterpret_cast<memblock_t *>(what_ptr);
  // Corruption done

  // Verify state
  REQUIRE(where == nullptr);

  Z_Free(ptr);

  // Check if successful
  REQUIRE(where != nullptr);
  REQUIRE(*where == 0x42424242);
}
