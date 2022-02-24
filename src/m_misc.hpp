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
//      Miscellaneous.
//

#ifndef __M_MISC__
#define __M_MISC__

#include <cstdio>
#include <cstdarg>

#include "doomtype.hpp"

bool        M_WriteFile(const char *name, const void *source, int length);
int         M_ReadFile(const char *name, uint8_t **buffer);
void        M_MakeDirectory(const char *dir);
char       *M_TempFile(const char *s);
bool        M_FileExists(const char *file);
char       *M_FileCaseExists(const char *file);
long        M_FileLength(FILE *handle);
bool        M_StrToInt(const char *str, int *result);
char       *M_DirName(const char *path);
const char *M_BaseName(const char *path);
void        M_ExtractFileBase(const char *path, char *dest);
void        M_ForceUppercase(char *text);
void        M_ForceLowercase(char *text);
const char *M_StrCaseStr(const char *haystack, const char *needle);
char       *M_StringDuplicate(const char *orig);
bool        M_StringCopy(char *dest, const char *src, size_t dest_size);
bool        M_StringConcat(char *dest, const char *src, size_t dest_size);
char       *M_StringReplace(const char *haystack, const char *needle, const char *replacement);
char       *M_StringJoin(const char *s, ...);
bool        M_StringStartsWith(const char *s, const char *prefix);
bool        M_StringEndsWith(const char *s, const char *suffix);
int         M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);
int         M_snprintf(char *buf, size_t buf_len, const char *s, ...) PRINTF_ATTR(3, 4);
char       *M_OEMToUTF8(const char *ansi);

#endif
