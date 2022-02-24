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
// Main dehacked code
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include <fmt/printf.h>

#include "i_glob.hpp"
#include "i_system.hpp"
#include "d_iwad.hpp"
#include "m_argv.hpp"
#include "w_wad.hpp"
#include "deh_defs.hpp"
#include "deh_io.hpp"
#include "deh_main.hpp"

extern deh_section_t *deh_section_types[];
extern const char    *deh_signatures[];

static bool deh_initialized = false;

// If true, we can parse [STRINGS] sections in BEX format.

bool deh_allow_extended_strings = true; // [crispy] always allow

// If true, we can do long string replacements.

bool deh_allow_long_strings = true; // [crispy] always allow

// If true, we can do cheat replacements longer than the originals.

bool deh_allow_long_cheats = true; // [crispy] always allow

// If false, dehacked cheat replacements are ignored.

bool deh_apply_cheats = true;

void DEH_Checksum(sha1_digest_t digest) {
  sha1_context_t sha1_context;

  SHA1_Init(&sha1_context);

  for (unsigned int i = 0; deh_section_types[i] != nullptr; ++i) {
    if (deh_section_types[i]->sha1_hash != nullptr) {
      deh_section_types[i]->sha1_hash(&sha1_context);
    }
  }

  SHA1_Final(digest, &sha1_context);
}

// Called on startup to call the Init functions

static void InitializeSections() {
  for (unsigned int i = 0; deh_section_types[i] != nullptr; ++i) {
    if (deh_section_types[i]->init != nullptr) {
      deh_section_types[i]->init();
    }
  }
}

static void DEH_Init() {
  //!
  // @category mod
  //
  // Ignore cheats in dehacked files.
  //

  if (M_CheckParm("-nocheats") > 0) {
    deh_apply_cheats = false;
  }

  // Call init functions for all the section definitions.
  InitializeSections();

  deh_initialized = true;
}

// Given a section name, get the section structure which corresponds

static deh_section_t *GetSectionByName(char *name) {
  // we explicitely do not recognize [STRINGS] sections at all
  // if extended strings are not allowed

  if (!deh_allow_extended_strings && !strncasecmp("[STRINGS]", name, 9)) {
    return nullptr;
  }

  for (unsigned int i = 0; deh_section_types[i] != nullptr; ++i) {
    if (!strcasecmp(deh_section_types[i]->name, name)) {
      return deh_section_types[i];
    }
  }

  return nullptr;
}

// Is the string passed just whitespace?

static bool IsWhitespace(char *s) {
  for (; *s; ++s) {
    if (!isspace(*s))
      return false;
  }

  return true;
}

// Strip whitespace from the start and end of a string

static char *CleanString(char *s) {
  // Leading whitespace

  while (*s && isspace(*s))
    ++s;

  // Trailing whitespace

  char *strending = s + strlen(s) - 1;

  while (strlen(s) > 0 && isspace(*strending)) {
    *strending = '\0';
    --strending;
  }

  return s;
}

// This pattern is used a lot of times in different sections,
// an assignment is essentially just a statement of the form:
//
// Variable Name = Value
//
// The variable name can include spaces or any other characters.
// The string is split on the '=', essentially.
//
// Returns true if read correctly

bool DEH_ParseAssignment(char *line, char **variable_name, char **value) {
  // find the equals

  char *p = strchr(line, '=');

  if (p == nullptr) {
    return false;
  }

  // variable name at the start
  // turn the '=' into a \0 to terminate the string here

  *p             = '\0';
  *variable_name = CleanString(line);

  // value immediately follows the '='

  *value = CleanString(p + 1);

  return true;
}

extern void DEH_SaveLineStart(deh_context_t *context);
extern void DEH_RestoreLineStart(deh_context_t *context);

static bool CheckSignatures(deh_context_t *context) {
  // [crispy] save pointer to start of line (should be 0 here)
  DEH_SaveLineStart(context);

  // Read the first line

  char *line = DEH_ReadLine(context, false);

  if (line == nullptr) {
    return false;
  }

  // Check all signatures to see if one matches

  for (size_t i = 0; deh_signatures[i] != nullptr; ++i) {
    if (!strcmp(deh_signatures[i], line)) {
      return true;
    }
  }

  // [crispy] not a valid signature, try parsing this line again
  // and see if it starts with a section marker
  DEH_RestoreLineStart(context);

  return false;
}

// Parses a comment string in a dehacked file.

static void DEH_ParseComment(char *comment) {
  //
  // Welcome, to the super-secret Chocolate Doom-specific Dehacked
  // overrides function.
  //
  // Putting these magic comments into your Dehacked lumps will
  // allow you to go beyond the normal limits of Vanilla Dehacked.
  // Because of this, these comments are deliberately undocumented,
  // and if you're using them you should be aware that your mod
  // is not compatible with Vanilla Doom and you're probably a
  // very naughty person.
  //

  // Allow comments containing this special value to allow string
  // replacements longer than those permitted by DOS dehacked.
  // This allows us to use a dehacked patch for doing string
  // replacements for emulating Chex Quest.
  //
  // If you use this, your dehacked patch may not work in Vanilla
  // Doom.

  if (strstr(comment, "*allow-long-strings*") != nullptr) {
    deh_allow_long_strings = true;
  }

  // Allow magic comments to allow longer cheat replacements than
  // those permitted by DOS dehacked.  This is also for Chex
  // Quest.

  if (strstr(comment, "*allow-long-cheats*") != nullptr) {
    deh_allow_long_cheats = true;
  }

  // Allow magic comments to allow parsing [STRINGS] section
  // that are usually only found in BEX format files. This allows
  // for substitution of map and episode names when loading
  // Freedoom/FreeDM IWADs.

  if (strstr(comment, "*allow-extended-strings*") != nullptr) {
    deh_allow_extended_strings = true;
  }
}

