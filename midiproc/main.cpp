//
// Copyright(C) 2012 James Haley
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
//
// Win32/SDL_mixer MIDI Server
//
// Uses pipes to communicate with Doom. This allows this separate process to
// have its own independent volume control even under Windows Vista and up's
// broken, stupid, completely useless mixer model that can't assign separate
// volumes to different devices for the same process.
//
// Seriously, how did they screw up something so fundamental?
//

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// intentional newline to avoid reordering by Clang Format

#include <cstdio>
#include <cstdlib>

#include <SDL.h>
#include <SDL_mixer.h>

#include "buffer.hpp"
#include "proto.hpp"

#include "doomtype.hpp"

static HANDLE midi_process_in;  // Standard In.
static HANDLE midi_process_out; // Standard Out.

// Sound sample rate to use for digital output (Hz)
static int snd_samplerate = 0;

// Currently playing music track.
static Mix_Music * music = nullptr;

//=============================================================================
//
// Private functions
//

//
// Write an unsigned integer into a simple CHAR buffer.
//
static bool WriteInt16(CHAR * out, size_t osize, unsigned int in) {
  if (osize < 2) {
    return false;
  }

  out[0] = (in >> 8) & 0xff;
  out[1] = in & 0xff;

  return true;
}

//
// Cleanly close our in-use pipes.
//
static void FreePipes() {
  if (midi_process_in != nullptr) {
    CloseHandle(midi_process_in);
    midi_process_in = nullptr;
  }
  if (midi_process_out != nullptr) {
    CloseHandle(midi_process_out);
    midi_process_out = nullptr;
  }
}

//
// Unregisters the currently playing song.  This is never called from the
// protocol, we simply do this before playing a new song.
//
static void UnregisterSong() {
  if (music == nullptr) {
    return;
  }

  Mix_FreeMusic(music);
  music = nullptr;
}

//
// Cleanly shut down SDL.
//
static void ShutdownSDL() {
  UnregisterSong();
  Mix_CloseAudio();
  SDL_Quit();
}

//=============================================================================
//
// SDL_mixer Interface
//

static bool RegisterSong(cstring_view filename) {
  music = Mix_LoadMUS(filename.c_str());

  // Remove the temporary MIDI file
  remove(filename.c_str());

  if (music == nullptr) {
    return false;
  }

  return true;
}

static void SetVolume(int vol) {
  Mix_VolumeMusic(vol);
}

static void PlaySong(int loops) {
  Mix_PlayMusic(music, loops);

  // [AM] BUG: In my testing, setting the volume of a MIDI track while there
  //      is no song playing appears to be a no-op.  This can happen when
  //      you're mixing midiproc with vanilla SDL_Mixer, such as when you
  //      are alternating between a digital music pack (in the parent
  //      process) and MIDI (in this process).
  //
  //      To work around this bug, we set the volume to itself after the MIDI
  //      has started playing.
  Mix_VolumeMusic(Mix_VolumeMusic(-1));
}

static void StopSong() {
  Mix_HaltMusic();
}

//=============================================================================
//
// Pipe Server Interface
//

static bool MidiPipe_RegisterSong(buffer_reader_t * reader) {
  char * filename = Reader_ReadString(reader);
  if (filename == nullptr) {
    return false;
  }

  return RegisterSong(filename);
}

static bool MidiPipe_UnregisterSong(buffer_reader_t *) {
  UnregisterSong();
  return true;
}

bool MidiPipe_SetVolume(buffer_reader_t * reader) {
  int  vol;
  bool ok = Reader_ReadInt32(reader, (uint32_t *)&vol);
  if (!ok) {
    return false;
  }

  SetVolume(vol);

  return true;
}

bool MidiPipe_PlaySong(buffer_reader_t * reader) {
  int  loops;
  bool ok = Reader_ReadInt32(reader, (uint32_t *)&loops);
  if (!ok) {
    return false;
  }

  PlaySong(loops);

  return true;
}

bool MidiPipe_StopSong() {
  StopSong();

  return true;
}

bool MidiPipe_Shutdown() {
  exit(EXIT_SUCCESS);
}

//=============================================================================
//
// Server Implementation
//

//
// Parses a command and directs to the proper read function.
//
bool ParseCommand(buffer_reader_t * reader, uint16_t command) {
  switch (command) {
  case MIDIPIPE_PACKET_TYPE_REGISTER_SONG:
    return MidiPipe_RegisterSong(reader);
  case MIDIPIPE_PACKET_TYPE_UNREGISTER_SONG:
    return MidiPipe_UnregisterSong(reader);
  case MIDIPIPE_PACKET_TYPE_SET_VOLUME:
    return MidiPipe_SetVolume(reader);
  case MIDIPIPE_PACKET_TYPE_PLAY_SONG:
    return MidiPipe_PlaySong(reader);
  case MIDIPIPE_PACKET_TYPE_STOP_SONG:
    return MidiPipe_StopSong();
  case MIDIPIPE_PACKET_TYPE_SHUTDOWN:
    return MidiPipe_Shutdown();
  default:
    return false;
  }
}

