#pragma once

#include <cstring>
#include <string>

struct cstring_view {

  cstring_view(const char * str)
      : d(str)
      , s(strlen(d)) { }
  cstring_view(const std::string_view & str)
      : d(str.data())
      , s(str.size()) { }

  const char * c_str() const noexcept {
    return d;
  }

  const char * data() const noexcept {
    return d;
  }

  const char * begin() const noexcept {
    return d;
  }

  const char * end() const noexcept {
    return d + s;
  }

  size_t size() const noexcept {
    return s;
  }

private:
  const char * d;
  std::size_t  s;
};

inline bool iequals(cstring_view first, cstring_view second)
{
  return std::equal(first.begin(), first.end(), second.begin(), second.end(),
                    [](char a, char b) {
                      return tolower(a) == tolower(b);
                    });
}