// Parses a dehacked file by reading from the context

static void DEH_ParseContext(deh_context_t *context) {
  deh_section_t *current_section = nullptr;
  deh_section_t *prev_section    = nullptr; // [crispy] remember previous line parser
  char           section_name[20];
  void          *tag = nullptr;

  // Read the header and check it matches the signature

  if (!CheckSignatures(context)) {
    // [crispy] make non-fatal
    fmt::fprintf(stderr, "This is not a valid dehacked patch file!\n");
  }

  // Read the file

  while (!DEH_HadError(context)) {
    // Read the next line. We only allow the special extended parsing
    // for the BEX [STRINGS] section.
    bool extended = current_section != nullptr
                    && !strcasecmp(current_section->name, "[STRINGS]");
    // [crispy] save pointer to start of line, just in case
    DEH_SaveLineStart(context);
    char *line = DEH_ReadLine(context, extended);

    // end of file?

    if (line == nullptr) {
      return;
    }

    while (line[0] != '\0' && isspace(line[0]))
      ++line;

    if (line[0] == '#') {
      // comment

      DEH_ParseComment(line);
      continue;
    }

    if (IsWhitespace(line)) {
      if (current_section != nullptr) {
        // end of section

        if (current_section->end != nullptr) {
          current_section->end(context, tag);
        }

        // [crispy] if this was a BEX line parser, remember it in case
        // the next section does not start with a section marker
        if (current_section->name[0] == '[') {
          prev_section = current_section;
        } else {
          prev_section = nullptr;
        }

        // printf("end %s tag\n", current_section->name);
        current_section = nullptr;
      }
    } else {
      if (current_section != nullptr) {
        // parse this line

        current_section->line_parser(context, line, tag);
      } else {
        // possibly the start of a new section

        sscanf(line, "%19s", section_name);

        current_section = GetSectionByName(section_name);

        if (current_section != nullptr) {
          tag = current_section->start(context, line);
          // printf("started %s tag\n", section_name);
        } else if (prev_section != nullptr) {
          // [crispy] try this line again with the previous line parser
          DEH_RestoreLineStart(context);
          current_section = prev_section;
          prev_section    = nullptr;
        } else {
          // printf("unknown section name %s\n", section_name);
        }
      }
    }
  }
}

// Parses a dehacked file

int DEH_LoadFile(const char *filename) {
  if (!deh_initialized) {
    DEH_Init();
  }

  // Before parsing a new file, reset special override flags to false.
  // Magic comments should only apply to the file in which they were
  // defined, and shouldn't carry over to subsequent files as well.
  // [crispy] always allow everything
  /*
  deh_allow_long_strings = false;
  deh_allow_long_cheats = false;
  deh_allow_extended_strings = false;
*/

  fmt::printf(" loading %s\n", filename);

  deh_context_t *context = DEH_OpenFile(filename);

  if (context == nullptr) {
    fmt::fprintf(stderr, "DEH_LoadFile: Unable to open %s\n", filename);
    return 0;
  }

  DEH_ParseContext(context);

  DEH_CloseFile(context);

  if (DEH_HadError(context)) {
    I_Error("Error parsing dehacked file");
  }

  return 1;
}

// Load all dehacked patches from the given directory.
void DEH_AutoLoadPatches(const char *path) {
  glob_t *glob = I_StartMultiGlob(path, GLOB_FLAG_NOCASE | GLOB_FLAG_SORTED, "*.deh", "*.bex", "*.hhe", "*.seh", nullptr); // [crispy] *.bex
  for (;;) {
    const char *filename = I_NextGlob(glob);
    if (filename == nullptr) {
      break;
    }
    fmt::printf(" [autoload]");
    DEH_LoadFile(filename);
  }

  I_EndGlob(glob);
}

// Load dehacked file from WAD lump.
// If allow_long is set, allow long strings and cheats just for this lump.

int DEH_LoadLump(int lumpnum, bool, bool allow_error) {
  if (!deh_initialized) {
    DEH_Init();
  }

  // Reset all special flags to defaults.
  // [crispy] always allow everything
  /*
  deh_allow_long_strings = allow_long;
  deh_allow_long_cheats = allow_long;
  deh_allow_extended_strings = false;
*/

  deh_context_t *context = DEH_OpenLump(lumpnum);

  if (context == nullptr) {
    fmt::fprintf(stderr, "DEH_LoadFile: Unable to open lump %i\n", lumpnum);
    return 0;
  }

  DEH_ParseContext(context);

  DEH_CloseFile(context);

  // If there was an error while parsing, abort with an error, but allow
  // errors to just be ignored if allow_error=true.
  if (!allow_error && DEH_HadError(context)) {
    I_Error("Error parsing dehacked lump");
  }

  return 1;
}

int DEH_LoadLumpByName(const char *name, bool allow_long, bool allow_error) {
  int lumpnum = W_CheckNumForName(name);

  if (lumpnum == -1) {
    fmt::fprintf(stderr, "DEH_LoadLumpByName: '%s' lump not found\n", name);
    return 0;
  }

  return DEH_LoadLump(lumpnum, allow_long, allow_error);
}

// Check the command line for -deh argument, and others.
void DEH_ParseCommandLine() {
  //!
  // @arg <files>
  // @category mod
  //
  // Load the given dehacked patch(es)
  //

  int p = M_CheckParm("-deh");

  if (p > 0) {
    ++p;

    while (p < myargc && myargv[p][0] != '-') {
      char *filename = D_TryFindWADByName(myargv[p]);
      DEH_LoadFile(filename);
      free(filename);
      ++p;
    }
  }
}
