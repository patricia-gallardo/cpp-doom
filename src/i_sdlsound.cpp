//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2008 David Flater
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
//	System interface for sound.
//

#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <fmt/printf.h>

#include <SDL.h>
#include <SDL_mixer.h>

#ifdef HAVE_LIBSAMPLERATE
#include <samplerate.h>
#endif

#include "deh_str.hpp"
#include "i_sound.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "doomtype.hpp"
#include "lump.hpp"

#define LOW_PASS_FILTER
//#define DEBUG_DUMP_WAVS
constexpr auto NUM_CHANNELS = 16 * 2; // [crispy] support up to 32 sound channels

using allocated_sound_t = struct allocated_sound_s;

struct allocated_sound_s {
  sfxinfo_t *        sfxinfo;
  Mix_Chunk          chunk;
  int                use_count;
  int                pitch;
  allocated_sound_t *prev, *next;
};

static bool sound_initialized = false;

static std::array<allocated_sound_t *, NUM_CHANNELS> channels_playing;

static int    mixer_freq;
static Uint16 mixer_format;
static int    mixer_channels;
static bool   use_sfx_prefix;
static bool (*ExpandSoundData)(sfxinfo_t * sfxinfo, uint8_t * data, int samplerate, int bits, int length) = nullptr;

// Doubly-linked list of allocated sounds.
// When a sound is played, it is moved to the head, so that the oldest
// sounds not used recently are at the tail.

static allocated_sound_t * allocated_sounds_head = nullptr;
static allocated_sound_t * allocated_sounds_tail = nullptr;
static int                 allocated_sounds_size = 0;

// [crispy] values 3 and higher might reproduce DOOM.EXE more accurately,
// but 1 is closer to "use_libsamplerate = 0" which is the default in Choco
// and causes only a short delay at startup
int use_libsamplerate = 1;

// Scale factor used when converting libsamplerate floating point numbers
// to integers. Too high means the sounds can clip; too low means they
// will be too quiet. This is an amount that should avoid clipping most
// of the time: with all the Doom IWAD sound effects, at least. If a PWAD
// is used, clipping might occur.

float libsamplerate_scale = 0.65f;

// Hook a sound into the linked list at the head.

static void AllocatedSoundLink(allocated_sound_t * snd) {
  snd->prev = nullptr;

  snd->next             = allocated_sounds_head;
  allocated_sounds_head = snd;

  if (allocated_sounds_tail == nullptr) {
    allocated_sounds_tail = snd;
  } else {
    snd->next->prev = snd;
  }
}

// Unlink a sound from the linked list.

static void AllocatedSoundUnlink(allocated_sound_t * snd) {
  if (snd->prev == nullptr) {
    allocated_sounds_head = snd->next;
  } else {
    snd->prev->next = snd->next;
  }

  if (snd->next == nullptr) {
    allocated_sounds_tail = snd->prev;
  } else {
    snd->next->prev = snd->prev;
  }
}

static void FreeAllocatedSound(allocated_sound_t * snd) {
  // Unlink from linked list.

  AllocatedSoundUnlink(snd);

  // Keep track of the amount of allocated sound data:

  allocated_sounds_size -= snd->chunk.alen;

  free(snd);
}

// Search from the tail backwards along the allocated sounds list, find
// and free a sound that is not in use, to free up memory.  Return true
// for success.

static bool FindAndFreeSound() {
  allocated_sound_t * snd = allocated_sounds_tail;

  while (snd != nullptr) {
    if (snd->use_count == 0) {
      FreeAllocatedSound(snd);
      return true;
    }

    snd = snd->prev;
  }

  // No available sounds to free...

  return false;
}

// Enforce SFX cache size limit.  We are just about to allocate "len"
// bytes on the heap for a new sound effect, so free up some space
// so that we keep allocated_sounds_size < snd_cachesize

