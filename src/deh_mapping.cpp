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
// Dehacked "mapping" code
// Allows the fields in structures to be mapped out and accessed by
// name
//

#include <cstring>

#include <fmt/printf.h>

#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_misc.hpp"

#include "deh_mapping.hpp"

static deh_mapping_entry_t * GetMappingEntryByName(deh_context_t * context,
                                                   deh_mapping_t * mapping,
                                                   char *          name) {
  for (int i = 0; mapping->entries[i].name != nullptr; ++i) {
    deh_mapping_entry_t * entry = &mapping->entries[i];

    if (!strcasecmp(entry->name, name)) {
      if (entry->location == nullptr) {
        DEH_Warning(context, "Field '%s' is unsupported", name);
        return nullptr;
      }

      return entry;
    }
  }

  // Not found.

  DEH_Warning(context, "Field named '%s' not found", name);

  return nullptr;
}

//
// Get the location of the specified field in the specified structure.
//

static void * GetStructField(void *                structptr,
                             deh_mapping_t *       mapping,
                             deh_mapping_entry_t * entry) {
  auto * location_byte_ptr = reinterpret_cast<uint8_t *>(entry->location);
  auto * base_byte_ptr     = reinterpret_cast<uint8_t *>(mapping->base);
  auto   offset            = static_cast<unsigned int>(location_byte_ptr - base_byte_ptr);
  return reinterpret_cast<uint8_t *>(structptr) + offset;
}

//
// Set the value of a particular field in a structure by name
//

bool DEH_SetMapping(deh_context_t * context, deh_mapping_t * mapping, void * structptr, char * name, int value) {
  deh_mapping_entry_t * entry = GetMappingEntryByName(context, mapping, name);

  if (entry == nullptr) {
    return false;
  }

  // Sanity check:

  if (entry->is_string) {
    DEH_Error(context, "Tried to set '%s' as integer (BUG)", name);
    return false;
  }

  void * location = GetStructField(structptr, mapping, entry);

  //      fmt::printf("Setting %p::%s to %i (%i bytes)\n",
  //               structptr, name, value, entry->size);

  // Set field content based on its type:

  switch (entry->size) {
  case 1:
    *(reinterpret_cast<uint8_t *>(location)) = static_cast<uint8_t>(value);
    break;
  case 2:
    *(reinterpret_cast<uint16_t *>(location)) = static_cast<uint16_t>(value);
    break;
  case 4:
    *(reinterpret_cast<uint32_t *>(location)) = static_cast<uint32_t>(value);
    break;
  default:
    DEH_Error(context, "Unknown field type for '%s' (BUG)", name);
    return false;
  }

  return true;
}

//
// Set the value of a string field in a structure by name
//

[[maybe_unused]] bool DEH_SetStringMapping(deh_context_t * context, deh_mapping_t * mapping, void * structptr, char * name, char * value) {
  deh_mapping_entry_t * entry = GetMappingEntryByName(context, mapping, name);

  if (entry == nullptr) {
    return false;
  }

  // Sanity check:

  if (!entry->is_string) {
    DEH_Error(context, "Tried to set '%s' as string (BUG)", name);
    return false;
  }

  auto * location = static_cast<char *>(GetStructField(structptr, mapping, entry));

  // Copy value into field:

  M_StringCopy(location, value, static_cast<size_t>(entry->size));

  return true;
}

void DEH_StructSHA1Sum(sha1_context_t * context, deh_mapping_t * mapping, void * structptr) {
  // Go through each mapping

  for (int i = 0; mapping->entries[i].name != nullptr; ++i) {
    deh_mapping_entry_t * entry = &mapping->entries[i];

    if (entry->location == nullptr) {
      // Unsupported field

      continue;
    }

    // Add in data for this field

    void * location = reinterpret_cast<uint8_t *>(structptr) + (reinterpret_cast<uint8_t *>(entry->location) - reinterpret_cast<uint8_t *>(mapping->base));

    switch (entry->size) {
    case 1:
      SHA1_UpdateInt32(context, *(reinterpret_cast<uint8_t *>(location)));
      break;
    case 2:
      SHA1_UpdateInt32(context, *(reinterpret_cast<uint16_t *>(location)));
      break;
    case 4:
      SHA1_UpdateInt32(context, *(reinterpret_cast<uint32_t *>(location)));
      break;
    default:
      I_Error("Unknown dehacked mapping field type for '%s' (BUG)",
              entry->name);
      break;
    }
  }
}
