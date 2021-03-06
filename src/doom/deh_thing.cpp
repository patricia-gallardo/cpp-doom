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
// Parses "Thing" sections in dehacked files
//

#include <cstdio>
#include <cstdlib>

#include "deh_defs.hpp"
#include "deh_main.hpp"
#include "deh_mapping.hpp"
#include "doomtype.hpp"
#include "info.hpp"
#include "p_mobj.hpp" // [crispy] MF_*

struct bex_thingbits_t {
  const char * flag;
  unsigned int bits;
};

static constexpr bex_thingbits_t bex_thingbitstable[] = {
  {"SPECIAL",       MF_SPECIAL                               },
  { "SOLID",        MF_SOLID                                 },
  { "SHOOTABLE",    MF_SHOOTABLE                             },
  { "NOSECTOR",     MF_NOSECTOR                              },
  { "NOBLOCKMAP",   MF_NOBLOCKMAP                            },
  { "AMBUSH",       MF_AMBUSH                                },
  { "JUSTHIT",      MF_JUSTHIT                               },
  { "JUSTATTACKED", MF_JUSTATTACKED                          },
  { "SPAWNCEILING", MF_SPAWNCEILING                          },
  { "NOGRAVITY",    MF_NOGRAVITY                             },
  { "DROPOFF",      MF_DROPOFF                               },
  { "PICKUP",       MF_PICKUP                                },
  { "NOCLIP",       MF_NOCLIP                                },
  { "SLIDE",        MF_SLIDE                                 },
  { "FLOAT",        MF_FLOAT                                 },
  { "TELEPORT",     MF_TELEPORT                              },
  { "MISSILE",      MF_MISSILE                               },
  { "DROPPED",      MF_DROPPED                               },
  { "SHADOW",       MF_SHADOW                                },
  { "NOBLOOD",      MF_NOBLOOD                               },
  { "CORPSE",       MF_CORPSE                                },
  { "INFLOAT",      MF_INFLOAT                               },
  { "COUNTKILL",    MF_COUNTKILL                             },
  { "COUNTITEM",    MF_COUNTITEM                             },
  { "SKULLFLY",     MF_SKULLFLY                              },
  { "NOTDMATCH",    MF_NOTDMATCH                             },
  { "TRANSLUCENT",  static_cast<unsigned int>(MF_TRANSLUCENT)},
 // TRANSLATION consists of 2 bits, not 1
  { "TRANSLATION",  0x04000000                               },
  { "TRANSLATION1", 0x04000000                               },
  { "TRANSLATION2", 0x08000000                               },
 // unused bits, for Boom compatibility
  { "UNUSED1",      0x08000000                               },
  { "UNUSED2",      0x10000000                               },
  { "UNUSED3",      0x20000000                               },
  { "UNUSED4",      0x40000000                               },
};

DEH_BEGIN_MAPPING(thing_mapping, mobjinfo_t)
DEH_MAPPING("ID #", doomednum)
DEH_MAPPING("Initial frame", spawnstate)
DEH_MAPPING("Hit points", spawnhealth)
DEH_MAPPING("First moving frame", seestate)
DEH_MAPPING("Alert sound", seesound)
DEH_MAPPING("Reaction time", reactiontime)
DEH_MAPPING("Attack sound", attacksound)
DEH_MAPPING("Injury frame", painstate)
DEH_MAPPING("Pain chance", painchance)
DEH_MAPPING("Pain sound", painsound)
DEH_MAPPING("Close attack frame", meleestate)
DEH_MAPPING("Far attack frame", missilestate)
DEH_MAPPING("Death frame", deathstate)
DEH_MAPPING("Exploding frame", xdeathstate)
DEH_MAPPING("Death sound", deathsound)
DEH_MAPPING("Speed", speed)
DEH_MAPPING("Width", radius)
DEH_MAPPING("Height", height)
DEH_MAPPING("Mass", mass)
DEH_MAPPING("Missile damage", damage)
DEH_MAPPING("Action sound", activesound)
DEH_MAPPING("Bits", flags)
DEH_MAPPING("Respawn frame", raisestate)
DEH_END_MAPPING

static void * DEH_ThingStart(deh_context_t * context, char * line) {
  int thing_number = 0;

  if (sscanf(line, "Thing %i", &thing_number) != 1) {
    DEH_Warning(context, "Parse error on section start");
    return nullptr;
  }

  // dehacked files are indexed from 1
  --thing_number;

  if (thing_number < 0 || thing_number >= NUMMOBJTYPES) {
    DEH_Warning(context, "Invalid thing number: %i", thing_number);
    return nullptr;
  }

  mobjinfo_t * mobj = &mobjinfo[thing_number];

  return mobj;
}

static void DEH_ThingParseLine(deh_context_t * context, char * line, void * tag) {
  char *variable_name = nullptr, *value = nullptr;

  if (tag == nullptr)
    return;

  auto * mobj = reinterpret_cast<mobjinfo_t *>(tag);

  // Parse the assignment

  if (!DEH_ParseAssignment(line, &variable_name, &value)) {
    // Failed to parse

    DEH_Warning(context, "Failed to parse assignment");
    return;
  }

  //   fmt::printf("Set %s to %s for mobj\n", variable_name, value);

  // all values are integers

  int ivalue = std::atoi(value);

  // [crispy] support BEX bits mnemonics in Things fields
  if (!ivalue && iequals(variable_name, "bits")) {
    for (; (value = strtok(value, ",+| \t\f\r")); value = nullptr) {
      for (auto i : bex_thingbitstable)
        if (iequals(value, i.flag)) {
          ivalue |= i.bits;
          break;
        }
    }
  }

  // Set the field value

  DEH_SetMapping(context, &thing_mapping, mobj, variable_name, ivalue);
}

static void DEH_ThingSHA1Sum(sha1_context_t * context) {
  for (auto & i : mobjinfo) {
    DEH_StructSHA1Sum(context, &thing_mapping, &i);
  }
}

deh_section_t deh_section_thing = {
  "Thing",
  nullptr,
  DEH_ThingStart,
  DEH_ThingParseLine,
  nullptr,
  DEH_ThingSHA1Sum,
};
