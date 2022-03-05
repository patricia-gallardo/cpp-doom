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
//     OPL SDL interface.
//

#include <cassert>
#include <cstdio>

#include <fmt/printf.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "opl3.hpp"

#include "opl.hpp"
#include "opl_internal.hpp"

#include "opl_queue.hpp"

constexpr auto MAX_SOUND_SLICE_TIME = 100; /* ms */

struct opl_timer_t {
  unsigned int rate;        // Number of times the timer is advanced per sec.
  unsigned int enabled;     // Non-zero if timer is enabled.
  unsigned int value;       // Last value that was set.
  uint64_t     expire_time; // Calculated time that timer will expire.
};

// When the callback mutex is locked using OPL_Lock, callback functions
// are not invoked.

static SDL_mutex * callback_mutex = nullptr;

// Queue of callbacks waiting to be invoked.

static opl_callback_queue_t * callback_queue;

// Mutex used to control access to the callback queue.

static SDL_mutex * callback_queue_mutex = nullptr;

// Current time, in us since startup:

static uint64_t current_time;

// If non-zero, playback is currently paused.

static int opl_sdl_paused;

// Time offset (in us) due to the fact that callbacks
// were previously paused.

static uint64_t pause_offset;

// OPL software emulator structure.

static opl3_chip            opl_chip;
[[maybe_unused]] static int opl_opl3mode;

// Temporary mixing buffer used by the mixing callback.

static uint8_t * mix_buffer = nullptr;

// Register number that was written.

static int register_num = 0;

// Timers; DBOPL does not do timer stuff itself.

static opl_timer_t timer1 = { 12500, 0, 0, 0 };
static opl_timer_t timer2 = { 3125, 0, 0, 0 };

// SDL parameters.

static int    sdl_was_initialized = 0;
static int    mixing_freq, mixing_channels;
static Uint16 mixing_format;

static int SDLIsInitialized() {
  int    freq     = 0;
  int    channels = 0;
  Uint16 format   = 0;
  return Mix_QuerySpec(&freq, &format, &channels);
}

// Advance time by the specified number of samples, invoking any
// callback functions as appropriate.

static void AdvanceTime(unsigned int nsamples) {
  opl_callback_t callback {};
  void *         callback_data = nullptr;

  SDL_LockMutex(callback_queue_mutex);

  // Advance time.

  uint64_t us = static_cast<uint64_t>(nsamples * OPL_SECOND) / static_cast<unsigned long long int>(mixing_freq);
  current_time += us;

  if (opl_sdl_paused) {
    pause_offset += us;
  }

  // Are there callbacks to invoke now?  Keep invoking them
  // until there are no more left.

  while (!OPL_Queue_IsEmpty(callback_queue)
         && current_time >= OPL_Queue_Peek(callback_queue) + pause_offset) {
    // Pop the callback from the queue to invoke it.

    if (!OPL_Queue_Pop(callback_queue, &callback, &callback_data)) {
      break;
    }

    // The mutex stuff here is a bit complicated.  We must
    // hold callback_mutex when we invoke the callback (so that
    // the control thread can use OPL_Lock() to prevent callbacks
    // from being invoked), but we must not be holding
    // callback_queue_mutex, as the callback must be able to
    // call OPL_SetCallback to schedule new callbacks.

    SDL_UnlockMutex(callback_queue_mutex);

    SDL_LockMutex(callback_mutex);
    callback(callback_data);
    SDL_UnlockMutex(callback_mutex);

    SDL_LockMutex(callback_queue_mutex);
  }

  SDL_UnlockMutex(callback_queue_mutex);
}

// Call the OPL emulator code to fill the specified buffer.

static void FillBuffer(uint8_t * buffer, unsigned int nsamples) {
  // This seems like a reasonable assumption.  mix_buffer is
  // 1 second long, which should always be much longer than the
  // SDL mix buffer.
  assert(static_cast<int>(nsamples) < mixing_freq);

  // OPL output is generated into temporary buffer and then mixed
  // (to avoid overflows etc.)
  OPL3_GenerateStream(&opl_chip, reinterpret_cast<Bit16s *>(mix_buffer), nsamples);
  SDL_MixAudioFormat(buffer, mix_buffer, AUDIO_S16SYS, nsamples * 4, SDL_MIX_MAXVOLUME);
}

// Callback function to fill a new sound buffer:

