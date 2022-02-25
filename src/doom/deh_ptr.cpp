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
// Parses Action Pointer entries in dehacked files
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "deh_defs.hpp"
#include "deh_io.hpp"
#include "deh_main.hpp"
#include "info.hpp"

actionf_t codeptrs[NUMSTATES]; // [crispy] share with deh_bexptr.c

static int CodePointerIndex(actionf_t * ptr) {
  for (int i = 0; i < NUMSTATES; ++i) {
    if (!memcmp(&codeptrs[i], ptr, sizeof(actionf_t))) {
      return i;
    }
  }

  return -1;
}

static void DEH_PointerInit() {
  // Initialize list of dehacked pointers

  int index = 0;
  for (index = 0; index < EXTRASTATES; ++index)
    codeptrs[index] = states[index].action;

  // [BH] Initialize extra dehacked states
  for (; index < NUMSTATES; index++) {
    states[index].sprite    = SPR_TNT1;
    states[index].frame     = 0;
    states[index].tics      = -1;
    states[index].action    = null_hook();
    states[index].nextstate = static_cast<statenum_t>(index);
    states[index].misc1     = S_NULL;
    states[index].misc2     = 0;
    //	states[i].dehacked = false;
    codeptrs[index] = states[index].action;
  }
}

static void * DEH_PointerStart(deh_context_t * context, char * line) {
  int frame_number = 0;

  // FIXME: can the third argument here be something other than "Frame"
  // or are we ok?

  if (sscanf(line, "Pointer %*i (%*s %i)", &frame_number) != 1) {
    DEH_Warning(context, "Parse error on section start");
    return nullptr;
  }

  if (frame_number < 0 || frame_number >= NUMSTATES) {
    DEH_Warning(context, "Invalid frame number: %i", frame_number);
    return nullptr;
  }

  return &states[frame_number];
}

static void DEH_PointerParseLine(deh_context_t * context, char * line, void * tag) {
  char *variable_name = nullptr, *value = nullptr;

  if (tag == nullptr) return;

  auto * state = reinterpret_cast<state_t *>(tag);

  // Parse the assignment

  if (!DEH_ParseAssignment(line, &variable_name, &value)) {
    // Failed to parse
    DEH_Warning(context, "Failed to parse assignment");
    return;
  }

  //   fmt::printf("Set %s to %s for state\n", variable_name, value);

  // all values are integers

  int ivalue = std::atoi(value);

  // set the appropriate field

  if (!strcasecmp(variable_name, "Codep frame")) {
    if (ivalue < 0 || ivalue >= NUMSTATES) {
      DEH_Warning(context, "Invalid state '%i'", ivalue);
    } else {
      state->action = codeptrs[ivalue];
    }
  } else {
    DEH_Warning(context, "Unknown variable name '%s'", variable_name);
  }
}

static void DEH_PointerSHA1Sum(sha1_context_t * context) {
  for (auto & state : states) {
    SHA1_UpdateInt32(context, CodePointerIndex(&state.action));
  }
}

deh_section_t deh_section_pointer = {
  "Pointer",
  DEH_PointerInit,
  DEH_PointerStart,
  DEH_PointerParseLine,
  nullptr,
  DEH_PointerSHA1Sum,
};