static void ReserveCacheSpace(size_t len) {
  if (g_i_sound_globals->snd_cachesize <= 0) {
    return;
  }

  // Keep freeing sound effects that aren't currently being played,
  // until there is enough space for the new sound.

  while (allocated_sounds_size + len > static_cast<size_t>(g_i_sound_globals->snd_cachesize)) {
    // Free a sound.  If there is nothing more to free, stop.

    if (!FindAndFreeSound()) {
      break;
    }
  }
}

// Allocate a block for a new sound effect.

static allocated_sound_t * AllocateSound(sfxinfo_t * sfxinfo, size_t len) {
  allocated_sound_t * snd = nullptr;

  // Keep allocated sounds within the cache size.

  ReserveCacheSpace(len);

  // Allocate the sound structure and data.  The data will immediately
  // follow the structure, which acts as a header.

  do {
    snd = static_cast<allocated_sound_t *>(malloc(sizeof(allocated_sound_t) + len));

    // Out of memory?  Try to free an old sound, then loop round
    // and try again.

    if (snd == nullptr && !FindAndFreeSound()) {
      return nullptr;
    }

  } while (snd == nullptr);

  // Skip past the chunk structure for the audio buffer

  snd->chunk.abuf      = reinterpret_cast<uint8_t *>(snd + 1);
  snd->chunk.alen      = static_cast<Uint32>(len);
  snd->chunk.allocated = 1;
  snd->chunk.volume    = MIX_MAX_VOLUME;
  snd->pitch           = NORM_PITCH;

  snd->sfxinfo   = sfxinfo;
  snd->use_count = 0;

  // Keep track of how much memory all these cached sounds are using...

  allocated_sounds_size += static_cast<int>(len);

  AllocatedSoundLink(snd);

  return snd;
}

// Lock a sound, to indicate that it may not be freed.

static void LockAllocatedSound(allocated_sound_t * snd) {
  if (!snd)
    return;
  // Increase use count, to stop the sound being freed.

  ++snd->use_count;

  // printf("++ %s: Use count=%i\n", snd->sfxinfo->name, snd->use_count);

  // When we use a sound, re-link it into the list at the head, so
  // that the oldest sounds fall to the end of the list for freeing.

  AllocatedSoundUnlink(snd);
  AllocatedSoundLink(snd);
}

// Unlock a sound to indicate that it may now be freed.

static void UnlockAllocatedSound(allocated_sound_t * snd) {
  if (snd->use_count <= 0) {
    I_Error("Sound effect released more times than it was locked...");
  }

  --snd->use_count;

  // printf("-- %s: Use count=%i\n", snd->sfxinfo->name, snd->use_count);
}

// Search through the list of allocated sounds and return the one that matches
// the supplied sfxinfo entry and pitch level.

static allocated_sound_t * GetAllocatedSoundBySfxInfoAndPitch(sfxinfo_t * sfxinfo, int pitch) {
  allocated_sound_t * p = allocated_sounds_head;

  while (p != nullptr) {
    if (p->sfxinfo == sfxinfo && p->pitch == pitch) {
      return p;
    }
    p = p->next;
  }

  return nullptr;
}

// Allocate a new sound chunk and pitch-shift an existing sound up-or-down
// into it.

static allocated_sound_t * PitchShift(allocated_sound_t * insnd, int pitch) {
  auto * srcbuf = reinterpret_cast<Sint16 *>(insnd->chunk.abuf);
  Uint32 srclen = insnd->chunk.alen;

  // determine ratio pitch:NORM_PITCH and apply to srclen, then invert.
  // This is an approximation of vanilla behaviour based on measurements
  Uint32 dstlen = static_cast<Uint32>((1 + (1 - static_cast<float>(pitch) / NORM_PITCH)) * static_cast<float>(srclen));

  // ensure that the new buffer is an even length
  if ((dstlen % 2) == 0) {
    dstlen++;
  }

  allocated_sound_t * outsnd = AllocateSound(insnd->sfxinfo, dstlen);

  if (!outsnd) {
    return nullptr;
  }

  outsnd->pitch = pitch;
  auto * dstbuf = reinterpret_cast<Sint16 *>(outsnd->chunk.abuf);

  // loop over output buffer. find corresponding input cell, copy over
  for (Sint16 * outp = dstbuf; outp < dstbuf + dstlen / 2; ++outp) {
    Sint16 * inp = srcbuf + static_cast<int>(static_cast<float>(outp - dstbuf) / static_cast<float>(dstlen) * static_cast<float>(srclen));
    *outp        = *inp;
  }

  return outsnd;
}

