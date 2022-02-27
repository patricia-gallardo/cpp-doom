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

#pragma once

#include "deh_io.hpp"
#include "doomtype.hpp"
#include "sha1.hpp"

#define DEH_BEGIN_MAPPING(mapping_name, structname) \
  static structname    deh_mapping_base;            \
  static deh_mapping_t mapping_name = {             \
    &deh_mapping_base,                              \
    {

#define DEH_MAPPING(deh_name, fieldname) \
  { deh_name, &deh_mapping_base.fieldname, sizeof(deh_mapping_base.fieldname), false },

#define DEH_MAPPING_STRING(deh_name, fieldname) \
  { deh_name, &deh_mapping_base.fieldname, sizeof(deh_mapping_base.fieldname), true },

#define DEH_UNSUPPORTED_MAPPING(deh_name) \
  { deh_name, nullptr, -1, false },

#define DEH_END_MAPPING  \
  {                      \
    nullptr, nullptr, -1 \
  }                      \
  }                      \
  }                      \
  ;

constexpr auto MAX_MAPPING_ENTRIES = 32;

using deh_mapping_t       = struct deh_mapping_s;
using deh_mapping_entry_t = struct deh_mapping_entry_s;

struct deh_mapping_entry_s {
  // field name

  const char * name {};

  // location relative to the base in the deh_mapping_t struct
  // If this is nullptr, it is an unsupported mapping

  void * location {};

  // field size

  int size {};

  // if true, this is a string value.

  bool is_string {};
};

struct deh_mapping_s {
  void *              base {};
  deh_mapping_entry_t entries[MAX_MAPPING_ENTRIES];
};

bool                  DEH_SetMapping(deh_context_t * context, deh_mapping_t * mapping, void * structptr, char * name, int value);
[[maybe_unused]] bool DEH_SetStringMapping(deh_context_t * context, deh_mapping_t * mapping, void * structptr, char * name, char * value);
void                  DEH_StructSHA1Sum(sha1_context_t * context, deh_mapping_t * mapping, void * structptr);
