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
//     SDL emulation of VGA 640x480x4 planar video mode,
//     for Hexen startup loading screen.
//

#include "SDL.h"
#include <cstring>

#include "doomtype.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"

// Palette fade-in takes two seconds

constexpr auto FADE_TIME = 2000;

constexpr auto HR_SCREENWIDTH  = 640;
constexpr auto HR_SCREENHEIGHT = 480;

static SDL_Window  *hr_screen    = nullptr;
static SDL_Surface *hr_surface   = nullptr;
static const char  *window_title = "";

[[maybe_unused]] bool I_SetVideoModeHR() {
  int x = 0, y = 0;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return false;
  }

  I_GetWindowPosition(&x, &y, HR_SCREENWIDTH, HR_SCREENHEIGHT);

  // Create screen surface at the native desktop pixel depth (bpp=0),
  // as we cannot trust true 8-bit to reliably work nowadays.
  hr_screen = SDL_CreateWindow(window_title, x, y, HR_SCREENWIDTH, HR_SCREENHEIGHT, 0);

  if (hr_screen == nullptr) {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    return false;
  }

  // We do all actual drawing into an intermediate surface.
  hr_surface = SDL_CreateRGBSurface(0, HR_SCREENWIDTH, HR_SCREENHEIGHT, 8, 0, 0, 0, 0);

  return true;
}

[[maybe_unused]] void I_SetWindowTitleHR(const char *title) {
  window_title = title;
}

[[maybe_unused]] void I_UnsetVideoModeHR() {
  if (hr_screen != nullptr) {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    hr_screen = nullptr;
    SDL_FreeSurface(hr_surface);
    hr_surface = nullptr;
  }
}

[[maybe_unused]] void I_ClearScreenHR() {
  SDL_Rect area = { 0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT };

  SDL_FillRect(hr_surface, &area, 0);
}

void I_SlamBlockHR(int x, int y, int w, int h, const uint8_t *src) {
  SDL_Rect       blit_rect;
  const uint8_t *srcptrs[4];
  uint8_t        srcbits[4];
  int            bit;

  // Set up source pointers to read from source buffer - each 4-bit
  // pixel has its bits split into four sub-buffers

  for (int i = 0; i < 4; ++i) {
    srcptrs[i] = src + (i * w * h / 8);
  }

  if (SDL_LockSurface(hr_surface) < 0) {
    return;
  }

  // Draw each pixel

  bit = 0;

  for (int y1 = y; y1 < y + h; ++y1) {
    uint8_t *dest = (reinterpret_cast<uint8_t *>(hr_surface->pixels)) + y1 * hr_surface->pitch + x;

    for (int x1 = x; x1 < x + w; ++x1) {
      // Get the bits for this pixel
      // For each bit, find the byte containing it, shift down
      // and mask out the specific bit wanted.

      for (int i = 0; i < 4; ++i) {
        srcbits[i] = (srcptrs[i][bit / 8] >> (7 - (bit % 8))) & 0x1;
      }

      // Reassemble the pixel value

      *dest = static_cast<uint8_t>((srcbits[0] << 0) | (srcbits[1] << 1) | (srcbits[2] << 2) | (srcbits[3] << 3));

      // Next pixel!

      ++dest;
      ++bit;
    }
  }

  SDL_UnlockSurface(hr_surface);

  // Update the region we drew.
  blit_rect.x = x;
  blit_rect.y = y;
  blit_rect.w = w;
  blit_rect.h = h;
  SDL_BlitSurface(hr_surface, &blit_rect, SDL_GetWindowSurface(hr_screen), &blit_rect);
  SDL_UpdateWindowSurfaceRects(hr_screen, &blit_rect, 1);
}

[[maybe_unused]] void I_SlamHR(const uint8_t *buffer) {
  I_SlamBlockHR(0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT, buffer);
}

[[maybe_unused]] void I_InitPaletteHR() {
  // ...
}

void I_SetPaletteHR(const uint8_t *palette) {
  SDL_Rect  screen_rect = { 0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT };
  SDL_Color sdlpal[16];

  for (int i = 0; i < 16; ++i) {
    sdlpal[i].r = static_cast<Uint8>(palette[i * 3 + 0] * 4);
    sdlpal[i].g = static_cast<Uint8>(palette[i * 3 + 1] * 4);
    sdlpal[i].b = static_cast<Uint8>(palette[i * 3 + 2] * 4);
  }

  // After setting colors, update the screen.
  SDL_SetPaletteColors(hr_surface->format->palette, sdlpal, 0, 16);
  SDL_BlitSurface(hr_surface, &screen_rect, SDL_GetWindowSurface(hr_screen), &screen_rect);
  SDL_UpdateWindowSurfaceRects(hr_screen, &screen_rect, 1);
}

[[maybe_unused]] void I_FadeToPaletteHR(const uint8_t *palette) {
  uint8_t tmppal[48];

  int starttime = I_GetTimeMS();

  for (;;) {
    int elapsed = I_GetTimeMS() - starttime;

    if (elapsed >= FADE_TIME) {
      break;
    }

    // Generate the fake palette

    for (int i = 0; i < 16 * 3; ++i) {
      tmppal[i] = static_cast<uint8_t>((palette[i] * elapsed) / FADE_TIME);
    }

    I_SetPaletteHR(tmppal);
    SDL_UpdateWindowSurface(hr_screen);

    // Sleep a bit

    I_Sleep(10);
  }

  // Set the final palette

  I_SetPaletteHR(palette);
}

[[maybe_unused]] void I_BlackPaletteHR() {
  uint8_t blackpal[16 * 3];

  std::memset(blackpal, 0, sizeof(blackpal));

  I_SetPaletteHR(blackpal);
}

// Check if the user has hit the escape key to abort startup.
[[maybe_unused]] bool I_CheckAbortHR() {
  SDL_Event ev;
  bool      result = false;

  // Not initialized?
  if (hr_surface == nullptr) {
    return false;
  }

  while (SDL_PollEvent(&ev)) {
    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
      result = true;
    }
  }

  return result;
}
