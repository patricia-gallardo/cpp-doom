//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//      Miscellaneous.
//

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// intentional newline to avoid reordering by Clang Format
#include <io.h>
#ifdef _MSC_VER
#include <direct.h>
#endif
#else
#include <sys/stat.h>
#endif

#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_misc.hpp"
#include "memory.hpp"
#include "z_zone.hpp"

//
// Create a directory
//

void M_MakeDirectory(cstring_view path) {
#ifdef _WIN32
  mkdir(path.c_str());
#else
  mkdir(path.c_str(), 0755);
#endif
}

// Check if a file exists

bool M_FileExists(cstring_view filename) {
  FILE * fstream;

  fstream = fopen(filename.c_str(), "r");

  if (fstream != nullptr) {
    fclose(fstream);
    return true;
  } else {
    // If we can't open because the file is a directory, the
    // "file" exists at least!

    return errno == EISDIR;
  }
}

// Check if a file exists by probing for common case variation of its filename.
// Returns a newly allocated string that the caller is responsible for freeing.

char * M_FileCaseExists(cstring_view path) {
  char *path_dup, *filename, *ext;

  path_dup = M_StringDuplicate(path);

  // 0: actual path
  if (M_FileExists(path_dup)) {
    return path_dup;
  }

  filename = strrchr(path_dup, DIR_SEPARATOR);
  if (filename != nullptr) {
    filename++;
  } else {
    filename = path_dup;
  }

  // 1: lowercase filename, e.g. doom2.wad
  M_ForceLowercase(filename);

  if (M_FileExists(path_dup)) {
    return path_dup;
  }

  // 2: uppercase filename, e.g. DOOM2.WAD
  M_ForceUppercase(filename);

  if (M_FileExists(path_dup)) {
    return path_dup;
  }

  // 3. uppercase basename with lowercase extension, e.g. DOOM2.wad
  ext = strrchr(path_dup, '.');
  if (ext != nullptr && ext > filename) {
    M_ForceLowercase(ext + 1);

    if (M_FileExists(path_dup)) {
      return path_dup;
    }
  }

  // 4. lowercase filename with uppercase first letter, e.g. Doom2.wad
  if (strlen(filename) > 1) {
    M_ForceLowercase(filename + 1);

    if (M_FileExists(path_dup)) {
      return path_dup;
    }
  }

  // 5. no luck
  free(path_dup);
  return nullptr;
}

//
// Determine the length of an open file.
//

long M_FileLength(FILE * handle) {
  long savedpos;
  long length;

  // save the current position in the file
  savedpos = ftell(handle);

  // jump to the end and find the length
  fseek(handle, 0, SEEK_END);
  length = ftell(handle);

  // go back to the old location
  fseek(handle, savedpos, SEEK_SET);

  return length;
}

//
// M_WriteFile
//

bool M_WriteFile(cstring_view name, const void * source, int length) {
  FILE * handle = fopen(name.c_str(), "wb");
  if (handle == nullptr)
    return false;

  size_t count = fwrite(source, 1, static_cast<size_t>(length), handle);
  fclose(handle);

  if (count < static_cast<size_t>(length))
    return false;

  return true;
}

//
// M_ReadFile
//

int M_ReadFile(cstring_view name, uint8_t ** buffer) {
  FILE * handle = fopen(name.c_str(), "rb");
  if (handle == nullptr)
    I_Error("Couldn't read file %s", name.c_str());

  // find the size of the file by seeking to the end and
  // reading the current position

  long length = M_FileLength(handle);

  uint8_t * buf   = zmalloc<uint8_t *>(static_cast<size_t>(length + 1), PU_STATIC, nullptr);
  size_t    count = fread(buf, 1, static_cast<size_t>(length), handle);
  fclose(handle);

  if (static_cast<long>(count) < length)
    I_Error("Couldn't read file %s", name.c_str());

  buf[length] = '\0';
  *buffer     = buf;
  return static_cast<int>(length);
}

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.

char * M_TempFile(cstring_view s) {
  const char * tempdir;

#ifdef _WIN32

  // Check the TEMP environment variable to find the location.

  tempdir = getenv("TEMP");

  if (tempdir == nullptr) {
    tempdir = ".";
  }
#else
  // In Unix, just use /tmp.

  tempdir = "/tmp";
#endif

  return M_StringJoin(tempdir, DIR_SEPARATOR_S, s.c_str());
}

bool M_StrToInt(cstring_view str, int * result) {
  unsigned int * res = reinterpret_cast<unsigned int *>(result);
  return sscanf(str.c_str(), " 0x%x", res) == 1
         || sscanf(str.c_str(), " 0X%x", res) == 1
         || sscanf(str.c_str(), " 0%o", res) == 1
         || sscanf(str.c_str(), " %d", result) == 1;
}

// Returns the directory portion of the given path, without the trailing
// slash separator character. If no directory is described in the path,
// the string "." is returned. In either case, the result is newly allocated
// and must be freed by the caller after use.
char * M_DirName(cstring_view path) {
  const auto * p = strrchr(path.c_str(), DIR_SEPARATOR);
  if (p == nullptr) {
    return M_StringDuplicate(".");
  } else {
    auto * result    = M_StringDuplicate(path);
    result[p - path.c_str()] = '\0';
    return result;
  }
}

// Returns the base filename described by the given path (without the
// directory name). The result points inside path and nothing new is
// allocated.
const char * M_BaseName(cstring_view path) {
  const char * p;

  p = strrchr(path.c_str(), DIR_SEPARATOR);
  if (p == nullptr) {
    return path.c_str();
  } else {
    return p + 1;
  }
}