static void OPL_Mix_Callback(void *, Uint8 * buffer, int len) {
  // Repeatedly call the OPL emulator update function until the buffer is
  // full.
  unsigned int filled         = 0;
  auto         buffer_samples = static_cast<unsigned int>(len / 4);

  while (filled < buffer_samples) {
    uint64_t nsamples = 0;

    SDL_LockMutex(callback_queue_mutex);

    // Work out the time until the next callback waiting in
    // the callback queue must be invoked.  We can then fill the
    // buffer with this many samples.

    if (opl_sdl_paused || OPL_Queue_IsEmpty(callback_queue)) {
      nsamples = buffer_samples - filled;
    } else {
      uint64_t next_callback_time = OPL_Queue_Peek(callback_queue) + pause_offset;

      nsamples = (next_callback_time - current_time) * static_cast<unsigned long long int>(mixing_freq);
      nsamples = (nsamples + OPL_SECOND - 1) / OPL_SECOND;

      if (nsamples > buffer_samples - filled) {
        nsamples = buffer_samples - filled;
      }
    }

    SDL_UnlockMutex(callback_queue_mutex);

    // Add emulator output to buffer.

    FillBuffer(buffer + filled * 4, static_cast<unsigned int>(nsamples));
    filled += static_cast<unsigned int>(nsamples);

    // Invoke callbacks for this point in time.

    AdvanceTime(static_cast<unsigned int>(nsamples));
  }
}

static void OPL_SDL_Shutdown() {
  Mix_HookMusic(nullptr, nullptr);

  if (sdl_was_initialized) {
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    OPL_Queue_Destroy(callback_queue);
    free(mix_buffer);
    sdl_was_initialized = 0;
  }

  /*
      if (opl_chip != nullptr)
      {
          OPLDestroy(opl_chip);
          opl_chip = nullptr;
      }
      */

  if (callback_mutex != nullptr) {
    SDL_DestroyMutex(callback_mutex);
    callback_mutex = nullptr;
  }

  if (callback_queue_mutex != nullptr) {
    SDL_DestroyMutex(callback_queue_mutex);
    callback_queue_mutex = nullptr;
  }
}

static unsigned int GetSliceSize() {
  int limit = (opl_sample_rate * MAX_SOUND_SLICE_TIME) / 1000;

  // Try all powers of two, not exceeding the limit.

  for (int n = 0;; ++n) {
    // 2^n <= limit < 2^n+1 ?

    if ((1 << (n + 1)) > limit) {
      return (1 << n);
    }
  }
  [[unreachable]];
}

static int OPL_SDL_Init(unsigned int) {
  // Check if SDL_mixer has been opened already
  // If not, we must initialize it now

  if (!SDLIsInitialized()) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      fmt::fprintf(stderr, "Unable to set up sound.\n");
      return 0;
    }

    if (Mix_OpenAudio(static_cast<int>(opl_sample_rate), AUDIO_S16SYS, 2, static_cast<int>(GetSliceSize())) < 0) {
      fmt::fprintf(stderr, "Error initialising SDL_mixer: %s\n", Mix_GetError());

      SDL_QuitSubSystem(SDL_INIT_AUDIO);
      return 0;
    }

    SDL_PauseAudio(0);

    // When this module shuts down, it has the responsibility to
    // shut down SDL.

    sdl_was_initialized = 1;
  } else {
    sdl_was_initialized = 0;
  }

  opl_sdl_paused = 0;
  pause_offset   = 0;

  // Queue structure of callbacks to invoke.

  callback_queue = OPL_Queue_Create();
  current_time   = 0;

  // Get the mixer frequency, format and number of channels.

  Mix_QuerySpec(&mixing_freq, &mixing_format, &mixing_channels);

  // Only supports AUDIO_S16SYS

  if (mixing_format != AUDIO_S16SYS || mixing_channels != 2) {
    fmt::fprintf(stderr,
                 "OPL_SDL only supports native signed 16-bit LSB, "
                 "stereo format!\n");

    OPL_SDL_Shutdown();
    return 0;
  }

  // Mix buffer: four bytes per sample (16 bits * 2 channels):
  mix_buffer = static_cast<uint8_t *>(malloc(static_cast<size_t>(mixing_freq * 4)));

  // Create the emulator structure:

  OPL3_Reset(&opl_chip, static_cast<Bit32u>(mixing_freq));
  opl_opl3mode = 0;

  callback_mutex       = SDL_CreateMutex();
  callback_queue_mutex = SDL_CreateMutex();

  // Set postmix that adds the OPL music. This is deliberately done
  // as a postmix and not using Mix_HookMusic() as the latter disables
  // normal SDL_mixer music mixing.
  Mix_SetPostMix(OPL_Mix_Callback, nullptr);

  return 1;
}