// When a sound stops, check if it is still playing.  If it is not,
// we can mark the sound data as CACHE to be freed back for other
// means.

static void ReleaseSoundOnChannel(int channel) {
  allocated_sound_t * snd = channels_playing[channel];

  Mix_HaltChannel(channel);

  if (snd == nullptr) {
    return;
  }

  channels_playing[channel] = nullptr;

  UnlockAllocatedSound(snd);

  // if the sound is a pitch-shift and it's not in use, immediately
  // free it
  if (snd->pitch != NORM_PITCH && snd->use_count <= 0) {
    FreeAllocatedSound(snd);
  }
}

#ifdef HAVE_LIBSAMPLERATE

// Returns the conversion mode for libsamplerate to use.

static int SRC_ConversionMode() {
  switch (use_libsamplerate) {
    // 0 = disabled

  default:
  case 0:
    return -1;

    // Ascending numbers give higher quality

  case 1:
    return SRC_LINEAR;
  case 2:
    return SRC_ZERO_ORDER_HOLD;
  case 3:
    return SRC_SINC_FASTEST;
  case 4:
    return SRC_SINC_MEDIUM_QUALITY;
  case 5:
    return SRC_SINC_BEST_QUALITY;
  }
}

// libsamplerate-based generic sound expansion function for any sample rate
//   unsigned 8 bits --> signed 16 bits
//   mono --> stereo
//   samplerate --> mixer_freq
// Returns number of clipped samples.
// DWF 2008-02-10 with cleanups by Simon Howard.

