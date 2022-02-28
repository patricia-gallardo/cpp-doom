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
//      System-specific joystick interface.
//

#pragma once

// Number of "virtual" joystick buttons defined in configuration files.
// This needs to be at least as large as the number of different key
// bindings supported by the higher-level game code (joyb* variables).
constexpr auto NUM_VIRTUAL_BUTTONS = 11;

// If this bit is set in a configuration file axis value, the axis is
// not actually a joystick axis, but instead is a "button axis". This
// means that instead of reading an SDL joystick axis, we read the
// state of two buttons to get the axis value. This is needed for eg.
// the PS3 SIXAXIS controller, where the D-pad buttons register as
// buttons, not as two axes.
constexpr auto BUTTON_AXIS = 0x10000;

// Query whether a given axis value describes a button axis.
template<typename T>
constexpr auto IS_BUTTON_AXIS(T axis) { return ((axis) >= 0 && ((axis)&BUTTON_AXIS) != 0); }

// Get the individual buttons from a button axis value.
template<typename T>
constexpr auto BUTTON_AXIS_NEG(T axis) { return ((axis)&0xff); }
template<typename T>
constexpr auto BUTTON_AXIS_POS(T axis) { return (((axis) >> 8) & 0xff); }

// Create a button axis value from two button values.
template<typename N, typename P>
constexpr auto CREATE_BUTTON_AXIS(N neg, P pos) { return (BUTTON_AXIS | (neg) | ((pos) << 8)); }

// If this bit is set in an axis value, the axis is not actually a
// joystick axis, but is a "hat" axis. This means that we read (one of)
// the hats on the joystick.
constexpr auto HAT_AXIS = 0x20000;

template<typename T>
constexpr auto IS_HAT_AXIS(T axis) { return ((axis) >= 0 && ((axis)&HAT_AXIS) != 0); }

// Get the hat number from a hat axis value.
template<typename T>
constexpr auto HAT_AXIS_HAT(T axis) { return ((axis)&0xff); }
// Which axis of the hat? (horizonal or vertical)
template<typename T>
constexpr auto HAT_AXIS_DIRECTION(T axis) { return (((axis) >> 8) & 0xff); }

template<typename H, typename D>
constexpr auto CREATE_HAT_AXIS(H hat, D direction) { return (HAT_AXIS | (hat) | ((direction) << 8)); }

constexpr auto HAT_AXIS_HORIZONTAL = 1;
constexpr auto HAT_AXIS_VERTICAL   = 2;

void I_InitJoystick();
void I_ShutdownJoystick();
void I_UpdateJoystick();

void I_BindJoystickVariables();
