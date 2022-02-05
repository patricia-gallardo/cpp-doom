//
// Created by jason on 4/3/20.
//

#ifndef CRISPY_DOOM_LUMP_HPP
#define CRISPY_DOOM_LUMP_HPP

#include "../src/w_wad.hpp"

template<typename DataType>
auto
  cache_lump_name(const char *name, const int tag)
{
  return static_cast<DataType>(W_CacheLumpName(name, tag));
}

template<typename DataType>
auto
  cache_lump_num(lumpindex_t index, const int tag)
{
  return static_cast<DataType>(W_CacheLumpNum(index, tag));
}

#endif // CRISPY_DOOM_LUMP_HPP
