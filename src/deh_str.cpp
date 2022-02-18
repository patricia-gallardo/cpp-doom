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
//
// Parses Text substitution sections in dehacked files
//

#include <cstdarg>
#include <string>
#include <vector>
#include <unordered_map>

#include "deh_str.hpp"
#include "m_misc.hpp"

#include "memory.hpp"
#include "z_zone.hpp"

struct deh_substitution_t {
    std::string from_text;
    std::string to_text;
};

static std::unordered_map<std::string, deh_substitution_t> hash_table;

static std::optional<deh_substitution_t> SubstitutionForString(std::string s)
{
    auto search = hash_table.find(s);
    if (search != hash_table.end()) {
        return search->second;
    }
    else
    {
        return {};
    }
}

// Look up a string to see if it has been replaced with something else
// This will be used throughout the program to substitute text

std::string DEH_String(std::string_view s)
{
    std::string needle(s);
    auto subst = SubstitutionForString(needle);

    if (subst)
    {
        return subst->to_text;
    }
    else
    {
        return needle;
    }
}

// [crispy] returns true if a string has been substituted

bool DEH_HasStringReplacement(const char *s)
{
    return DEH_String(s) != s;
}

void DEH_AddStringReplacement(const char *from_text, const char *to_text)
{
    // Check to see if there is an existing substitution already in place.
    auto sub = SubstitutionForString(from_text);

    if (sub)
    {
        sub->to_text = to_text;
    }
    else
    {
        deh_substitution_t substitution;
        substitution.from_text = from_text;
        substitution.to_text = to_text;
        hash_table[substitution.from_text] = substitution;
    }
}

void DEH_AddStringReplacement(const std::string & from_text, const std::string & to_text)
{
    DEH_AddStringReplacement(from_text.c_str(), to_text.c_str());
}

enum format_arg_t
{
    FORMAT_ARG_INVALID,
    FORMAT_ARG_INT,
    FORMAT_ARG_FLOAT,
    FORMAT_ARG_CHAR,
    FORMAT_ARG_STRING,
    FORMAT_ARG_PTR,
    FORMAT_ARG_SAVE_POS
};

// Get the type of a format argument.
// We can mix-and-match different format arguments as long as they
// are for the same data type.

static format_arg_t FormatArgumentType(char c)
{
    switch (c)
    {
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
    case 'X':
        return FORMAT_ARG_INT;

    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
        return FORMAT_ARG_FLOAT;

    case 'c':
    case 'C':
        return FORMAT_ARG_CHAR;

    case 's':
    case 'S':
        return FORMAT_ARG_STRING;

    case 'p':
        return FORMAT_ARG_PTR;

    case 'n':
        return FORMAT_ARG_SAVE_POS;

    default:
        return FORMAT_ARG_INVALID;
    }
}

// Given the specified string, get the type of the first format
// string encountered.

static format_arg_t NextFormatArgument(const char **str)
{
    // Search for the '%' starting the next string.

    while (**str != '\0')
    {
        if (**str == '%')
        {
            ++*str;

            // Don't stop for double-%s.

            if (**str != '%')
            {
                break;
            }
        }

        ++*str;
    }

    // Find the type of the format string.

    while (**str != '\0')
    {
        format_arg_t argtype = FormatArgumentType(**str);

        if (argtype != FORMAT_ARG_INVALID)
        {
            ++*str;

            return argtype;
        }

        ++*str;
    }

    // Stop searching, we have reached the end.

    *str = nullptr;

    return FORMAT_ARG_INVALID;
}

// Check if the specified argument type is a valid replacement for
// the original.

static bool ValidArgumentReplacement(format_arg_t original,
    format_arg_t                                     replacement)
{
    // In general, the original and replacement types should be
    // identical.  However, there are some cases where the replacement
    // is valid and the types don't match.

    // Characters can be represented as ints.

    if (original == FORMAT_ARG_CHAR && replacement == FORMAT_ARG_INT)
    {
        return true;
    }

    // Strings are pointers.

    if (original == FORMAT_ARG_STRING && replacement == FORMAT_ARG_PTR)
    {
        return true;
    }

    return original == replacement;
}

// Return true if the specified string contains no format arguments.

static bool ValidFormatReplacement(const std::string & original, const std::string & replacement)
{
    // Check each argument in turn and compare types.

    const char *rover1 = original.c_str();
    const char *rover2 = replacement.c_str();

    for (;;)
    {
        const auto argtype1 = NextFormatArgument(&rover1);
        const auto argtype2 = NextFormatArgument(&rover2);

        if (argtype2 == FORMAT_ARG_INVALID)
        {
            // No more arguments left to read from the replacement string.

            break;
        }
        else if (argtype1 == FORMAT_ARG_INVALID)
        {
            // Replacement string has more arguments than the original.

            return false;
        }
        else if (!ValidArgumentReplacement(argtype1, argtype2))
        {
            // Not a valid replacement argument.

            return false;
        }
    }

    return true;
}

// Get replacement format string, checking arguments.

static std::string FormatStringReplacement(const std::string & s)
{
    auto repl = DEH_String(s);

    if (!ValidFormatReplacement(s, repl))
    {
        printf("WARNING: Unsafe dehacked replacement provided for "
               "printf format string: %s\n",
            s.c_str());

        return s;
    }

    return repl;
}

// printf(), performing a replacement on the format string.

void DEH_printf(const char *fmt, ...)
{
    va_list     args;
    auto repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    vprintf(repl.c_str(), args);

    va_end(args);
}

// fprintf(), performing a replacement on the format string.

void DEH_fprintf(FILE *fstream, const char *fmt, ...)
{
    va_list     args;
    auto repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    vfprintf(fstream, repl.c_str(), args);

    va_end(args);
}

// snprintf(), performing a replacement on the format string.

void DEH_snprintf(char *buffer, size_t len, const char *fmt, ...)
{
    va_list     args;
    auto repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    M_vsnprintf(buffer, len, repl.c_str(), args);

    va_end(args);
}
