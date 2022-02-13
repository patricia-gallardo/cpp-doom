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


#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#ifdef _MSC_VER
#include <direct.h>
#endif
#else
#include <sys/stat.h>
#endif

#include "doomtype.hpp"

#include "deh_str.hpp"

#include "memory.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "i_video.hpp"
#include "m_misc.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

//
// Create a directory
//

void M_MakeDirectory(const char *path)
{
#ifdef _WIN32
    mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

// Check if a file exists

bool M_FileExists(const char *filename)
{
    FILE *fstream;

    fstream = fopen(filename, "r");

    if (fstream != nullptr)
    {
        fclose(fstream);
        return true;
    }
    else
    {
        // If we can't open because the file is a directory, the
        // "file" exists at least!

        return errno == EISDIR;
    }
}

// Check if a file exists by probing for common case variation of its filename.
// Returns a newly allocated string that the caller is responsible for freeing.

char *M_FileCaseExists(const char *path)
{
    char *path_dup, *filename, *ext;

    path_dup = M_StringDuplicate(path);

    // 0: actual path
    if (M_FileExists(path_dup))
    {
        return path_dup;
    }

    filename = strrchr(path_dup, DIR_SEPARATOR);
    if (filename != nullptr)
    {
        filename++;
    }
    else
    {
        filename = path_dup;
    }

    // 1: lowercase filename, e.g. doom2.wad
    M_ForceLowercase(filename);

    if (M_FileExists(path_dup))
    {
        return path_dup;
    }

    // 2: uppercase filename, e.g. DOOM2.WAD
    M_ForceUppercase(filename);

    if (M_FileExists(path_dup))
    {
        return path_dup;
    }

    // 3. uppercase basename with lowercase extension, e.g. DOOM2.wad
    ext = strrchr(path_dup, '.');
    if (ext != nullptr && ext > filename)
    {
        M_ForceLowercase(ext + 1);

        if (M_FileExists(path_dup))
        {
            return path_dup;
        }
    }

    // 4. lowercase filename with uppercase first letter, e.g. Doom2.wad
    if (strlen(filename) > 1)
    {
        M_ForceLowercase(filename + 1);

        if (M_FileExists(path_dup))
        {
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

long M_FileLength(FILE *handle)
{
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

bool M_WriteFile(const char *name, const void *source, int length)
{
    FILE *handle = fopen(name, "wb");
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

int M_ReadFile(const char *name, uint8_t **buffer)
{
    FILE *handle = fopen(name, "rb");
    if (handle == nullptr)
        I_Error("Couldn't read file %s", name);

    // find the size of the file by seeking to the end and
    // reading the current position

    long length = M_FileLength(handle);

    uint8_t *buf = zmalloc<uint8_t *>(static_cast<size_t>(length + 1), PU_STATIC, nullptr);
    size_t count = fread(buf, 1, static_cast<size_t>(length), handle);
    fclose(handle);

    if (static_cast<long>(count) < length)
        I_Error("Couldn't read file %s", name);

    buf[length] = '\0';
    *buffer     = buf;
    return static_cast<int>(length);
}

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.

char *M_TempFile(const char *s)
{
    const char *tempdir;

#ifdef _WIN32

    // Check the TEMP environment variable to find the location.

    tempdir = getenv("TEMP");

    if (tempdir == nullptr)
    {
        tempdir = ".";
    }
#else
    // In Unix, just use /tmp.

    tempdir = "/tmp";
#endif

    return M_StringJoin(tempdir, DIR_SEPARATOR_S, s, nullptr);
}

bool M_StrToInt(const char *str, int *result)
{
    return sscanf(str, " 0x%x", result) == 1
           || sscanf(str, " 0X%x", result) == 1
           || sscanf(str, " 0%o", result) == 1
           || sscanf(str, " %d", result) == 1;
}

// Returns the directory portion of the given path, without the trailing
// slash separator character. If no directory is described in the path,
// the string "." is returned. In either case, the result is newly allocated
// and must be freed by the caller after use.
char *M_DirName(const char *path)
{
    const auto *p = strrchr(path, DIR_SEPARATOR);
    if (p == nullptr)
    {
        return M_StringDuplicate(".");
    }
    else
    {
        auto *result     = M_StringDuplicate(path);
        result[p - path] = '\0';
        return result;
    }
}

// Returns the base filename described by the given path (without the
// directory name). The result points inside path and nothing new is
// allocated.
const char *M_BaseName(const char *path)
{
    const char *p;

    p = strrchr(path, DIR_SEPARATOR);
    if (p == nullptr)
    {
        return path;
    }
    else
    {
        return p + 1;
    }
}

void M_ExtractFileBase(const char *path, char *dest)
{
    const char *src;
    const char *filename;
    int         length;

    src = path + strlen(path) - 1;

    // back up until a \ or the start
    while (src != path && *(src - 1) != DIR_SEPARATOR)
    {
        src--;
    }

    filename = src;

    // Copy up to eight characters
    // Note: Vanilla Doom exits with an error if a filename is specified
    // with a base of more than eight characters.  To remove the 8.3
    // filename limit, instead we simply truncate the name.

    length = 0;
    memset(dest, 0, 8);

    while (*src != '\0' && *src != '.')
    {
        if (length >= 8)
        {
            printf("Warning: Truncated '%s' lump name to '%.8s'.\n",
                filename, dest);
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

void M_ForceUppercase(char *text)
{
    for (char *p = text; *p != '\0'; ++p)
    {
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

void M_ForceLowercase(char *text)
{
    char *p;

    for (p = text; *p != '\0'; ++p)
    {
        *p = static_cast<char>(tolower(*p));
    }
}

//
// M_StrCaseStr
//
// Case-insensitive version of strstr()
//

const char *M_StrCaseStr(const char *haystack, const char *needle)
{
    size_t haystack_len = strlen(haystack);
    size_t needle_len   = strlen(needle);

    if (haystack_len < needle_len)
    {
        return nullptr;
    }

    size_t len = haystack_len - needle_len;

    for (size_t i = 0; i <= len; ++i)
    {
        if (!strncasecmp(haystack + i, needle, needle_len))
        {
            return haystack + i;
        }
    }

    return nullptr;
}

//
// Safe version of strdup() that checks the string was successfully
// allocated.
//

char *M_StringDuplicate(const char *orig)
{
    char *result;

    result = strdup(orig);

    if (result == nullptr)
    {
        I_Error("Failed to duplicate string (length %" PRIuPTR ")\n",
            strlen(orig));
    }

    return result;
}

//
// String replace function.
//

char *M_StringReplace(const char *haystack, const char *needle,
    const char *replacement)
{
    char *      result, *dst;
    const char *p;
    size_t      needle_len = strlen(needle);
    size_t      result_len, dst_len;

    // Iterate through occurrences of 'needle' and calculate the size of
    // the new string.
    result_len = strlen(haystack) + 1;
    p          = haystack;

    for (;;)
    {
        p = strstr(p, needle);
        if (p == nullptr)
        {
            break;
        }

        p += needle_len;
        result_len += strlen(replacement) - needle_len;
    }

    // Construct new string.

    result = static_cast<char *>(malloc(result_len));
    if (result == nullptr)
    {
        I_Error("M_StringReplace: Failed to allocate new string");
        return nullptr;
    }

    dst     = result;
    dst_len = result_len;
    p       = haystack;

    while (*p != '\0')
    {
        if (!strncmp(p, needle, needle_len))
        {
            M_StringCopy(dst, replacement, dst_len);
            p += needle_len;
            dst += strlen(replacement);
            dst_len -= strlen(replacement);
        }
        else
        {
            *dst = *p;
            ++dst;
            --dst_len;
            ++p;
        }
    }

    *dst = '\0';

    return result;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

bool M_StringCopy(char *dest, const char *src, size_t dest_size)
{
    size_t len;

    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
    else
    {
        return false;
    }

    len = strlen(dest);
    return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

bool M_StringConcat(char *dest, const char *src, size_t dest_size)
{
    size_t offset;

    offset = strlen(dest);
    if (offset > dest_size)
    {
        offset = dest_size;
    }

    return M_StringCopy(dest + offset, src, dest_size - offset);
}

// Returns true if 's' begins with the specified prefix.

bool M_StringStartsWith(const char *s, const char *prefix)
{
    return strlen(s) >= strlen(prefix)
           && strncmp(s, prefix, strlen(prefix)) == 0;
}

// Returns true if 's' ends with the specified suffix.

bool M_StringEndsWith(const char *s, const char *suffix)
{
    return strlen(s) >= strlen(suffix)
           && strcmp(s + strlen(s) - strlen(suffix), suffix) == 0;
}

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.

char *M_StringJoin(const char *s, ...)
{
    char *      result;
    const char *v;
    va_list     args;
    size_t      result_len;

    result_len = strlen(s) + 1;

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, const char *);
        if (v == nullptr)
        {
            break;
        }

        result_len += strlen(v);
    }
    va_end(args);

    result = static_cast<char *>(malloc(result_len));

    if (result == nullptr)
    {
        I_Error("M_StringJoin: Failed to allocate new string.");
        return nullptr;
    }

    M_StringCopy(result, s, result_len);

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, const char *);
        if (v == nullptr)
        {
            break;
        }

        M_StringConcat(result, v, result_len);
    }
    va_end(args);

    return result;
}

// On Windows, vsnprintf() is _vsnprintf().
#ifdef _WIN32
#if _MSC_VER < 1400 /* not needed for Visual Studio 2008 */
#define vsnprintf _vsnprintf
#endif
#endif

// Safe, portable vsnprintf().
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
    if (buf_len < 1)
    {
        return 0;
    }

    // Windows (and other OSes?) has a vsnprintf() that doesn't always
    // append a trailing \0. So we must do it, and write into a buffer
    // that is one byte shorter; otherwise this function is unsafe.
    int result = vsnprintf(buf, buf_len, s, args);

    // If truncated, change the final char in the buffer to a \0.
    // A negative result indicates a truncated buffer on Windows.
    if (result < 0 || result >= static_cast<int>(buf_len))
    {
        buf[buf_len - 1] = '\0';
        result           = static_cast<int>(buf_len - 1);
    }

    return result;
}

// Safe, portable snprintf().
int M_snprintf(char *buf, size_t buf_len, const char *s, ...)
{
    va_list args;
    va_start(args, s);
    int result = M_vsnprintf(buf, buf_len, s, args);
    va_end(args);
    return result;
}

#ifdef _WIN32

char *M_OEMToUTF8(const char *oem)
{
    size_t len = strlen(oem) + 1;
    wchar_t *    tmp;
    char *       result;

    tmp = static_cast<wchar_t *>(malloc(len * sizeof(wchar_t)));
    MultiByteToWideChar(CP_OEMCP, 0, oem, static_cast<int>(len), tmp, static_cast<int>(len));
    result = static_cast<char *>(malloc(len * 4));
    WideCharToMultiByte(CP_UTF8, 0, tmp, static_cast<int>(len), result, static_cast<int>(len) * 4, nullptr, nullptr);
    free(tmp);

    return result;
}

#endif
