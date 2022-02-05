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
//	System interface for music.
//


#include <cstdlib>
#include <cstring>

#include "SDL.h"
#include "SDL_mixer.h"

#include "i_midipipe.hpp"

#include "config.h"
#include "doomtype.hpp"
#include "memio.hpp"
#include "mus2mid.hpp"

#include "deh_str.hpp"
#include "gusconf.hpp"
#include "i_sound.hpp"
#include "i_system.hpp"
#include "i_swap.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_misc.hpp"
#include "sha1.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#define MAXMIDLENGTH (96 * 1024)

static boolean music_initialized = false;

// If this is true, this module initialized SDL sound and has the
// responsibility to shut it down

static boolean sdl_was_initialized = false;

static boolean musicpaused = false;
static int     current_music_volume;

char *timidity_cfg_path = const_cast<char *>("");

static char *temp_timidity_cfg = NULL;

// If the temp_timidity_cfg config variable is set, generate a "wrapper"
// config file for Timidity to point to the actual config file. This
// is needed to inject a "dir" command so that the patches are read
// relative to the actual config file.

static boolean WriteWrapperTimidityConfig(char *write_path)
{
    char *path;
    FILE *fstream;

    if (!strcmp(timidity_cfg_path, ""))
    {
        return false;
    }

    fstream = fopen(write_path, "w");

    if (fstream == NULL)
    {
        return false;
    }

    path = M_DirName(timidity_cfg_path);
    fprintf(fstream, "dir %s\n", path);
    free(path);

    fprintf(fstream, "source %s\n", timidity_cfg_path);
    fclose(fstream);

    return true;
}

void I_InitTimidityConfig()
{
    char   *env_string;
    boolean success;

    temp_timidity_cfg = M_TempFile("timidity.cfg");

    if (snd_musicdevice == SNDDEVICE_GUS)
    {
        success = GUS_WriteConfig(temp_timidity_cfg);
    }
    else
    {
        success = WriteWrapperTimidityConfig(temp_timidity_cfg);
    }

    // Set the TIMIDITY_CFG environment variable to point to the temporary
    // config file.

    if (success)
    {
        env_string = M_StringJoin("TIMIDITY_CFG=", temp_timidity_cfg, NULL);
        putenv(env_string);
    }
    else
    {
        free(temp_timidity_cfg);
        temp_timidity_cfg = NULL;
    }
}

// Remove the temporary config file generated by I_InitTimidityConfig().

static void RemoveTimidityConfig()
{
    if (temp_timidity_cfg != NULL)
    {
        remove(temp_timidity_cfg);
        free(temp_timidity_cfg);
    }
}

// Shutdown music

static void I_SDL_ShutdownMusic()
{
    if (music_initialized)
    {
#if defined(_WIN32)
        I_MidiPipe_ShutdownServer();
#endif
        Mix_HaltMusic();
        music_initialized = false;

        if (sdl_was_initialized)
        {
            Mix_CloseAudio();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            sdl_was_initialized = false;
        }
    }
}

static boolean SDLIsInitialized()
{
    int    freq, channels;
    Uint16 format;

    return Mix_QuerySpec(&freq, &format, &channels) != 0;
}

// Initialize music subsystem
static boolean I_SDL_InitMusic()
{
    // If SDL_mixer is not initialized, we have to initialize it
    // and have the responsibility to shut it down later on.

    if (SDLIsInitialized())
    {
        music_initialized = true;
    }
    else
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            fprintf(stderr, "Unable to set up sound.\n");
        }
        else if (Mix_OpenAudio(snd_samplerate, AUDIO_S16SYS, 2, 1024) < 0)
        {
            fprintf(stderr, "Error initializing SDL_mixer: %s\n",
                Mix_GetError());
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }
        else
        {
            SDL_PauseAudio(0);

            sdl_was_initialized = true;
            music_initialized   = true;
        }
    }

#if defined(SDL_MIXER_VERSION_ATLEAST)
#if SDL_MIXER_VERSION_ATLEAST(2, 0, 2)
    // Initialize SDL_Mixer for MIDI music playback
    Mix_Init(MIX_INIT_MID | MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_MP3); // [crispy] initialize some more audio formats
#endif
#endif

    // Once initialization is complete, the temporary Timidity config
    // file can be removed.

    RemoveTimidityConfig();

    // If snd_musiccmd is set, we need to call Mix_SetMusicCMD to
    // configure an external music playback program.

    if (strlen(snd_musiccmd) > 0)
    {
        Mix_SetMusicCMD(snd_musiccmd);
    }

#if defined(_WIN32)
    // [AM] Start up midiproc to handle playing MIDI music.
    // Don't enable it for GUS, since it handles its own volume just fine.
    if (snd_musicdevice != SNDDEVICE_GUS)
    {
        I_MidiPipe_InitServer();
    }
#endif

    return music_initialized;
}

//
// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
//

static void UpdateMusicVolume()
{
    int vol;

    if (musicpaused)
    {
        vol = 0;
    }
    else
    {
        vol = (current_music_volume * MIX_MAX_VOLUME) / 127;
    }

#if defined(_WIN32)
    I_MidiPipe_SetVolume(vol);
#endif
    Mix_VolumeMusic(vol);
}

// Set music volume (0 - 127)