static bool ExpandSoundData_SRC(sfxinfo_t * sfxinfo,
                                uint8_t *      data,
                                int         samplerate,
                                int         bits,
                                int         length) {
  SRC_DATA src_data;
  uint32_t abuf_index = 0, clipped = 0;
  //    uint32_t alen;
  int                 retn;
  int16_t *           expanded;
  allocated_sound_t * snd;
  Mix_Chunk *         chunk;
  uint32_t            samplecount = length / (bits / 8);

  src_data.input_frames = samplecount;
  std::vector<float> data_in(samplecount);
  src_data.data_in   = data_in.data();
  src_data.src_ratio = static_cast<double>(mixer_freq) / samplerate;

  // We include some extra space here in case of rounding-up.
  src_data.output_frames = src_data.src_ratio * samplecount + (mixer_freq / 4);
  std::vector<float> data_out(src_data.output_frames);
  src_data.data_out = data_out.data();

  assert(src_data.data_in != nullptr && src_data.data_out != nullptr);

  // Convert input data to floats
  // [crispy] Handle 16 bit audio data
  if (bits == 16) {
    for (uint32_t i = 0; i < samplecount; ++i) {
      // Code below uses 32767, so use it here too and trust it to clip.
      data_in[i] = static_cast<float>(static_cast<int16_t>(data[i * 2] | (data[i * 2 + 1] << 8)) / 32767.0);
    }
  } else {
    for (int i = 0; i < length; ++i) {
      // Unclear whether 128 should be interpreted as "zero" or whether a
      // symmetrical range should be assumed.  The following assumes a
      // symmetrical range.
      data_in[i] = static_cast<float>(data[i] / 127.5 - 1);
    }
  }

  // Do the sound conversion

  retn = src_simple(&src_data, SRC_ConversionMode(), 1);
  if (retn != 0) {
    fmt::fprintf(stderr, "SampleRate src_simple returned non zero value %d\n", retn);
    assert(retn == 0);
  }

  // Allocate the new chunk.

  //    alen = src_data.output_frames_gen * 4;

  snd = AllocateSound(sfxinfo, src_data.output_frames_gen * 4);

  if (snd == nullptr) {
    return false;
  }

  chunk    = &snd->chunk;
  expanded = reinterpret_cast<int16_t *>(chunk->abuf);

  // Convert the result back into 16-bit integers.

  for (long i = 0; i < src_data.output_frames_gen; ++i) {
    // libsamplerate does not limit itself to the -1.0 .. 1.0 range on
    // output, so a multiplier less than INT16_MAX (32767) is required
    // to avoid overflows or clipping.  However, the smaller the
    // multiplier, the quieter the sound effects get, and the more you
    // have to turn down the music to keep it in balance.

    // 22265 is the largest multiplier that can be used to resample all
    // of the Vanilla DOOM sound effects to 48 kHz without clipping
    // using SRC_SINC_BEST_QUALITY.  It is close enough (only slightly
    // too conservative) for SRC_SINC_MEDIUM_QUALITY and
    // SRC_SINC_FASTEST.  PWADs with interestingly different sound
    // effects or target rates other than 48 kHz might still result in
    // clipping--I don't know if there's a limit to it.

    // As the number of clipped samples increases, the signal is
    // gradually overtaken by noise, with the loudest parts going first.
    // However, a moderate amount of clipping is often tolerated in the
    // quest for the loudest possible sound overall.  The results of
    // using INT16_MAX as the multiplier are not all that bad, but
    // artifacts are noticeable during the loudest parts.

    float cvtval_f =
        src_data.data_out[i] * libsamplerate_scale * INT16_MAX;
    int32_t cvtval_i = cvtval_f + (cvtval_f < 0 ? -0.5 : 0.5);

    // Asymmetrical sound worries me, so we won't use -32768.
    if (cvtval_i < -INT16_MAX) {
      cvtval_i = -INT16_MAX;
      ++clipped;
    } else if (cvtval_i > INT16_MAX) {
      cvtval_i = INT16_MAX;
      ++clipped;
    }

    // Left and right channels

    expanded[abuf_index++] = static_cast<int16_t>(cvtval_i);
    expanded[abuf_index++] = static_cast<int16_t>(cvtval_i);
  }

  if (clipped > 0) {
    fmt::fprintf(stderr, "Sound '%s': clipped %u samples (%0.2f %%)\n", sfxinfo->name, clipped, 400.0 * clipped / chunk->alen);
  }

  return true;
}

#endif

static bool ConvertibleRatio(int freq1, int freq2) {
  if (freq1 > freq2) {
    return ConvertibleRatio(freq2, freq1);
  } else if ((freq2 % freq1) != 0) {
    // Not in a direct ratio

    return false;
  } else {
    // Check the ratio is a power of 2

    int ratio = freq2 / freq1;

    while ((ratio & 1) == 0) {
      ratio = ratio >> 1;
    }

    return ratio == 1;
  }
}

#ifdef DEBUG_DUMP_WAVS

// Debug code to dump resampled sound effects to WAV files for analysis.

static void WriteWAV(char * filename, byte * data, uint32_t length, int samplerate) {
  FILE *         wav;
  unsigned int   i;
  unsigned short s;

  wav = fopen(filename, "wb");

  // Header

  fwrite("RIFF", 1, 4, wav);
  i = LONG(36 + samplerate);
  fwrite(&i, 4, 1, wav);
  fwrite("WAVE", 1, 4, wav);

  // Subchunk 1

  fwrite("fmt ", 1, 4, wav);
  i = LONG(16);
  fwrite(&i, 4, 1, wav); // Length
  s = SHORT(1);
  fwrite(&s, 2, 1, wav); // Format (PCM)
  s = SHORT(2);
  fwrite(&s, 2, 1, wav); // Channels (2=stereo)
  i = LONG(samplerate);
  fwrite(&i, 4, 1, wav); // Sample rate
  i = LONG(samplerate * 2 * 2);
  fwrite(&i, 4, 1, wav); // Byte rate (samplerate * stereo * 16 bit)
  s = SHORT(2 * 2);
  fwrite(&s, 2, 1, wav); // Block align (stereo * 16 bit)
  s = SHORT(16);
  fwrite(&s, 2, 1, wav); // Bits per sample (16 bit)

  // Data subchunk

  fwrite("data", 1, 4, wav);
  i = LONG(length);
  fwrite(&i, 4, 1, wav);        // Data length
  fwrite(data, 1, length, wav); // Data

  fclose(wav);
}

