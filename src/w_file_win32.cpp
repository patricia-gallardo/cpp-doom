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
//	WAD I/O functions.
//

#include "config.h"

#ifdef _WIN32

#include <cstdio>
#include <memory.hpp>

#include <fmt/printf.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// intentional newline to avoid reordering by Clang Format

#include "i_system.hpp"
#include "m_misc.hpp"
#include "w_file.hpp"
#include "z_zone.hpp"

// This constant doesn't exist in VC6:

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER 0xffffffff
#endif

struct win32_wad_file_t {
  wad_file_t wad;
  HANDLE     handle;
  HANDLE     handle_map;
};

extern wad_file_class_t win32_wad_file;

static void MapFile(win32_wad_file_t * wad, const char * filename) {
  wad->handle_map = CreateFileMapping(wad->handle,
                                      nullptr,
                                      PAGE_WRITECOPY,
                                      0,
                                      0,
                                      nullptr);

  if (wad->handle_map == nullptr) {
    fmt::fprintf(stderr, "W_Win32_OpenFile: Unable to CreateFileMapping() "
                         "for %s\n",
                 filename);
    return;
  }

  wad->wad.mapped = static_cast<uint8_t *>(MapViewOfFile(wad->handle_map,
                                                         FILE_MAP_COPY,
                                                         0,
                                                         0,
                                                         0));

  if (wad->wad.mapped == nullptr) {
    fmt::fprintf(stderr, "W_Win32_OpenFile: Unable to MapViewOfFile() for %s\n", filename);
  }
}

unsigned int GetFileLength(HANDLE handle) {
  DWORD result = SetFilePointer(handle, 0, nullptr, FILE_END);

  if (result == INVALID_SET_FILE_POINTER) {
    I_Error("W_Win32_OpenFile: Failed to read file length");
  }

  return result;
}

static wad_file_t * W_Win32_OpenFile(cstring_view path) {
  wchar_t wpath[MAX_PATH + 1];

  // Open the file:

  MultiByteToWideChar(CP_OEMCP, 0, path.c_str(), static_cast<int>(strlen(path.c_str()) + 1), wpath, static_cast<int>(sizeof(wpath)));

  HANDLE handle = CreateFileW(wpath,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              nullptr,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);

  if (handle == INVALID_HANDLE_VALUE) {
    return nullptr;
  }

  // Create a new win32_wad_file_t to hold the file handle.

  win32_wad_file_t * result = zmalloc<win32_wad_file_t *>(sizeof(win32_wad_file_t), PU_STATIC, 0);
  result->wad.file_class    = &win32_wad_file;
  result->wad.length        = GetFileLength(handle);
  result->wad.path          = M_StringDuplicate(path);
  result->handle            = handle;

  // Try to map the file into memory with mmap:

  MapFile(result, path.c_str());

  return &result->wad;
}

static void W_Win32_CloseFile(wad_file_t * wad) {
  auto * win32_wad = reinterpret_cast<win32_wad_file_t *>(wad);

  // If mapped, unmap it.

  if (win32_wad->wad.mapped != nullptr) {
    UnmapViewOfFile(win32_wad->wad.mapped);
  }

  if (win32_wad->handle_map != nullptr) {
    CloseHandle(win32_wad->handle_map);
  }

  // Close the file

  if (win32_wad->handle != nullptr) {
    CloseHandle(win32_wad->handle);
  }

  Z_Free(win32_wad);
}

// Read data from the specified position in the file into the
// provided buffer.  Returns the number of bytes read.

size_t W_Win32_Read(wad_file_t * wad, unsigned int offset, void * buffer, size_t buffer_len) {
  auto * win32_wad = reinterpret_cast<win32_wad_file_t *>(wad);

  // Jump to the specified position in the file.

  DWORD result = SetFilePointer(win32_wad->handle, offset, nullptr, FILE_BEGIN);

  if (result == INVALID_SET_FILE_POINTER) {
    I_Error("W_Win32_Read: Failed to set file pointer to %i",
            offset);
  }

  // Read into the buffer.
  DWORD bytes_read = 0;
  if (!ReadFile(win32_wad->handle, buffer, static_cast<DWORD>(buffer_len), &bytes_read, nullptr)) {
    I_Error("W_Win32_Read: Error reading from file");
  }

  return bytes_read;
}

wad_file_class_t win32_wad_file = {
  W_Win32_OpenFile,
  W_Win32_CloseFile,
  W_Win32_Read,
};

#endif /* #ifdef _WIN32 */
