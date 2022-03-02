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

#include <array>
#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "config.h"
#include "doomtype.hpp"
#include "gusconf.hpp"
#include "i_midipipe.hpp"
#include "i_sound.hpp"
#include "i_swap.hpp"
#include "m_misc.hpp"
#include "memio.hpp"
#include "mus2mid.hpp"
#include "z_zone.hpp"

static bool music_initialized = false;

// If this is true, this module initialized SDL sound and has the
// responsibility to shut it down

static bool sdl_was_initialized = false;

static bool musicpaused = false;
static int  current_music_volume;

const char * timidity_cfg_path = ("");

static char * temp_timidity_cfg = nullptr;

// If the temp_timidity_cfg config variable is set, generate a "wrapper"
// config file for Timidity to point to the actual config file. This
// is needed to inject a "dir" command so that the patches are read
// relative to the actual config file.

static bool WriteWrapperTimidityConfig(char * write_path) {
  if (!strcmp(timidity_cfg_path, "")) {
    return false;
  }

  FILE * fstream = fopen(write_path, "w");

  if (fstream == nullptr) {
    return false;
  }

  char * path = M_DirName(timidity_cfg_path);
  fmt::fprintf(fstream, "dir %s\n", path);
  free(path);

  fmt::fprintf(fstream, "source %s\n", timidity_cfg_path);
  fclose(fstream);

  return true;
}

void I_InitTimidityConfig() {

  temp_timidity_cfg = M_TempFile("timidity.cfg");

  bool success = false;
  if (g_i_sound_globals->snd_musicdevice == SNDDEVICE_GUS) {
    success = GUS_WriteConfig(temp_timidity_cfg);
  } else {
    success = WriteWrapperTimidityConfig(temp_timidity_cfg);
  }

  // Set the TIMIDITY_CFG environment variable to point to the temporary
  // config file.

  if (success) {
    setenv("TIMIDITY_CFG", temp_timidity_cfg, 1);
  } else {
    free(temp_timidity_cfg);
    temp_timidity_cfg = nullptr;
  }
}

// Remove the temporary config file generated by I_InitTimidityConfig().

static void RemoveTimidityConfig() {
  if (temp_timidity_cfg != nullptr) {
    remove(temp_timidity_cfg);
    free(temp_timidity_cfg);
  }
}

// Shutdown music

static void I_SDL_ShutdownMusic() {
  if (music_initialized) {
#if defined(_WIN32)
    I_MidiPipe_ShutdownServer();
#endif
    Mix_HaltMusic();
    music_initialized = false;

    if (sdl_was_initialized) {
      Mix_CloseAudio();
      SDL_QuitSubSystem(SDL_INIT_AUDIO);
      sdl_was_initialized = false;
    }
  }
}

static bool SDLIsInitialized() {
  int    freq     = 0;
  int    channels = 0;
  Uint16 format   = 0;
  return Mix_QuerySpec(&freq, &format, &channels) != 0;
}

