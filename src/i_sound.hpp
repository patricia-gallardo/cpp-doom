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


#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomtype.hpp"

// so that the individual game logic and sound driver code agree
#define NORM_PITCH 127

//
// SoundFX struct.
//
using sfxinfo_t = struct sfxinfo_struct;

struct sfxinfo_struct {
    // tag name, used for hexen.
    const char *tagname;

    // lump name.  If we are running with use_sfx_prefix=true, a
    // 'DS' (or 'DP' for PC speaker sounds) is prepended to this.

    char name[9];

    // Sfx priority
    int priority;

    // referenced sound if a link
    sfxinfo_t *link;

    // pitch if a link (Doom), whether to pitch-shift (Hexen)
    int pitch;

    // volume if a link
    int volume;

    // this is checked every second to see if sound
    // can be thrown out (if 0, then decrement, if -1,
    // then throw out, if > 0, then it is in use)
    int usefulness;

    // lump number of sfx
    int lumpnum;

    // Maximum number of channels that the sound can be played on
    // (Heretic)
    int numchannels;

    // data used by the low level code
    void *driver_data;
};

//
// MusicInfo struct.
//
using musicinfo_t = struct
{
    // up to 6-character name
    const char *name;

    // lump number of music
    int lumpnum;

    // music data
    void *data;

    // music handle once registered
    void *handle;

};

enum snddevice_t : int
{
    SNDDEVICE_NONE        = 0,
    SNDDEVICE_PCSPEAKER   = 1,
    SNDDEVICE_ADLIB       = 2,
    SNDDEVICE_SB          = 3,
    SNDDEVICE_PAS         = 4,
    SNDDEVICE_GUS         = 5,
    SNDDEVICE_WAVEBLASTER = 6,
    SNDDEVICE_SOUNDCANVAS = 7,
    SNDDEVICE_GENMIDI     = 8,
    SNDDEVICE_AWE32       = 9,
    SNDDEVICE_CD          = 10,
};

// Interface for sound modules

using sound_module_t = struct
{
    // List of sound devices that this sound module is used for.

    snddevice_t *sound_devices;
    int          num_sound_devices;

    // Initialise sound module
    // Returns true if successfully initialised

    boolean (*Init)(boolean use_sfx_prefix);

    // Shutdown sound module

    void (*Shutdown)();

    // Returns the lump index of the given sound.

    int (*GetSfxLumpNum)(sfxinfo_t *sfxinfo);

    // Called periodically to update the subsystem.

    void (*Update)();

    // Update the sound settings on the given channel.

    void (*UpdateSoundParams)(int channel, int vol, int sep);

    // Start a sound on a given channel.  Returns the channel id
    // or -1 on failure.

    int (*StartSound)(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch);

    // Stop the sound playing on the given channel.

    void (*StopSound)(int channel);

    // Query if a sound is playing on the given channel

    boolean (*SoundIsPlaying)(int channel);

    // Called on startup to precache sound effects (if necessary)

    void (*CacheSounds)(sfxinfo_t *sounds, int num_sounds);

};

void    I_InitSound(boolean use_sfx_prefix);
void    I_ShutdownSound();
int     I_GetSfxLumpNum(sfxinfo_t *sfxinfo);
void    I_UpdateSound();
void    I_UpdateSoundParams(int channel, int vol, int sep);
int     I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch);
void    I_StopSound(int channel);
boolean I_SoundIsPlaying(int channel);
void    I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds);

// Interface for music modules

using music_module_t = struct
{
    // List of sound devices that this music module is used for.

    snddevice_t *sound_devices;
    int          num_sound_devices;

    // Initialise the music subsystem

    boolean (*Init)();

    // Shutdown the music subsystem

    void (*Shutdown)();

    // Set music volume - range 0-127

    void (*SetMusicVolume)(int volume);

    // Pause music

    void (*PauseMusic)();

    // Un-pause music

    void (*ResumeMusic)();

    // Register a song handle from data
    // Returns a handle that can be used to play the song

    void *(*RegisterSong)(void *data, int len);

    // Un-register (free) song data

    void (*UnRegisterSong)(void *handle);

    // Play the song

    void (*PlaySong)(void *handle, boolean looping);

    // Stop playing the current song.

    void (*StopSong)();

    // Query if music is playing.

    boolean (*MusicIsPlaying)();

    // Invoked periodically to poll.

    void (*Poll)();
};

void    I_InitMusic();
void    I_ShutdownMusic();
void    I_SetMusicVolume(int volume);
void    I_PauseSong();
void    I_ResumeSong();
void *  I_RegisterSong(void *data, int len);
void    I_UnRegisterSong(void *handle);
void    I_PlaySong(void *handle, boolean looping);
void    I_StopSong();
boolean I_MusicIsPlaying();

extern int         snd_sfxdevice;
extern int         snd_musicdevice;
extern int         snd_samplerate;
extern int         snd_cachesize;
extern int         snd_maxslicetime_ms;
extern char *      snd_musiccmd;
extern int         snd_pitchshift;

void I_BindSoundVariables();

// DMX version to emulate for OPL emulation:
using opl_driver_ver_t = enum
{
    opl_doom1_1_666, // Doom 1 v1.666
    opl_doom2_1_666, // Doom 2 v1.666, Hexen, Heretic
    opl_doom_1_9     // Doom v1.9, Strife
};

void I_SetOPLDriverVer(opl_driver_ver_t ver);

#endif
