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
//    Exit text-mode ENDOOM screen.
//

#include <cstring>

#include "config.h"
#include "txt_main.hpp"

constexpr auto                  ENDOOM_W = 80;
[[maybe_unused]] constexpr auto ENDOOM_H = 25;

//
// Displays the text mode ending screen after the game quits
//

void I_Endoom(uint8_t *endoom_data) {
  // Set up text mode screen

  TXT_Init();

  TXT_SetWindowTitle(PACKAGE_STRING);
  // SDL2-TODO I_InitWindowTitle();
  // SDL2-TODO I_InitWindowIcon();

  // Write the data to the screen memory

  unsigned char *screendata = TXT_GetScreenData();

  int indent = (ENDOOM_W - TXT_SCREEN_W) / 2;

  for (int y = 0; y < TXT_SCREEN_H; ++y) {
    std::memcpy(screendata + (y * TXT_SCREEN_W * 2),
                endoom_data + (y * ENDOOM_W + indent) * 2,
                TXT_SCREEN_W * 2);
  }

  // Wait for a keypress

  while (true) {
    TXT_UpdateScreen();

    if (TXT_GetChar() > 0) {
      break;
    }

    TXT_Sleep(0);
  }

  // Shut down text mode screen

  TXT_Shutdown();
}
