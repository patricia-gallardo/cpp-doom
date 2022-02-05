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
// DESCRIPTION:
//    PC speaker interface.
//

#ifndef PCSOUND_H
#define PCSOUND_H

using pcsound_callback_func = void (*)(int *, int *);

// Initialise the PC speaker subsystem.  The given function is called
// periodically to request more sound data to play.

int
  PCSound_Init(pcsound_callback_func callback_func);

// Shut down the PC speaker subsystem.

void
  PCSound_Shutdown();

// Set the preferred output sample rate when emulating a PC speaker.
// This must be called before PCSound_Init.

void
  PCSound_SetSampleRate(int rate);

#endif /* #ifndef PCSOUND_H */
