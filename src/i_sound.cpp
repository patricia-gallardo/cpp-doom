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
// DESCRIPTION:  none
//

#include <cstdio>

#include "SDL_mixer.h"

#include "config.h"
#include "doomtype.hpp"

#include "gusconf.hpp"
#include "i_sound.hpp"
#include "i_video.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"

// Sound sample rate to use for digital output (Hz)

int snd_samplerate = 44100;

// Maximum number of bytes to dedicate to allocated sound effects.
// (Default: 64MB)

int snd_cachesize = 64 * 1024 * 1024;

// Config variable that controls the sound buffer size.
// We default to 28ms (1000 / 35fps = 1 buffer per tic).

int snd_maxslicetime_ms = 28;

// External command to invoke to play back music.

char *snd_musiccmd = const_cast<char *>("");

// Whether to vary the pitch of sound effects
// Each game will set the default differently

int snd_pitchshift = -1;

int snd_musicdevice = SNDDEVICE_SB;
int snd_sfxdevice   = SNDDEVICE_SB;

// Low-level sound and music modules we are using
static sound_module_t *sound_module;
static music_module_t *music_module;

// If true, the music pack module was successfully initialized.
static bool music_packs_active = false;

// This is either equal to music_module or &music_pack_module,
// depending on whether the current track is substituted.
static music_module_t *active_music_module;

// Sound modules

extern void           I_InitTimidityConfig();
extern sound_module_t sound_sdl_module;
extern sound_module_t sound_pcsound_module;
extern music_module_t music_sdl_module;
extern music_module_t music_opl_module;
extern music_module_t music_pack_module;

// For OPL module:

extern opl_driver_ver_t opl_drv_ver;
extern int              opl_io_port;

// For native music module:

extern char *music_pack_path;
extern char *timidity_cfg_path;

// DOS-specific options: These are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_sbport = 0;
static int snd_sbirq  = 0;
static int snd_sbdma  = 0;
static int snd_mport  = 0;

// Compiled-in sound modules:

static sound_module_t *sound_modules[] = {
    &sound_sdl_module,
    &sound_pcsound_module,
    nullptr,
};

// Compiled-in music modules:

static music_module_t *music_modules[] = {
    &music_sdl_module,
    &music_opl_module,
    nullptr,
};

// Check if a sound device is in the given list of devices

static bool SndDeviceInList(snddevice_t device, snddevice_t *list,
    int len)
{
    int i;

    for (i = 0; i < len; ++i)
    {
        if (device == list[i])
        {
            return true;
        }
    }

    return false;
}

// Find and initialize a sound_module_t appropriate for the setting
// in snd_sfxdevice.

static void InitSfxModule(bool use_sfx_prefix)
{
    int i;

    sound_module = nullptr;

    for (i = 0; sound_modules[i] != nullptr; ++i)
    {
        // Is the sfx device in the list of devices supported by
        // this module?

        if (SndDeviceInList(static_cast<snddevice_t>(snd_sfxdevice),
                sound_modules[i]->sound_devices,
                sound_modules[i]->num_sound_devices))
        {
            // Initialize the module

            if (sound_modules[i]->Init(use_sfx_prefix))
            {
                sound_module = sound_modules[i];
                return;
            }
        }
    }
}

// Initialize music according to snd_musicdevice.

static void InitMusicModule()
{
    int i;

    music_module = nullptr;

    for (i = 0; music_modules[i] != nullptr; ++i)
    {
        // Is the music device in the list of devices supported
        // by this module?

        if (SndDeviceInList(static_cast<snddevice_t>(snd_musicdevice),
                music_modules[i]->sound_devices,
                music_modules[i]->num_sound_devices))
        {
            // Initialize the module

            if (music_modules[i]->Init())
            {
                music_module = music_modules[i];
                return;
            }
        }
    }
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void I_InitSound(bool use_sfx_prefix)
{
    bool nosound, nosfx, nomusic, nomusicpacks;

    //!
    // @vanilla
    //
    // Disable all sound output.
    //

    nosound = M_CheckParm("-nosound") > 0;

    //!
    // @vanilla
    //
    // Disable sound effects.
    //

    nosfx = M_CheckParm("-nosfx") > 0;

    //!
    // @vanilla
    //
    // Disable music.
    //

    nomusic = M_CheckParm("-nomusic") > 0;

    //!
    //
    // Disable substitution music packs.
    //

    nomusicpacks = M_ParmExists("-nomusicpacks");

    // Auto configure the music pack directory.
    M_SetMusicPackDir();

    // Initialize the sound and music subsystems.

    if (!nosound && !screensaver_mode)
    {
        // This is kind of a hack. If native MIDI is enabled, set up
        // the TIMIDITY_CFG environment variable here before SDL_mixer
        // is opened.

        if (!nomusic
            && (snd_musicdevice == SNDDEVICE_GENMIDI
                || snd_musicdevice == SNDDEVICE_GUS))
        {
            I_InitTimidityConfig();
        }

        if (!nosfx)
        {
            InitSfxModule(use_sfx_prefix);
        }

        if (!nomusic)
        {
            InitMusicModule();
            active_music_module = music_module;
        }

        // We may also have substitute MIDIs we can load.
        if (!nomusicpacks && music_module != nullptr)
        {
            music_packs_active = music_pack_module.Init();
        }
    }
    // [crispy] print the SDL audio backend
    {
        const char *driver_name = SDL_GetCurrentAudioDriver();

        fprintf(stderr, "I_InitSound: SDL audio driver is %s\n", driver_name ? driver_name : "none");
    }
}

void I_ShutdownSound()
{
    if (sound_module != nullptr)
    {
        sound_module->Shutdown();
    }

    if (music_packs_active)
    {
        music_pack_module.Shutdown();
    }

    if (music_module != nullptr)
    {
        music_module->Shutdown();
    }
}

int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    if (sound_module != nullptr)
    {
        return sound_module->GetSfxLumpNum(sfxinfo);
    }
    else
    {
        return 0;
    }
}