#endif

// Generic sound expansion function for any sample rate.
// Returns number of clipped samples (always 0).

static bool ExpandSoundData_SDL(sfxinfo_t * sfxinfo, uint8_t * data, int samplerate, int bits, int length) {
  uint32_t samplecount = length / (bits / 8);

  // Calculate the length of the expanded version of the sample.

  auto expanded_length = static_cast<uint32_t>((static_cast<uint64_t>(samplecount) * mixer_freq) / samplerate);

  // Double up twice: 8 -> 16 bit and mono -> stereo

  expanded_length *= 4;

  // Allocate a chunk in which to expand the sound

  allocated_sound_t * snd = AllocateSound(sfxinfo, expanded_length);

  if (snd == nullptr) {
    return false;
  }

  Mix_Chunk * chunk = &snd->chunk;

  // If we can, use the standard / optimized SDL conversion routines.
  SDL_AudioCVT convertor;

  if (samplerate <= mixer_freq
      && ConvertibleRatio(samplerate, mixer_freq)
      && SDL_BuildAudioCVT(&convertor,
                           bits == 16 ? AUDIO_S16 : AUDIO_U8,
                           1,
                           samplerate,
                           mixer_format,
                           static_cast<Uint8>(mixer_channels),
                           mixer_freq)) {
    convertor.len = length;
    convertor.buf = static_cast<Uint8 *>(malloc(convertor.len * convertor.len_mult));
    assert(convertor.buf != nullptr);
    std::memcpy(convertor.buf, data, length);

    SDL_ConvertAudio(&convertor);

    std::memcpy(chunk->abuf, convertor.buf, chunk->alen);
    free(convertor.buf);
  } else {
    auto * expanded = reinterpret_cast<Sint16 *>(chunk->abuf);

    // Generic expansion if conversion does not work:
    //
    // SDL's audio conversion only works for rate conversions that are
    // powers of 2; if the two formats are not in a direct power of 2
    // ratio, do this naive conversion instead.

    // number of samples in the converted sound

    int expanded_length_local = static_cast<int>((static_cast<uint64_t>(samplecount) * mixer_freq) / samplerate);
    int expand_ratio          = (samplecount << 8) / expanded_length_local;

    for (int i = 0; i < expanded_length_local; ++i) {
      Sint16 sample = 0;
      int    src    = (i * expand_ratio) >> 8;

      // [crispy] Handle 16 bit audio data
      if (bits == 16) {
        sample = static_cast<Sint16>(data[src * 2] | (data[src * 2 + 1] << 8));
      } else {
        sample = static_cast<Sint16>(data[src] | (data[src] << 8));
        sample = static_cast<Sint16>(sample - 32768);
      }

      // expand mono->stereo

      expanded[i * 2] = expanded[i * 2 + 1] = sample;
    }

#ifdef LOW_PASS_FILTER
    // Perform a low-pass filter on the upscaled sound to filter
    // out high-frequency noise from the conversion process.

    {
      // Low-pass filter for cutoff frequency f:
      //
      // For sampling rate r, dt = 1 / r
      // rc = 1 / 2*pi*f
      // alpha = dt / (rc + dt)

      // Filter to the half sample rate of the original sound effect
      // (maximum frequency, by nyquist)

      float dt    = 1.0f / static_cast<float>(mixer_freq);
      float rc    = 1.0f / (3.14f * static_cast<float>(samplerate));
      float alpha = dt / (rc + dt);

      // Both channels are processed in parallel, hence [i-2]:

      for (int i = 2; i < expanded_length_local * 2; ++i) {
        expanded[i] = static_cast<Sint16>(alpha * expanded[i]
                                          + (1 - alpha) * expanded[i - 2]);
      }
    }
#endif /* #ifdef LOW_PASS_FILTER */
  }

  return true;
}

