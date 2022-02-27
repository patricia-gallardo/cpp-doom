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
// Dehacked I/O code (does all reads from dehacked files)
//

#pragma once

#include <cstdio>

#include <fmt/printf.h>

#include "deh_defs.hpp"

enum deh_input_type_t
{
  DEH_INPUT_FILE,
  DEH_INPUT_LUMP
};

struct deh_context_t {
  deh_input_type_t type;
  char *           filename;

  // If the input comes from a memory buffer, pointer to the memory
  // buffer.
  unsigned char * input_buffer;
  size_t          input_buffer_len;
  unsigned int    input_buffer_pos;
  int             lumpnum;

  // If the input comes from a file, the file stream for reading
  // data.
  FILE * stream;

  // Current line number that we have reached:
  int linenum;

  // Used by DEH_ReadLine:
  bool   last_was_newline;
  char * readbuffer;
  int    readbuffer_size;

  // Error handling.
  bool had_error;

  // [crispy] pointer to start of current line
  long linestart;
};

deh_context_t * DEH_OpenFile(const char * filename);
deh_context_t * DEH_OpenLump(int lumpnum);
void            DEH_CloseFile(deh_context_t * context);
int             DEH_GetChar(deh_context_t * context);
char *          DEH_ReadLine(deh_context_t * context, bool extended);

template <typename... Args>
inline void DEH_Error(deh_context_t * context, const char * msg, Args &&... args) {
  fmt::fprintf(stderr, "%s:%i: ", context->filename, context->linenum);
  fmt::fprintf(stderr, msg, args...);
  fmt::fprintf(stderr, "\n");

  context->had_error = true;
}

template <typename... Args>
inline void DEH_Warning(const deh_context_t * context, const char * msg, Args &&... args) {
  fmt::fprintf(stderr, "%s:%i: warning: ", context->filename, context->linenum);
  fmt::fprintf(stderr, msg, args...);
  fmt::fprintf(stderr, "\n");
}

bool   DEH_HadError(deh_context_t * context);
char * DEH_FileName(deh_context_t * context); // [crispy] returns filename