void I_UpdateSound()
{
    if (sound_module != nullptr)
    {
        sound_module->Update();
    }

    if (active_music_module != nullptr && active_music_module->Poll != nullptr)
    {
        active_music_module->Poll();
    }
}

static void CheckVolumeSeparation(int *vol, int *sep)
{
    if (*sep < 0)
    {
        *sep = 0;
    }
    else if (*sep > 254)
    {
        *sep = 254;
    }

    if (*vol < 0)
    {
        *vol = 0;
    }
    else if (*vol > 127)
    {
        *vol = 127;
    }
}

void I_UpdateSoundParams(int channel, int vol, int sep)
{
    if (sound_module != nullptr)
    {
        CheckVolumeSeparation(&vol, &sep);
        sound_module->UpdateSoundParams(channel, vol, sep);
    }
}

int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch)
{
    if (sound_module != nullptr)
    {
        CheckVolumeSeparation(&vol, &sep);
        return sound_module->StartSound(sfxinfo, channel, vol, sep, pitch);
    }
    else
    {
        return 0;
    }
}

void I_StopSound(int channel)
{
    if (sound_module != nullptr)
    {
        sound_module->StopSound(channel);
    }
}

bool I_SoundIsPlaying(int channel)
{
    if (sound_module != nullptr)
    {
        return sound_module->SoundIsPlaying(channel);
    }
    else
    {
        return false;
    }
}

void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    if (sound_module != nullptr && sound_module->CacheSounds != nullptr)
    {
        sound_module->CacheSounds(sounds, num_sounds);
    }
}

void I_InitMusic()
{
}

void I_ShutdownMusic()
{
}

void I_SetMusicVolume(int volume)
{
    if (active_music_module != nullptr)
    {
        active_music_module->SetMusicVolume(volume);
    }
}

void I_PauseSong()
{
    if (active_music_module != nullptr)
    {
        active_music_module->PauseMusic();
    }
}

void I_ResumeSong()
{
    if (active_music_module != nullptr)
    {
        active_music_module->ResumeMusic();
    }
}

void *I_RegisterSong(void *data, int len)
{
    // If the music pack module is active, check to see if there is a
    // valid substitution for this track. If there is, we set the
    // active_music_module pointer to the music pack module for the
    // duration of this particular track.
    if (music_packs_active)
    {
        void *handle;

        handle = music_pack_module.RegisterSong(data, len);
        if (handle != nullptr)
        {
            active_music_module = &music_pack_module;
            return handle;
        }
    }

    // No substitution for this track, so use the main module.
    active_music_module = music_module;
    if (active_music_module != nullptr)
    {
        return active_music_module->RegisterSong(data, len);
    }
    else
    {
        return nullptr;
    }
}

void I_UnRegisterSong(void *handle)
{
    if (active_music_module != nullptr)
    {
        active_music_module->UnRegisterSong(handle);
    }
}

void I_PlaySong(void *handle, bool looping)
{
    if (active_music_module != nullptr)
    {
        active_music_module->PlaySong(handle, looping);
    }
}

void I_StopSong()
{
    if (active_music_module != nullptr)
    {
        active_music_module->StopSong();
    }
}

bool I_MusicIsPlaying()
{
    if (active_music_module != nullptr)
    {
        return active_music_module->MusicIsPlaying();
    }
    else
    {
        return false;
    }
}

void I_BindSoundVariables()
{
    extern char *snd_dmxoption;
    extern int   use_libsamplerate;
    extern float libsamplerate_scale;

    M_BindIntVariable("snd_musicdevice", reinterpret_cast<int *>(&snd_musicdevice));
    M_BindIntVariable("snd_sfxdevice", reinterpret_cast<int *>(&snd_sfxdevice));
    M_BindIntVariable("snd_sbport", &snd_sbport);
    M_BindIntVariable("snd_sbirq", &snd_sbirq);
    M_BindIntVariable("snd_sbdma", &snd_sbdma);
    M_BindIntVariable("snd_mport", &snd_mport);
    M_BindIntVariable("snd_maxslicetime_ms", &snd_maxslicetime_ms);
    M_BindStringVariable("snd_musiccmd", &snd_musiccmd);
    M_BindStringVariable("snd_dmxoption", &snd_dmxoption);
    M_BindIntVariable("snd_samplerate", &snd_samplerate);
    M_BindIntVariable("snd_cachesize", &snd_cachesize);
    M_BindIntVariable("opl_io_port", &opl_io_port);
    M_BindIntVariable("snd_pitchshift", &snd_pitchshift);

    M_BindStringVariable("music_pack_path", &music_pack_path);
    M_BindStringVariable("timidity_cfg_path", &timidity_cfg_path);
    M_BindStringVariable("gus_patch_path", &gus_patch_path);
    M_BindIntVariable("gus_ram_kb", &gus_ram_kb);

    M_BindIntVariable("use_libsamplerate", &use_libsamplerate);
    M_BindFloatVariable("libsamplerate_scale", &libsamplerate_scale);
}