// Load and convert a sound effect
// Returns true if successful

static bool CacheSFX(sfxinfo_t * sfxinfo) {
  int          samplerate = 0;
  unsigned int bits       = 0;
  unsigned int length     = 0;

  // need to load the sound

  int    lumpnum = sfxinfo->lumpnum;
  auto * data    = cache_lump_num<uint8_t *>(lumpnum, PU_STATIC);
  size_t lumplen = W_LumpLength(lumpnum);

  // [crispy] Check if this is a valid RIFF wav file
  if (lumplen > 44 && memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "WAVEfmt ", 8) == 0) {
    // Valid RIFF wav file
    // Make sure this is a PCM format file
    // "fmt " chunk size must == 16
    int check = data[16] | (data[17] << 8) | (data[18] << 16) | (data[19] << 24);
    if (check != 16)
      return false;

    // Format must == 1 (PCM)
    check = data[20] | (data[21] << 8);
    if (check != 1)
      return false;

    // FIXME: can't handle stereo wavs
    // Number of channels must == 1
    check = data[22] | (data[23] << 8);
    if (check != 1)
      return false;

    samplerate = data[24] | (data[25] << 8) | (data[26] << 16) | (data[27] << 24);
    length     = data[40] | (data[41] << 8) | (data[42] << 16) | (data[43] << 24);

    if (length > lumplen - 44)
      length = static_cast<unsigned int>(lumplen - 44);

    bits = data[34] | (data[35] << 8);

    // Reject non 8 or 16 bit
    if (bits != 16 && bits != 8)
      return false;

    data += 44 - 8;
  }
  // Check the header, and ensure this is a valid sound
  else if (lumplen >= 8 && data[0] == 0x03 && data[1] == 00) {
    // Valid DOOM sound

    // 16 bit sample rate field, 32 bit length field
    samplerate = (data[3] << 8) | data[2];
    length     = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];

    // If the header specifies that the length of the sound is greater than
    // the length of the lump itself, this is an invalid sound lump

    // We also discard sound lumps that are less than 49 samples long,
    // as this is how DMX behaves - although the actual cut-off length
    // seems to vary slightly depending on the sample rate.  This needs
    // further investigation to better understand the correct
    // behavior.

    if (length > lumplen - 8 || length <= 48) {
      return false;
    }

    // All Doom sounds are 8-bit
    bits = 8;

    // The DMX sound library seems to skip the first 16 and last 16
    // bytes of the lump - reason unknown.

    data += 16;
    length -= 32;
  } else {
    // Invalid sound
    return false;
  }

  // Sample rate conversion

  if (!ExpandSoundData(sfxinfo, data + 8, samplerate, bits, length)) {
    return false;
  }

#ifdef DEBUG_DUMP_WAVS
  {
    char                filename[16];
    allocated_sound_t * snd;

    M_snprintf(filename, sizeof(filename), "%s.wav", DEH_String(sfxinfo->name));
    snd = GetAllocatedSoundBySfxInfoAndPitch(sfxinfo, NORM_PITCH);
    WriteWAV(filename, snd->chunk.abuf, snd->chunk.alen, mixer_freq);
  }
#endif

  // don't need the original lump any more

  W_ReleaseLumpNum(lumpnum);

  return true;
}

static std::string GetSfxLumpName(sfxinfo_t * sfx) {
  // Linked sfx lumps? Get the lump number for the sound linked to.

  if (sfx->link != nullptr) {
    sfx = sfx->link;
  }

  // Doom adds a DS* prefix to sound lumps; Heretic and Hexen don't
  // do this.

  std::string namebuf;

  if (use_sfx_prefix) {
    namebuf = "ds";
  }

  namebuf.append(DEH_String(sfx->name.c_str()));

  return namebuf;
}

#ifdef HAVE_LIBSAMPLERATE

// Preload all the sound effects - stops nasty ingame freezes

