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
//	Cheat code checking.
//

#pragma once

//
// CHEAT SEQUENCE PACKAGE
//

// declaring a cheat

#define CHEAT(value, parameters)                   \
  {                                                \
    value, sizeof(value) - 1, parameters, 0, 0, "" \
  }

constexpr auto MAX_CHEAT_LEN    = 25;
constexpr auto MAX_CHEAT_PARAMS = 5;

struct cheatseq_t {
  // settings for this cheat

  char   sequence[MAX_CHEAT_LEN] {};
  size_t sequence_len {};
  int    parameter_chars {};

  // state used during the game

  size_t chars_read {};
  int    param_chars_read {};
  char   parameter_buf[MAX_CHEAT_PARAMS] {};
};

int cht_CheckCheat(cheatseq_t * cht,
                   char         key);

void cht_GetParam(cheatseq_t * cht,
                  char *       buffer);
