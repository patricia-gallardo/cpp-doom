//
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
//
// Dehacked string replacements
//

#pragma once

#include <cstdio>

#include "doomtype.hpp"

// Used to do dehacked text substitutions throughout the program

const char * DEH_String(const char * s) PRINTF_ARG_ATTR(1);

// snprintf(), performing a replacement on the format string.
template <typename... Args>
void DEH_snprintf(char * buffer, size_t len, const char * fmt, Args &&... args) {
  if (len < 1) {
    return;
  }

  // Windows (and other OSes?) has a vsnprintf() that doesn't always
  // append a trailing \0. So we must do it, and write into a buffer
  // that is one byte shorter; otherwise this function is unsafe.
  int result = snprintf(buffer, len, fmt, args...);

  // If truncated, change the final char in the buffer to a \0.
  // A negative result indicates a truncated buffer on Windows.
  if (result < 0 || result >= static_cast<int>(len)) {
    buffer[len - 1] = '\0';
  }
}

void DEH_AddStringReplacement(const char * from_text, const char * to_text);
bool DEH_HasStringReplacement(const char * s);
