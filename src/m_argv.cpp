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
//

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include "cstring_view.hpp"
#include "i_system.hpp"
#include "m_argv.hpp" // haleyjd 20110212: warning fix
#include "m_misc.hpp"

#ifdef WIN32
#include "d_iwad.hpp"
#include <SDL_stdinc.h>
#endif

int     myargc;
char ** myargv;

//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
//

int M_CheckParmWithArgs(cstring_view check, int num_args) {
  for (int i = 1; i < myargc - num_args; i++) {
    if (iequals(check, myargv[i]))
      return i;
  }

  return 0;
}

//
// M_ParmExists
//
// Returns true if the given parameter exists in the program's command
// line arguments, false if not.
//

bool M_ParmExists(cstring_view check) {
  return M_CheckParm(check) != 0;
}

int M_CheckParm(cstring_view check) {
  return M_CheckParmWithArgs(check, 0);
}

constexpr auto MAXARGVS = 100;

static void LoadResponseFile(int argv_index, cstring_view filename) {
  // Read the response file into memory
  FILE * handle = fopen(filename.c_str(), "rb");

  if (handle == nullptr) {
    fmt::printf("\nNo such response file!");
    exit(1);
  }

  fmt::printf("Found response file %s!\n", filename.c_str());

  auto size = static_cast<size_t>(M_FileLength(handle));

  // Read in the entire file
  // Allocate one byte extra - this is in case there is an argument
  // at the end of the response file, in which case a '\0' will be
  // needed.

  char * file = static_cast<char *>(malloc(static_cast<size_t>(size + 1)));
  {
    size_t i = 0;
    while (i < size) {
      size_t k = fread(file + i, 1, static_cast<size_t>(size - i), handle);

      if (ferror(handle)) { I_Error("Failed to read full contents of '%s'", filename); }

      i += k;
    }

    fclose(handle);
  }

  // Create new arguments list array

  char ** newargv = static_cast<char **>(malloc(sizeof(char *) * MAXARGVS));
  int     newargc = 0;
  std::memset(newargv, 0, sizeof(char *) * MAXARGVS);

  // Copy all the arguments in the list up to the response file

  for (int i = 0; i < argv_index; ++i) {
    newargv[i] = myargv[i];
    ++newargc;
  }

  char * infile = file;
  size_t k      = 0;

  while (k < size) {
    // Skip past space characters to the next argument

    while (k < size && isspace(infile[k])) {
      ++k;
    }

    if (k >= size) {
      break;
    }

    // If the next argument is enclosed in quote marks, treat
    // the contents as a single argument.  This allows long filenames
    // to be specified.

    if (infile[k] == '\"') {
      // Skip the first character(")
      ++k;

      newargv[newargc++] = &infile[k];

      // Read all characters between quotes

      while (k < size && infile[k] != '\"' && infile[k] != '\n') {
        ++k;
      }

      if (k >= size || infile[k] == '\n') {
        I_Error("Quotes unclosed in response file '%s'",
                filename);
      }

      // Cut off the string at the closing quote

      infile[k] = '\0';
      ++k;
    } else {
      // Read in the next argument until a space is reached

      newargv[newargc++] = &infile[k];

      while (k < size && !isspace(infile[k])) {
        ++k;
      }

      // Cut off the end of the argument at the first space

      infile[k] = '\0';

      ++k;
    }
  }

  // Add arguments following the response file argument

  for (int i = argv_index + 1; i < myargc; ++i) {
    newargv[newargc] = myargv[i];
    ++newargc;
  }

  myargv = newargv;
  myargc = newargc;

#if 0
    // Disabled - Vanilla Doom does not do this.
    // Display arguments

   fmt::printf("%d command-line args:\n", myargc);

    for (k=1; k<myargc; k++)
    {
       fmt::printf("'%s'\n", myargv[k]);
    }
#endif
}

//
// Find a Response File
//

void M_FindResponseFile() {
  for (int i = 1; i < myargc; i++) {
    if (myargv[i][0] == '@') {
      LoadResponseFile(i, myargv[i] + 1);
    }
  }

  for (;;) {
    //!
    // @arg <filename>
    //
    // Load extra command line arguments from the given response file.
    // Arguments read from the file will be inserted into the command
    // line replacing this argument. A response file can also be loaded
    // using the abbreviated syntax '@filename.rsp'.
    //
    int i = M_CheckParmWithArgs("-response", 1);
    if (i <= 0) {
      break;
    }
    // Replace the -response argument so that the next time through
    // the loop we'll ignore it. Since some parameters stop reading when
    // an argument beginning with a '-' is encountered, we keep something
    // that starts with a '-'.
    myargv[i] = const_cast<char *>("-_");
    LoadResponseFile(i + 1, myargv[i + 1]);
  }
}