void M_ExtractFileBase(cstring_view path, char * dest) {
  const char * src;
  const char * filename;
  int          length;

  src = path.c_str() + path.size() - 1;

  // back up until a \ or the start
  while (src != path.c_str() && *(src - 1) != DIR_SEPARATOR) {
    src--;
  }

  filename = src;

  // Copy up to eight characters
  // Note: Vanilla Doom exits with an error if a filename is specified
  // with a base of more than eight characters.  To remove the 8.3
  // filename limit, instead we simply truncate the name.

  length = 0;
  std::memset(dest, 0, 8);

  while (*src != '\0' && *src != '.') {
    if (length >= 8) {
      fmt::printf("Warning: Truncated '%s' lump name to '%.8s'.\n",
                  filename,
                  dest);
      break;
    }

    dest[length++] = static_cast<char>(toupper(static_cast<int>(*src++)));
  }
}

//---------------------------------------------------------------------------
//
// PROC M_ForceUppercase
//
// Change string to uppercase.
//
//---------------------------------------------------------------------------

void M_ForceUppercase(char * text) {
  for (char * p = text; *p != '\0'; ++p) {
    *p = static_cast<char>(toupper(*p));
  }
}

//---------------------------------------------------------------------------
//
// PROC M_ForceLowercase
//
// Change string to lowercase.
//
//---------------------------------------------------------------------------

void M_ForceLowercase(char * text) {
  char * p;

  for (p = text; *p != '\0'; ++p) {
    *p = static_cast<char>(tolower(*p));
  }
}

//
// M_StrCaseStr
//
// Case-insensitive version of strstr()
//

[[maybe_unused]] const char * M_StrCaseStr(cstring_view haystack, cstring_view needle) {
  size_t haystack_len = haystack.size();
  size_t needle_len   = needle.size();

  if (haystack_len < needle_len) {
    return nullptr;
  }

  size_t len = haystack_len - needle_len;

  for (size_t i = 0; i <= len; ++i) {
    if (!strncasecmp(haystack.c_str() + i, needle.c_str(), needle_len)) {
      return haystack.c_str() + i;
    }
  }

  return nullptr;
}

//
// Safe version of strdup() that checks the string was successfully
// allocated.
//

char * M_StringDuplicate(cstring_view orig) {
  char * result;

  result = strdup(orig.c_str());

  if (result == nullptr) {
    I_Error("Failed to duplicate string (length %" PRIuPTR ")\n",
            orig.size());
  }

  return result;
}

//
// String replace function.
//

char * M_StringReplace(cstring_view haystack, cstring_view needle, cstring_view replacement) {
  std::string result = haystack.c_str();
  for (std::string::size_type pos{}; std::string::npos != (pos = result.find(needle.data(), pos, needle.size())); pos += replacement.size()) {
    result.replace(pos, needle.size(), replacement.data(), replacement.size());
  }
  return strdup(result.c_str());
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

bool M_StringCopy(char * dest, cstring_view src, size_t dest_size) {
  size_t len;

  if (dest_size >= 1) {
    dest[dest_size - 1] = '\0';
    strncpy(dest, src.c_str(), dest_size - 1);
  } else {
    return false;
  }

  len = strlen(dest);
  const char * ptr = src.c_str();
  return ptr[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

bool M_StringConcat(char * dest, cstring_view src, size_t dest_size) {
  size_t offset;

  offset = strlen(dest);
  if (offset > dest_size) {
    offset = dest_size;
  }

  return M_StringCopy(dest + offset, src, dest_size - offset);
}

// Returns true if 's' begins with the specified prefix.

bool M_StringStartsWith(cstring_view s, cstring_view prefix) {
  return s.size() >= prefix.size()
         && strncmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
}

// Returns true if 's' ends with the specified suffix.

bool M_StringEndsWith(cstring_view s, cstring_view suffix) {
  return s.size() >= suffix.size()
         && strcmp(s.c_str() + s.size() - suffix.size(), suffix.c_str()) == 0;
}

// On Windows, vsnprintf() is _vsnprintf().
#ifdef _WIN32
#if _MSC_VER < 1400 /* not needed for Visual Studio 2008 */
#define vsnprintf _vsnprintf
#endif
#endif

// Safe, portable vsnprintf().
int M_vsnprintf(char * buf, size_t buf_len, cstring_view s, va_list args) {
  if (buf_len < 1) {
    return 0;
  }

  // Windows (and other OSes?) has a vsnprintf() that doesn't always
  // append a trailing \0. So we must do it, and write into a buffer
  // that is one byte shorter; otherwise this function is unsafe.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  int result = vsnprintf(buf, buf_len, s.c_str(), args);
#pragma GCC diagnostic pop

  // If truncated, change the final char in the buffer to a \0.
  // A negative result indicates a truncated buffer on Windows.
  if (result < 0 || result >= static_cast<int>(buf_len)) {
    buf[buf_len - 1] = '\0';
    result           = static_cast<int>(buf_len - 1);
  }

  return result;
}

#ifdef _WIN32

char * M_OEMToUTF8(cstring_view oem) {
  size_t    len = oem.size() + 1;
  wchar_t * tmp;
  char *    result;

  tmp = static_cast<wchar_t *>(malloc(len * sizeof(wchar_t)));
  MultiByteToWideChar(CP_OEMCP, 0, oem.c_str(), static_cast<int>(len), tmp, static_cast<int>(len));
  result = static_cast<char *>(malloc(len * 4));
  WideCharToMultiByte(CP_UTF8, 0, tmp, static_cast<int>(len), result, static_cast<int>(len) * 4, nullptr, nullptr);
  free(tmp);

  return result;
}

#endif