// Initialize music subsystem
static bool I_SDL_InitMusic() {
  // If SDL_mixer is not initialized, we have to initialize it
  // and have the responsibility to shut it down later on.

  if (SDLIsInitialized()) {
    music_initialized = true;
  } else {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      fmt::fprintf(stderr, "Unable to set up sound.\n");
    } else if (Mix_OpenAudio(g_i_sound_globals->snd_samplerate, AUDIO_S16SYS, 2, 1024) < 0) {
      fmt::fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
      SDL_QuitSubSystem(SDL_INIT_AUDIO);
    } else {
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

  if (strlen(g_i_sound_globals->snd_musiccmd) > 0) {
    Mix_SetMusicCMD(g_i_sound_globals->snd_musiccmd);
  }

#if defined(_WIN32)
  // [AM] Start up midiproc to handle playing MIDI music.
  // Don't enable it for GUS, since it handles its own volume just fine.
  if (g_i_sound_globals->snd_musicdevice != SNDDEVICE_GUS) {
    I_MidiPipe_InitServer();
  }
#endif

  return music_initialized;
}

//
// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
//

static void UpdateMusicVolume() {
  int vol = 0;
  if (musicpaused) {
    vol = 0;
  } else {
    vol = (current_music_volume * MIX_MAX_VOLUME) / 127;
  }

#if defined(_WIN32)
  I_MidiPipe_SetVolume(vol);
#endif
  Mix_VolumeMusic(vol);
}

// Set music volume (0 - 127)

static void I_SDL_SetMusicVolume(int volume) {
  // Internal state variable.
  current_music_volume = volume;

  UpdateMusicVolume();
}

// Start playing a mid

static void I_SDL_PlaySong(void * handle, bool looping) {
  if (!music_initialized) {
    return;
  }

  if (handle == nullptr && !midi_server_registered) {
    return;
  }

  int loops = 0;
  if (looping) {
    loops = -1;
  } else {
    loops = 1;
  }

#if defined(_WIN32)
  if (midi_server_registered) {
    I_MidiPipe_PlaySong(loops);
  } else
#endif
  {
    Mix_PlayMusic(reinterpret_cast<Mix_Music *>(handle), loops);
  }
}

static void I_SDL_PauseSong() {
  if (!music_initialized) {
    return;
  }

  musicpaused = true;

  UpdateMusicVolume();
}

static void I_SDL_ResumeSong() {
  if (!music_initialized) {
    return;
  }

  musicpaused = false;

  UpdateMusicVolume();
}

static void I_SDL_StopSong() {
  if (!music_initialized) {
    return;
  }

#if defined(_WIN32)
  if (midi_server_registered) {
    I_MidiPipe_StopSong();
  } else
#endif
  {
    Mix_HaltMusic();
  }
}

static void I_SDL_UnRegisterSong(void * handle) {
  auto * music = reinterpret_cast<Mix_Music *>(handle);

  if (!music_initialized) {
    return;
  }

#if defined(_WIN32)
  if (midi_server_registered) {
    I_MidiPipe_UnregisterSong();
  } else
#endif
  {
    if (handle != nullptr) {
      Mix_FreeMusic(music);
    }
  }
}

// Determine whether memory block is a .mid file

// [crispy] Reverse Choco's logic from "if (MIDI)" to "if (not MUS)"
/*
static bool IsMid(byte *mem, int len)
{
    return len > 4 && !memcmp(mem, "MThd", 4);
}
*/

static bool ConvertMus(uint8_t * musdata, int len, cstring_view filename) {
  MEMFILE * instream  = mem_fopen_read(musdata, static_cast<size_t>(len));
  MEMFILE * outstream = mem_fopen_write();

  int result = mus2mid(instream, outstream);

  if (result == 0) {
    void *    outbuf = nullptr;
    size_t    outbuf_len = 0;
    mem_get_buf(outstream, &outbuf, &outbuf_len);
    M_WriteFile(filename, outbuf, static_cast<int>(outbuf_len));
  }

  mem_fclose(instream);
  mem_fclose(outstream);

  return result;
}

static void * I_SDL_RegisterSong(void * data, int len) {
  if (!music_initialized) {
    return nullptr;
  }

  // MUS files begin with "MUS"
  // Reject anything which doesnt have this signature

  char * filename = M_TempFile("doom"); // [crispy] generic filename

  // [crispy] Reverse Choco's logic from "if (MIDI)" to "if (not MUS)"
  // MUS is the only format that requires conversion,
  // let SDL_Mixer figure out the others
  if (len < 4 || memcmp(data, "MUS\x1a", 4) != 0) // [crispy] MUS_HEADER_MAGIC
  {
    M_WriteFile(filename, data, len);
  } else {
    // Assume a MUS file and try to convert

    ConvertMus(static_cast<uint8_t *>(data), len, filename);
  }

  // Load the MIDI. In an ideal world we'd be using Mix_LoadMUS_RW()
  // by now, but Mix_SetMusicCMD() only works with Mix_LoadMUS(), so
  // we have to generate a temporary file.
  Mix_Music * music = nullptr;
#if defined(_WIN32)
  // [AM] If we do not have an external music command defined, play
  //      music with the MIDI server.
  if (midi_server_initialized) {
    music = nullptr;
    if (!I_MidiPipe_RegisterSong(filename)) {
      fmt::fprintf(stderr, "Error loading midi: %s\n", "Could not communicate with midiproc.");
    }
  } else
#endif
  {
    music = Mix_LoadMUS(filename);
    if (music == nullptr) {
      // Failed to load
      fmt::fprintf(stderr, "Error loading midi: %s\n", Mix_GetError());
    }

    // Remove the temporary MIDI file; however, when using an external
    // MIDI program we can't delete the file. Otherwise, the program
    // won't find the file to play. This means we leave a mess on
    // disk :(

    if (strlen(g_i_sound_globals->snd_musiccmd) == 0) {
      remove(filename);
    }
  }

  free(filename);

  return music;
}

// Is the song playing?
static bool I_SDL_MusicIsPlaying() {
  if (!music_initialized) {
    return false;
  }

  return Mix_PlayingMusic();
}

static std::array music_sdl_devices = {
  SNDDEVICE_PAS,
  SNDDEVICE_GUS,
  SNDDEVICE_WAVEBLASTER,
  SNDDEVICE_SOUNDCANVAS,
  SNDDEVICE_GENMIDI,
  SNDDEVICE_AWE32,
};

music_module_t music_sdl_module = {
  music_sdl_devices.data(),
  static_cast<int>(std::size(music_sdl_devices)),
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
  nullptr, // Poll
};