#if defined(_WIN32)
enum
{
  FILETYPE_UNKNOWN = 0x0,
  FILETYPE_IWAD    = 0x2,
  FILETYPE_PWAD    = 0x4,
  FILETYPE_DEH     = 0x8,
};

static int GuessFileType(cstring_view name) {
  int          ret = FILETYPE_UNKNOWN;
  const char * base;
  char *       lower;
  static bool  iwad_found = false;

  base  = M_BaseName(name);
  lower = M_StringDuplicate(base);
  M_ForceLowercase(lower);

  // only ever add one argument to the -iwad parameter

  if (iwad_found == false && D_IsIWADName(lower)) {
    ret        = FILETYPE_IWAD;
    iwad_found = true;
  } else if (M_StringEndsWith(lower, ".wad") || M_StringEndsWith(lower, ".lmp")) {
    ret = FILETYPE_PWAD;
  } else if (M_StringEndsWith(lower, ".deh") || M_StringEndsWith(lower, ".bex") || // [crispy] *.bex
             M_StringEndsWith(lower, ".hhe") || M_StringEndsWith(lower, ".seh")) {
    ret = FILETYPE_DEH;
  }

  free(lower);

  return ret;
}

struct argument_t {
  char * str;
  int    type, stable;
};

static int CompareByFileType(const void * a, const void * b) {
  const argument_t * arg_a = (const argument_t *)a;
  const argument_t * arg_b = (const argument_t *)b;

  const int ret = arg_a->type - arg_b->type;

  return ret ? ret : (arg_a->stable - arg_b->stable);
}

void M_AddLooseFiles() {
  int          i, types = 0;
  char **      newargv;
  argument_t * arguments;

  if (myargc < 2) {
    return;
  }

  // allocate space for up to three additional regular parameters

  arguments = static_cast<argument_t *>(malloc((myargc + 3) * sizeof(*arguments)));
  std::memset(arguments, 0, (myargc + 3) * sizeof(*arguments));

  // check the command line and make sure it does not already
  // contain any regular parameters or response files
  // but only fully-qualified LFS or UNC file paths

  for (i = 1; i < myargc; i++) {
    char * arg = myargv[i];
    int    type;

    if (strlen(arg) < 3 || arg[0] == '-' || arg[0] == '@' || ((!isalpha(arg[0]) || arg[1] != ':' || arg[2] != '\\') && (arg[0] != '\\' || arg[1] != '\\'))) {
      free(arguments);
      return;
    }

    type                = GuessFileType(arg);
    arguments[i].str    = arg;
    arguments[i].type   = type;
    arguments[i].stable = i;
    types |= type;
  }

  // add space for one additional regular parameter
  // for each discovered file type in the new argument  list
  // and sort parameters right before their corresponding file paths

  if (types & FILETYPE_IWAD) {
    arguments[myargc].str  = M_StringDuplicate("-iwad");
    arguments[myargc].type = FILETYPE_IWAD - 1;
    myargc++;
  }
  if (types & FILETYPE_PWAD) {
    arguments[myargc].str  = M_StringDuplicate("-merge");
    arguments[myargc].type = FILETYPE_PWAD - 1;
    myargc++;
  }
  if (types & FILETYPE_DEH) {
    arguments[myargc].str  = M_StringDuplicate("-deh");
    arguments[myargc].type = FILETYPE_DEH - 1;
    myargc++;
  }

  newargv = static_cast<char **>(malloc(myargc * sizeof(*newargv)));

  // sort the argument list by file type, except for the zeroth argument
  // which is the executable invocation itself

  SDL_qsort(arguments + 1, myargc - 1, sizeof(*arguments), CompareByFileType);

  newargv[0] = myargv[0];

  for (i = 1; i < myargc; i++) {
    newargv[i] = arguments[i].str;
  }

  free(arguments);

  myargv = newargv;
}
#endif

// Return the name of the executable used to start the program:

const char * M_GetExecutableName() {
  return M_BaseName(myargv[0]);
}
