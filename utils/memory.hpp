#pragma once

#include "../src/z_zone.hpp"
#include <cstdlib>
#include <new>

// todo fix me
template <typename DataType>
auto create_struct() {
  auto * mem = malloc(sizeof(DataType));
  return new (mem) DataType {};
}

template <typename DataType>
auto create_struct(const std::size_t size) {
  auto * mem = malloc(sizeof(DataType) * size);
  return static_cast<DataType *>(new (mem) DataType[size]);
}

template <typename DataType>
auto zmalloc(size_t size, int tag, void * ptr) {
  return static_cast<DataType>(Z_Malloc(static_cast<int>(size), tag, ptr));
}
