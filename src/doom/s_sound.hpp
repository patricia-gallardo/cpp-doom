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
//	The not so system specific sound interface.
//


#ifndef __S_SOUND__
#define __S_SOUND__

#include "p_mobj.hpp"
#include "sounds.hpp"

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume);


// Shut down sound

void S_Shutdown();


//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

void S_Start();

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//

void S_StartSound(void *origin, int sound_id);
void S_StartSoundOnce(void *origin, int sound_id);

// Stop sound for thing at <origin>
void S_StopSound(mobj_t *origin);
void S_UnlinkSound(mobj_t *origin);


// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
void S_ChangeMusic(int music_id, int looping);
void S_ChangeMusInfoMusic(int lumpnum, int looping);

// query if music is playing
bool S_MusicPlaying();

// Stops the music fer sure.
void S_StopMusic();

// Stop and resume music, during game PAUSE.
void S_PauseSound();
void S_ResumeSound();


//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t *listener);

void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);

extern int snd_channels;

void S_UpdateSndChannels();
void S_UpdateStereoSeparation();

#endif
