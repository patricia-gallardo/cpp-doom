#pragma once

#include <cstring>
#include <string>

struct cstring_view {

  cstring_view(const char * str)
      : d(str)
      , s(str ? strlen(d) : 0) { }
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
  const char * d{ nullptr };
  std::size_t  s{ 0 };
};

inline bool iequals(cstring_view first, cstring_view second)
{
  return std::equal(first.begin(), first.end(), second.begin(), second.end(),
                    [](char a, char b) {
                      return tolower(a) == tolower(b);
                    });
}