static void I_SDL_PrecacheSounds(sfxinfo_t * sounds, int num_sounds) {
  // Don't need to precache the sounds unless we are using libsamplerate.

  if (use_libsamplerate == 0) {
    return;
  }

  fmt::printf("I_SDL_PrecacheSounds: Precaching all sound effects..");

  for (int i = 0; i < num_sounds; ++i) {
    if ((i % 6) == 0) {
      fmt::printf(".");
      fflush(stdout);
    }

    std::string namebuf = GetSfxLumpName(&sounds[i]);

    sounds[i].lumpnum = W_CheckNumForName(namebuf.c_str());

    if (sounds[i].lumpnum != -1) {
      CacheSFX(&sounds[i]);
    }
  }

  fmt::printf("\n");
}

#else

static void I_SDL_PrecacheSounds(sfxinfo_t *, int) {
  // no-op
}

#endif

// Load a SFX chunk into memory and ensure that it is locked.

static bool LockSound(sfxinfo_t * sfxinfo) {
  // If the sound isn't loaded, load it now
  if (GetAllocatedSoundBySfxInfoAndPitch(sfxinfo, NORM_PITCH) == nullptr) {
    if (!CacheSFX(sfxinfo)) {
      return false;
    }
  }

  LockAllocatedSound(GetAllocatedSoundBySfxInfoAndPitch(sfxinfo, NORM_PITCH));

  return true;
}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//

static int I_SDL_GetSfxLumpNum(sfxinfo_t * sfx) {
  std::string namebuf = GetSfxLumpName(sfx);

  // [crispy] make missing sounds non-fatal
  return W_CheckNumForName(namebuf.c_str());
}

static void I_SDL_UpdateSoundParams(int handle, int vol, int sep) {
  if (!sound_initialized || handle < 0 || handle >= NUM_CHANNELS) {
    return;
  }

  int left  = ((254 - sep) * vol) / 127;
  int right = ((sep)*vol) / 127;

  if (left < 0)
    left = 0;
  else if (left > 255)
    left = 255;
  if (right < 0)
    right = 0;
  else if (right > 255)
    right = 255;

  Mix_SetPanning(handle, static_cast<Uint8>(left), static_cast<Uint8>(right));
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//

static int I_SDL_StartSound(sfxinfo_t * sfxinfo, int channel, int vol, int sep, int pitch) {
  if (!sound_initialized || channel < 0 || channel >= NUM_CHANNELS) {
    return -1;
  }

  // Release a sound effect if there is already one playing
  // on this channel

  ReleaseSoundOnChannel(channel);

  // Get the sound data

  if (!LockSound(sfxinfo)) {
    return -1;
  }

  allocated_sound_t * snd = GetAllocatedSoundBySfxInfoAndPitch(sfxinfo, pitch);

  if (snd == nullptr) {
    // fetch the base sound effect, un-pitch-shifted
    snd = GetAllocatedSoundBySfxInfoAndPitch(sfxinfo, NORM_PITCH);

    if (snd == nullptr) {
      return -1;
    }

    if (g_i_sound_globals->snd_pitchshift) {
      allocated_sound_t * newsnd = PitchShift(snd, pitch);

      if (newsnd) {
        LockAllocatedSound(newsnd);
        UnlockAllocatedSound(snd);
        snd = newsnd;
      }
    }
  } else {
    LockAllocatedSound(snd);
  }

  // play sound

  Mix_PlayChannel(channel, &snd->chunk, 0);

  channels_playing[channel] = snd;

  // set separation, etc.

  I_SDL_UpdateSoundParams(channel, vol, sep);

  return channel;
}

static void I_SDL_StopSound(int handle) {
  if (!sound_initialized || handle < 0 || handle >= NUM_CHANNELS) {
    return;
  }

  // Sound data is no longer needed; release the
  // sound data being used for this channel

  ReleaseSoundOnChannel(handle);
}

static bool I_SDL_SoundIsPlaying(int handle) {
  if (!sound_initialized || handle < 0 || handle >= NUM_CHANNELS) {
    return false;
  }

  return Mix_Playing(handle);
}

//
// Periodically called to update the sound system
//

static void I_SDL_UpdateSound() {
  // Check all channels to see if a sound has finished

  for (int i = 0; i < NUM_CHANNELS; ++i) {
    if (channels_playing[i] && !I_SDL_SoundIsPlaying(i)) {
      // Sound has finished playing on this channel,
      // but sound data has not been released to cache

      ReleaseSoundOnChannel(i);
    }
  }
}

static void I_SDL_ShutdownSound() {
  if (!sound_initialized) {
    return;
  }

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);

  sound_initialized = false;
}

