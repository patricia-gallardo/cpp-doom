#include <catch2/catch.hpp>

#include "net_sdl.hpp"

TEST_CASE("InitServer", "[net_sdl]") {
  REQUIRE(net_sdl_module.InitServer() == true);
}

TEST_CASE("InitClient", "[net_sdl]") {
  REQUIRE(net_sdl_module.InitClient() == true);
}

TEST_CASE("ResolveAddress", "[net_sdl]") {
  net_addr_t * addr = net_sdl_module.ResolveAddress("localhost:1234");
  //static char buf[128];
  std::string buf;
  buf.reserve(128);
  net_sdl_module.AddrToString(addr, buf.data(), sizeof(buf.data()) - 1);
  REQUIRE( buf == "" );
}
