//
// Copyright(C) 2013 James Haley et al.
// Copyright(C) 2017 Alex Mayfield
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
//     Client Interface to Midi Server
//

#ifndef __I_MIDIPIPE__
#define __I_MIDIPIPE__

#if _WIN32

#include "SDL_mixer.h"

#include "doomtype.hpp"

extern bool midi_server_initialized;
extern bool midi_server_registered;

bool I_MidiPipe_RegisterSong(char * filename);
void I_MidiPipe_UnregisterSong();
void I_MidiPipe_SetVolume(int vol);
void I_MidiPipe_PlaySong(int loops);
void I_MidiPipe_StopSong();
void I_MidiPipe_ShutdownServer();

bool I_MidiPipe_InitServer();

#else

#include "doomtype.hpp"

static const bool midi_server_registered = false;

#endif

#endif