// Calculate slice size, based on snd_maxslicetime_ms.
// The result must be a power of two.

static int GetSliceSize() {
  int limit = (g_i_sound_globals->snd_samplerate * g_i_sound_globals->snd_maxslicetime_ms) / 1000;

  // Try all powers of two, not exceeding the limit.

  for (int n = 0;; ++n) {
    // 2^n <= limit < 2^n+1 ?

    if ((1 << (n + 1)) > limit) {
      return (1 << n);
    }
  }
  [[unreachable]];
}

static bool I_SDL_InitSound(bool _use_sfx_prefix) {
  // SDL 2.0.6 has a bug that makes it unusable.
  if constexpr (SDL_COMPILEDVERSION == SDL_VERSIONNUM(2, 0, 6)) {
    I_Error(
        "I_SDL_InitSound: "
        "You are trying to launch with SDL 2.0.6 which has a known bug "
        "that makes the game crash. Please either downgrade to "
        "SDL 2.0.5 or upgrade to 2.0.7. See the following bug for some "
        "additional context:\n"
        "<https://github.com/chocolate-doom/chocolate-doom/issues/945>");
  }

  use_sfx_prefix = _use_sfx_prefix;

  // No sounds yet
  for (int i = 0; i < NUM_CHANNELS; ++i) {
    channels_playing[i] = nullptr;
  }

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    fmt::fprintf(stderr, "Unable to set up sound.\n");
    return false;
  }

  if (Mix_OpenAudio(g_i_sound_globals->snd_samplerate, AUDIO_S16SYS, 2, GetSliceSize()) < 0) {
    fmt::fprintf(stderr, "Error initialising SDL_mixer: %s\n", Mix_GetError());
    return false;
  }

  ExpandSoundData = ExpandSoundData_SDL;

  Mix_QuerySpec(&mixer_freq, &mixer_format, &mixer_channels);

#ifdef HAVE_LIBSAMPLERATE
  if (use_libsamplerate != 0) {
    if (SRC_ConversionMode() < 0) {
      I_Error("I_SDL_InitSound: Invalid value for use_libsamplerate: %i",
              use_libsamplerate);
    }

    ExpandSoundData = ExpandSoundData_SRC;
  }
#else
  if (use_libsamplerate != 0) {
    fmt::fprintf(stderr, "I_SDL_InitSound: use_libsamplerate=%i, but "
                         "libsamplerate support not compiled in.\n",
                 use_libsamplerate);
  }
#endif

  Mix_AllocateChannels(NUM_CHANNELS);

  SDL_PauseAudio(0);

  sound_initialized = true;

  return true;
}

static std::array sound_sdl_devices = {
  SNDDEVICE_SB,
  SNDDEVICE_PAS,
  SNDDEVICE_GUS,
  SNDDEVICE_WAVEBLASTER,
  SNDDEVICE_SOUNDCANVAS,
  SNDDEVICE_AWE32,
};

sound_module_t sound_sdl_module = {
  sound_sdl_devices.data(),
  static_cast<int>(std::size(sound_sdl_devices)),
  I_SDL_InitSound,
  I_SDL_ShutdownSound,
  I_SDL_GetSfxLumpNum,
  I_SDL_UpdateSound,
  I_SDL_UpdateSoundParams,
  I_SDL_StartSound,
  I_SDL_StopSound,
  I_SDL_SoundIsPlaying,
  I_SDL_PrecacheSounds,
};