static unsigned int OPL_SDL_PortRead(opl_port_t port) {
  unsigned int result = 0;

  if (port == OPL_REGISTER_PORT_OPL3) {
    return 0xff;
  }

  if (timer1.enabled && current_time > timer1.expire_time) {
    result |= 0x80; // Either have expired
    result |= 0x40; // Timer 1 has expired
  }

  if (timer2.enabled && current_time > timer2.expire_time) {
    result |= 0x80; // Either have expired
    result |= 0x20; // Timer 2 has expired
  }

  return result;
}

static void OPLTimer_CalculateEndTime(opl_timer_t * timer) {
  // If the timer is enabled, calculate the time when the timer
  // will expire.

  if (timer->enabled) {
    int tics           = static_cast<int>(0x100 - timer->value);
    timer->expire_time = current_time
                         + (static_cast<uint64_t>(static_cast<unsigned long long int>(tics) * OPL_SECOND)) / timer->rate;
  }
}

static void WriteRegister(unsigned int reg_num, unsigned int value) {
  switch (reg_num) {
  case OPL_REG_TIMER1:
    timer1.value = value;
    OPLTimer_CalculateEndTime(&timer1);
    break;

  case OPL_REG_TIMER2:
    timer2.value = value;
    OPLTimer_CalculateEndTime(&timer2);
    break;

  case OPL_REG_TIMER_CTRL:
    if (value & 0x80) {
      timer1.enabled = 0;
      timer2.enabled = 0;
    } else {
      if ((value & 0x40) == 0) {
        timer1.enabled = (value & 0x01) != 0;
        OPLTimer_CalculateEndTime(&timer1);
      }

      if ((value & 0x20) == 0) {
        timer1.enabled = (value & 0x02) != 0;
        OPLTimer_CalculateEndTime(&timer2);
      }
    }

    break;

  case OPL_REG_NEW:
    opl_opl3mode = value & 0x01;
    [[fallthrough]];
  default:
    OPL3_WriteRegBuffered(&opl_chip, static_cast<Bit16u>(reg_num), static_cast<Bit8u>(value));
    break;
  }
}

static void OPL_SDL_PortWrite(opl_port_t port, unsigned int value) {
  if (port == OPL_REGISTER_PORT) {
    register_num = static_cast<int>(value);
  } else if (port == OPL_REGISTER_PORT_OPL3) {
    register_num = static_cast<int>(value | 0x100);
  } else if (port == OPL_DATA_PORT) {
    WriteRegister(static_cast<unsigned int>(register_num), value);
  }
}

static void OPL_SDL_SetCallback(uint64_t us, opl_callback_t callback, void * data) {
  SDL_LockMutex(callback_queue_mutex);
  OPL_Queue_Push(callback_queue, callback, data, current_time - pause_offset + us);
  SDL_UnlockMutex(callback_queue_mutex);
}

static void OPL_SDL_ClearCallbacks() {
  SDL_LockMutex(callback_queue_mutex);
  OPL_Queue_Clear(callback_queue);
  SDL_UnlockMutex(callback_queue_mutex);
}

static void OPL_SDL_Lock() {
  SDL_LockMutex(callback_mutex);
}

static void OPL_SDL_Unlock() {
  SDL_UnlockMutex(callback_mutex);
}

static void OPL_SDL_SetPaused(int paused) {
  opl_sdl_paused = paused;
}

static void OPL_SDL_AdjustCallbacks(float factor) {
  SDL_LockMutex(callback_queue_mutex);
  OPL_Queue_AdjustCallbacks(callback_queue, current_time, factor);
  SDL_UnlockMutex(callback_queue_mutex);
}

opl_driver_t opl_sdl_driver = {
  "SDL",
  OPL_SDL_Init,
  OPL_SDL_Shutdown,
  OPL_SDL_PortRead,
  OPL_SDL_PortWrite,
  OPL_SDL_SetCallback,
  OPL_SDL_ClearCallbacks,
  OPL_SDL_Lock,
  OPL_SDL_Unlock,
  OPL_SDL_SetPaused,
  OPL_SDL_AdjustCallbacks,
};