//
// Server packet parser
//
bool ParseMessage(buffer_t * buf) {
  CHAR              buffer[2];
  DWORD             bytes_written;
  int               bytes_read;
  uint16_t          command;
  buffer_reader_t * reader = NewReader(buf);

  // Attempt to read a command out of the buffer.
  if (!Reader_ReadInt16(reader, &command)) {
    goto fail;
  }

  // Attempt to parse a complete message.
  if (!ParseCommand(reader, command)) {
    goto fail;
  }

  // We parsed a complete message!  We can now safely shift
  // the prior message off the front of the buffer.
  bytes_read = Reader_BytesRead(reader);
  DeleteReader(reader);
  Buffer_Shift(buf, bytes_read);

  // Send acknowledgement back that the command has completed.
  if (!WriteInt16(buffer, sizeof(buffer), MIDIPIPE_PACKET_TYPE_ACK)) {
    goto fail;
  }

  WriteFile(midi_process_out, buffer, sizeof(buffer), &bytes_written, nullptr);

  return true;

fail:
  // We did not read a complete packet.  Delete our reader and try again
  // with more data.
  DeleteReader(reader);
  return false;
}

//
// The main pipe "listening" loop
//
bool ListenForever() {
  BOOL  wok = FALSE;
  CHAR  pipe_buffer[8192];
  DWORD pipe_buffer_read = 0;

  bool       ok     = false;
  buffer_t * buffer = NewBuffer();

  for (;;) {
    // Wait until we see some data on the pipe.
    wok = PeekNamedPipe(midi_process_in, nullptr, 0, nullptr, &pipe_buffer_read, nullptr);
    if (!wok) {
      break;
    } else if (pipe_buffer_read == 0) {
      SDL_Delay(1);
      continue;
    }

    // Read data off the pipe and add it to the buffer.
    wok = ReadFile(midi_process_in, pipe_buffer, sizeof(pipe_buffer), &pipe_buffer_read, nullptr);
    if (!wok) {
      break;
    }

    ok = Buffer_Push(buffer, pipe_buffer, pipe_buffer_read);
    if (!ok) {
      break;
    }

    do {
      // Read messages off the buffer until we can't anymore.
      ok = ParseMessage(buffer);
    } while (ok);
  }

  return false;
}

//=============================================================================
//
// Main Program
//

//
// InitSDL
//
// Start up SDL and SDL_mixer.
//
bool InitSDL() {
  if (SDL_Init(SDL_INIT_AUDIO) == -1) {
    return false;
  }

  if (Mix_OpenAudio(snd_samplerate, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    return false;
  }

  atexit(ShutdownSDL);

  return true;
}

//
// InitPipes
//
// Ensure that we can communicate.
//
void InitPipes(HANDLE in, HANDLE out) {
  midi_process_in  = in;
  midi_process_out = out;

  atexit(FreePipes);
}

//
// main
//
// Application entry point.
//
int main(int argc, char * argv[]) {
  HANDLE in, out;

  // Make sure we're not launching this process by itself.
  if (argc < 5) {
    MessageBox(nullptr, TEXT("This program is tasked with playing Native ") TEXT("MIDI music, and is intended to be launched by ") TEXT(PACKAGE_NAME) TEXT("."), TEXT(PACKAGE_STRING), MB_OK | MB_ICONASTERISK);

    return EXIT_FAILURE;
  }

  // Make sure our Choccolate Doom and midiproc version are lined up.
  if (strcmp(PACKAGE_STRING, argv[1]) != 0) {
    char message[1024];
    _snprintf(message, sizeof(message), "It appears that the version of %s and %smidiproc are out "
                                        "of sync.  Please reinstall %s.\r\n\r\n"
                                        "Server Version: %s\r\nClient Version: %s",
              PACKAGE_NAME,
              PROGRAM_PREFIX,
              PACKAGE_NAME,
              PACKAGE_STRING,
              argv[1]);
    message[sizeof(message) - 1] = '\0';

    MessageBox(nullptr, TEXT(message), TEXT(PACKAGE_STRING), MB_OK | MB_ICONASTERISK);

    return EXIT_FAILURE;
  }

  // Parse out the sample rate - if we can't, default to 44100.
  snd_samplerate = strtol(argv[2], nullptr, 10);
  if (snd_samplerate == LONG_MAX || snd_samplerate == LONG_MIN || snd_samplerate == 0) {
    snd_samplerate = 44100;
  }

  // Parse out our handle ids.
  long in_long   = strtol(argv[3], nullptr, 10);
  auto in_intptr = static_cast<intptr_t>(in_long);
  in             = reinterpret_cast<HANDLE>(in_intptr);
  if (in == 0) {
    return EXIT_FAILURE;
  }

  long out_long   = strtol(argv[4], nullptr, 10);
  auto out_intptr = static_cast<intptr_t>(out_long);
  out             = reinterpret_cast<HANDLE>(out_intptr);
  if (out == 0) {
    return EXIT_FAILURE;
  }

  InitPipes(in, out);

  if (!InitSDL()) {
    return EXIT_FAILURE;
  }

  if (!ListenForever()) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#endif // #ifdef _WIN32
