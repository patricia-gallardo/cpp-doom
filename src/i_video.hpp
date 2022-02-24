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
//	System specific interface stuff.
//

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.hpp"
#include "crispy.hpp"

// Screen width and height.

constexpr auto ORIGWIDTH  = 320; // [crispy]
constexpr auto ORIGHEIGHT = 200; // [crispy]

constexpr auto MAXWIDTH  = (ORIGWIDTH << 2);  // [crispy]
constexpr auto MAXHEIGHT = (ORIGHEIGHT << 1); // [crispy]

extern int SCREENWIDTH;
extern int SCREENHEIGHT;
extern int HIRESWIDTH;              // [crispy] non-widescreen SCREENWIDTH
extern int DELTAWIDTH;              // [crispy] horizontal widescreen offset
void       I_GetScreenDimensions(); // [crispy] re-calculate DELTAWIDTH

// Screen height used when aspect_ratio_correct=true.

constexpr auto                  ORIGHEIGHT_4_3 = 240;                   // [crispy]
[[maybe_unused]] constexpr auto MAXHEIGHT_4_3  = (ORIGHEIGHT_4_3 << 1); // [crispy]

[[maybe_unused]] extern int SCREENHEIGHT_4_3;

using grabmouse_callback_t = bool (*)();

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics();

void I_GraphicsCheckCommandLine();

void I_ShutdownGraphics();

// Takes full 8 bit values.
#ifndef CRISPY_TRUECOLOR
void I_SetPalette(uint8_t * palette);
int  I_GetPaletteIndex(int r, int g, int b);
#else
void                 I_SetPalette(int palette);
extern const pixel_t I_MapRGB(const uint8_t r, const uint8_t g, const uint8_t b);
#endif

void I_FinishUpdate();

void I_ReadScreen(pixel_t * scr);

[[maybe_unused]] void I_BeginRead();

void I_SetWindowTitle(const char * title);

void I_CheckIsScreensaver();
void I_SetGrabMouseCallback(grabmouse_callback_t func);

void I_DisplayFPSDots(bool dots_on);
void I_BindVideoVariables();

void I_InitWindowTitle();
void I_InitWindowIcon();

// Called before processing any tics in a frame (just after displaying a frame).
// Time consuming syncronous operations are performed here (joystick reading).

void I_StartFrame();

// Called before processing each tic in a frame.
// Quick syncronous operations are performed here.

void I_StartTic();

// Enable the loading disk image displayed when reading from disk.

[[maybe_unused]] void I_EnableLoadingDisk(int xoffs, int yoffs);

struct i_video_t {
  // SDL video driver name
  char * video_driver;
  // Flag indicating whether the screen is currently visible:
  // when the screen isnt visible, don't render the screen
  bool screenvisible;

  int vanilla_keyboard_mapping;
  // If true, game is running as a screensaver
  bool screensaver_mode;
  // Gamma correction level to use
  int usegamma;
  // The screen buffer; this is modified to draw things to the screen
  pixel_t * I_VideoBuffer;

  int screen_width;
  int screen_height;
  // Run in full screen mode?  (int type for config code)
  int fullscreen;
  // Aspect ratio correction mode
  int aspect_ratio_correct;
  // Force integer scales for resolution-independent rendering
  int integer_scaling;
  // VGA Porch palette change emulation
  int vga_porch_flash;
  // Force software rendering, for systems which lack effective hardware
  // acceleration
  int force_software_renderer;

  // Window position:
  char * window_position;

  // Joystic/gamepad hysteresis
  unsigned int joywait;
};

extern i_video_t * const g_i_video_globals;

void I_GetWindowPosition(int * x, int * y, int w, int h);

#endif
