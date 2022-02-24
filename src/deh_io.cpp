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

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "m_misc.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "lump.hpp"
#include "memory.hpp"
#include "deh_defs.hpp"
#include "deh_io.hpp"

static deh_context_t * DEH_NewContext() {
  deh_context_t * context = zmalloc<decltype(context)>(sizeof(*context), PU_STATIC, nullptr);

  // Initial read buffer size of 128 bytes

  context->readbuffer_size  = 128;
  context->readbuffer       = zmalloc<decltype(context->readbuffer)>(static_cast<size_t>(context->readbuffer_size), PU_STATIC, nullptr);
  context->linenum          = 0;
  context->last_was_newline = true;

  context->had_error = false;
  context->linestart = -1; // [crispy] initialize

  return context;
}

// Open a dehacked file for reading
// Returns nullptr if open failed

deh_context_t * DEH_OpenFile(const char * filename) {
  FILE * fstream = fopen(filename, "r");

  if (fstream == nullptr)
    return nullptr;

  deh_context_t * context = DEH_NewContext();

  context->type     = DEH_INPUT_FILE;
  context->stream   = fstream;
  context->filename = M_StringDuplicate(filename);

  return context;
}

// Open a WAD lump for reading.

deh_context_t * DEH_OpenLump(int lumpnum) {
  deh_context_t * context = DEH_NewContext();

  context->type             = DEH_INPUT_LUMP;
  context->lumpnum          = lumpnum;
  context->input_buffer     = cache_lump_num<unsigned char *>(lumpnum, PU_STATIC);
  context->input_buffer_len = W_LumpLength(lumpnum);
  context->input_buffer_pos = 0;

  context->filename = static_cast<char *>(malloc(9));
  M_StringCopy(context->filename, lumpinfo[lumpnum]->name, 9);

  return context;
}

// Close dehacked file

void DEH_CloseFile(deh_context_t * context) {
  if (context->type == DEH_INPUT_FILE) {
    fclose(context->stream);
  } else if (context->type == DEH_INPUT_LUMP) {
    W_ReleaseLumpNum(context->lumpnum);
  }

  free(context->filename);
  Z_Free(context->readbuffer);
  Z_Free(context);
}

int DEH_GetCharFile(deh_context_t * context) {
  if (feof(context->stream)) {
    // end of file

    return -1;
  }

  return fgetc(context->stream);
}

int DEH_GetCharLump(deh_context_t * context) {
  if (context->input_buffer_pos >= context->input_buffer_len) {
    return -1;
  }

  int result = context->input_buffer[context->input_buffer_pos];
  ++context->input_buffer_pos;

  return result;
}

// Reads a single character from a dehacked file

int DEH_GetChar(deh_context_t * context) {
  int result = 0;

  // Read characters, but ignore carriage returns
  // Essentially this is a DOS->Unix conversion

  do {
    switch (context->type) {
    case DEH_INPUT_FILE:
      result = DEH_GetCharFile(context);
      break;

    case DEH_INPUT_LUMP:
      result = DEH_GetCharLump(context);
      break;
    }
  } while (result == '\r');

  // Track the current line number

  if (context->last_was_newline) {
    ++context->linenum;
  }

  context->last_was_newline = result == '\n';

  return result;
}

// Increase the read buffer size

static void IncreaseReadBuffer(deh_context_t * context) {
  int    newbuffer_size = context->readbuffer_size * 2;
  char * newbuffer      = zmalloc<decltype(newbuffer)>(static_cast<size_t>(newbuffer_size), PU_STATIC, nullptr);

  std::memcpy(newbuffer, context->readbuffer, static_cast<size_t>(context->readbuffer_size));

  Z_Free(context->readbuffer);

  context->readbuffer      = newbuffer;
  context->readbuffer_size = newbuffer_size;
}

// [crispy] Save pointer to start of current line ...
void DEH_SaveLineStart(deh_context_t * context) {
  if (context->type == DEH_INPUT_FILE) {
    context->linestart = ftell(context->stream);
  } else if (context->type == DEH_INPUT_LUMP) {
    context->linestart = context->input_buffer_pos;
  }
}

// [crispy] ... and reset context to start of current line
// to retry with previous line parser in case of a parsing error
void DEH_RestoreLineStart(deh_context_t * context) {
  // [crispy] never point past the start
  if (context->linestart < 0)
    return;

  if (context->type == DEH_INPUT_FILE) {
    fseek(context->stream, context->linestart, SEEK_SET);
  } else if (context->type == DEH_INPUT_LUMP) {
    context->input_buffer_pos = static_cast<unsigned int>(context->linestart);
  }

  // [crispy] don't count this line twice
  --context->linenum;
}

// Read a whole line

char * DEH_ReadLine(deh_context_t * context, bool extended) {
  bool escaped = false;
  for (int pos = 0;;) {
    int c = DEH_GetChar(context);

    if (c < 0 && pos == 0) {
      // end of file

      return nullptr;
    }

    // cope with lines of any length: increase the buffer size

    if (pos >= context->readbuffer_size) {
      IncreaseReadBuffer(context);
    }

    // extended string support
    if (extended && c == '\\') {
      c = DEH_GetChar(context);

      // "\n" in the middle of a string indicates an internal linefeed
      if (c == 'n') {
        context->readbuffer[pos] = '\n';
        ++pos;
        continue;
      }

      // values to be assigned may be split onto multiple lines by ending
      // each line that is to be continued with a backslash
      if (c == '\n') {
        escaped = true;
        continue;
      }
    }

    // blanks before the backslash are included in the string
    // but indentation after the linefeed is not
    if (escaped && c >= 0 && isspace(c) && c != '\n') {
      continue;
    } else {
      escaped = false;
    }

    if (c == '\n' || c < 0) {
      // end of line: a full line has been read

      context->readbuffer[pos] = '\0';
      break;
    } else if (c != '\0') {
      // normal character; don't allow NUL characters to be
      // added.

      context->readbuffer[pos] = static_cast<char>(c);
      ++pos;
    }
  }

  return context->readbuffer;
}

bool DEH_HadError(deh_context_t * context) {
  return context->had_error;
}

// [crispy] return the filename of the DEHACKED file
// or nullptr if it is a DEHACKED lump loaded from a PWAD
char * DEH_FileName(deh_context_t * context) {
  if (context->type == DEH_INPUT_FILE) {
    return context->filename;
  }

  return nullptr;
}