static void I_SDL_SetMusicVolume(int volume)
{
    // Internal state variable.
    current_music_volume = volume;

    UpdateMusicVolume();
}

// Start playing a mid

static void I_SDL_PlaySong(void *handle, boolean looping)
{
    int loops;

    if (!music_initialized)
    {
        return;
    }

    if (handle == NULL && !midi_server_registered)
    {
        return;
    }

    if (looping)
    {
        loops = -1;
    }
    else
    {
        loops = 1;
    }

#if defined(_WIN32)
    if (midi_server_registered)
    {
        I_MidiPipe_PlaySong(loops);
    }
    else
#endif
    {
        Mix_PlayMusic((Mix_Music *)handle, loops);
    }
}

static void I_SDL_PauseSong()
{
    if (!music_initialized)
    {
        return;
    }

    musicpaused = true;

    UpdateMusicVolume();
}

static void I_SDL_ResumeSong()
{
    if (!music_initialized)
    {
        return;
    }

    musicpaused = false;

    UpdateMusicVolume();
}

static void I_SDL_StopSong()
{
    if (!music_initialized)
    {
        return;
    }

#if defined(_WIN32)
    if (midi_server_registered)
    {
        I_MidiPipe_StopSong();
    }
    else
#endif
    {
        Mix_HaltMusic();
    }
}

static void I_SDL_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *)handle;

    if (!music_initialized)
    {
        return;
    }

#if defined(_WIN32)
    if (midi_server_registered)
    {
        I_MidiPipe_UnregisterSong();
    }
    else
#endif
    {
        if (handle != NULL)
        {
            Mix_FreeMusic(music);
        }
    }
}

// Determine whether memory block is a .mid file

// [crispy] Reverse Choco's logic from "if (MIDI)" to "if (not MUS)"
/*
static boolean IsMid(byte *mem, int len)
{
    return len > 4 && !memcmp(mem, "MThd", 4);
}
*/

static boolean ConvertMus(byte *musdata, int len, const char *filename)
{
    MEMFILE *instream;
    MEMFILE *outstream;
    void    *outbuf;
    size_t   outbuf_len;
    int      result;

    instream  = mem_fopen_read(musdata, len);
    outstream = mem_fopen_write();

    result = mus2mid(instream, outstream);

    if (result == 0)
    {
        mem_get_buf(outstream, &outbuf, &outbuf_len);

        M_WriteFile(filename, outbuf, outbuf_len);
    }

    mem_fclose(instream);
    mem_fclose(outstream);

    return result;
}

static void *I_SDL_RegisterSong(void *data, int len)
{
    char      *filename;
    Mix_Music *music;

    if (!music_initialized)
    {
        return NULL;
    }

    // MUS files begin with "MUS"
    // Reject anything which doesnt have this signature

    filename = M_TempFile("doom"); // [crispy] generic filename

    // [crispy] Reverse Choco's logic from "if (MIDI)" to "if (not MUS)"
    // MUS is the only format that requires conversion,
    // let SDL_Mixer figure out the others
    /*
    if (IsMid(data, len) && len < MAXMIDLENGTH)
*/
    if (len < 4 || memcmp(data, "MUS\x1a", 4)) // [crispy] MUS_HEADER_MAGIC
    {
        M_WriteFile(filename, data, len);
    }
    else
    {
        // Assume a MUS file and try to convert

        ConvertMus(static_cast<byte *>(data), len, filename);
    }

    // Load the MIDI. In an ideal world we'd be using Mix_LoadMUS_RW()
    // by now, but Mix_SetMusicCMD() only works with Mix_LoadMUS(), so
    // we have to generate a temporary file.

#if defined(_WIN32)
    // [AM] If we do not have an external music command defined, play
    //      music with the MIDI server.
    if (midi_server_initialized)
    {
        music = NULL;
        if (!I_MidiPipe_RegisterSong(filename))
        {
            fprintf(stderr, "Error loading midi: %s\n",
                "Could not communicate with midiproc.");
        }
    }
    else
#endif
    {
        music = Mix_LoadMUS(filename);
        if (music == NULL)
        {
            // Failed to load
            fprintf(stderr, "Error loading midi: %s\n", Mix_GetError());
        }

        // Remove the temporary MIDI file; however, when using an external
        // MIDI program we can't delete the file. Otherwise, the program
        // won't find the file to play. This means we leave a mess on
        // disk :(

        if (strlen(snd_musiccmd) == 0)
        {
            remove(filename);
        }
    }

    free(filename);

    return music;
}

// Is the song playing?
static boolean I_SDL_MusicIsPlaying()
{
    if (!music_initialized)
    {
        return false;
    }

    return Mix_PlayingMusic();
}

static snddevice_t music_sdl_devices[] = {
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

music_module_t music_sdl_module = {
    music_sdl_devices,
    arrlen(music_sdl_devices),
    I_SDL_InitMusic,
    I_SDL_ShutdownMusic,
    I_SDL_SetMusicVolume,
    I_SDL_PauseSong,
    I_SDL_ResumeSong,
    I_SDL_RegisterSong,
    I_SDL_UnRegisterSong,
    I_SDL_PlaySong,
    I_SDL_StopSong,
    I_SDL_MusicIsPlaying,
    NULL, // Poll
};
