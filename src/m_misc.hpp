//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      Miscellaneous.
//

#pragma once

#include <cstdarg>
#include <cstdio>
#include <fmt/printf.h>

#include "cstring_view.hpp"
#include "doomtype.hpp"

bool         M_WriteFile(cstring_view name, const void * source, int length);
int          M_ReadFile(cstring_view name, uint8_t ** buffer);
void         M_MakeDirectory(cstring_view dir);
char *       M_TempFile(cstring_view s);
bool         M_FileExists(cstring_view file);
char *       M_FileCaseExists(cstring_view file);
long         M_FileLength(FILE * handle);
bool         M_StrToInt(cstring_view str, int * result);
char *       M_DirName(cstring_view path);
const char * M_BaseName(cstring_view path);
void         M_ExtractFileBase(cstring_view path, char * dest);
void         M_ForceUppercase(char * text);
void         M_ForceLowercase(char * text);
[[maybe_unused]] const char * M_StrCaseStr(cstring_view haystack, cstring_view needle);
char *       M_StringDuplicate(cstring_view orig);
bool         M_StringCopy(char * dest, cstring_view src, size_t dest_size);
bool         M_StringConcat(char * dest, cstring_view src, size_t dest_size);
char *       M_StringReplace(cstring_view haystack, cstring_view needle, cstring_view replacement);

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.

template <typename ...Args>
inline char * M_StringJoin(const char * s, Args &&... args) {
  std::string result = s;
  (result.append(args), ...);
  return strdup(result.c_str());
}

bool         M_StringStartsWith(cstring_view s, cstring_view prefix);
bool         M_StringEndsWith(cstring_view s, cstring_view suffix);
int          M_vsnprintf(char * buf, size_t buf_len, cstring_view s, va_list args);

// Safe, portable snprintf().
template <typename... Args>
inline size_t M_snprintf(char * buf, size_t buf_len, cstring_view fmt, Args &&... args) {
  if (buf_len < 1) {
    return 0;
  }

  auto copy_into_buf = [&buf, buf_len](const std::string & source) {
    std::size_t length = source.copy(buf, buf_len - 1);
    buf[length] = '\0';
    cstring_view view(buf);
    return view.size();
  };

  return copy_into_buf(fmt::sprintf(fmt.c_str(), args...));
}

char *       M_OEMToUTF8(cstring_view ansi);
