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
//     GUS emulation code.
//
//     Actually emulating a GUS is far too much work; fortunately
//     GUS "emulation" already exists in the form of Timidity, which
//     supports GUS patch files. This code therefore converts Doom's
//     DMXGUS lump into an equivalent Timidity configuration file.
//


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "memory.hpp"
#include "m_misc.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"
#include "gusconf.hpp"

constexpr auto MAX_INSTRUMENTS = 256;

typedef struct
{
    char *       patch_names[MAX_INSTRUMENTS];
    int          used[MAX_INSTRUMENTS];
    int          mapping[MAX_INSTRUMENTS];
    unsigned int count;
} gus_config_t;

static gusconf_t gusconf_s = {
    .gus_patch_path = const_cast<char *>(""),
    .gus_ram_kb     = 1024
};
gusconf_t *const g_gusconf_globals = &gusconf_s;

static unsigned int MappingIndex()
{
    auto result = static_cast<unsigned int>(g_gusconf_globals->gus_ram_kb / 256);

    if (result < 1)
    {
        return 1;
    }
    else if (result > 4)
    {
        return 4;
    }
    else
    {
        return result;
    }
}

static int SplitLine(char *line, char **fields, unsigned int max_fields)
{
    char *p = nullptr;

    fields[0]  = line;
    unsigned int num_fields = 1;

    for (p = line; *p != '\0'; ++p)
    {
        if (*p == ',')
        {
            *p = '\0';

            // Skip spaces following the comma.
            do
            {
                ++p;
            } while (*p != '\0' && isspace(*p));

            fields[num_fields] = p;
            ++num_fields;
            --p;

            if (num_fields >= max_fields)
            {
                break;
            }
        }
        else if (*p == '#')
        {
            *p = '\0';
            break;
        }
    }

    // Strip off trailing whitespace from the end of the line.
    p = fields[num_fields - 1] + strlen(fields[num_fields - 1]);
    while (p > fields[num_fields - 1] && isspace(*(p - 1)))
    {
        --p;
        *p = '\0';
    }

    return static_cast<int>(num_fields);
}

static void ParseLine(gus_config_t *config, char *line)
{
    char *fields[6];

    auto num_fields = static_cast<unsigned int>(SplitLine(line, fields, 6));

    if (num_fields < 6)
    {
        return;
    }

    auto instr_id = static_cast<unsigned int>(std::atoi(fields[0]));

    // Skip non GM percussions.
    if ((instr_id >= 128 && instr_id < 128 + 35) || instr_id > 128 + 81)
    {
        return;
    }

    auto mapped_id = static_cast<unsigned int>(std::atoi(fields[MappingIndex()]));
    unsigned int i = 0;

    for (i = 0; i < config->count; i++)
    {
        if (config->used[i] == static_cast<int>(mapped_id))
        {
            break;
        }
    }

    if (i == config->count)
    {
        // DMX uses wrong patch name (we should use name of 'mapped_id'
        // instrument, but DMX uses name of 'instr_id' instead).
        free(config->patch_names[i]);
        config->patch_names[i] = M_StringDuplicate(fields[5]);
        config->used[i]        = static_cast<int>(mapped_id);
        config->count++;
    }
    config->mapping[instr_id] = static_cast<int>(i);
}

static void ParseDMXConfig(char *dmxconf, gus_config_t *config)
{
    std::memset(config, 0, sizeof(gus_config_t));

    for (unsigned int i = 0; i < MAX_INSTRUMENTS; ++i)
    {
        config->mapping[i] = -1;
        config->used[i]    = -1;
    }

    config->count = 0;

    char *p = dmxconf;

    for (;;)
    {
        char *newline = strchr(p, '\n');

        if (newline != nullptr)
        {
            *newline = '\0';
        }

        ParseLine(config, p);

        if (newline == nullptr)
        {
            break;
        }
        else
        {
            p = newline + 1;
        }
    }
}

static void FreeDMXConfig(gus_config_t *config)
{
    for (auto & patch_name : config->patch_names)
    {
        free(patch_name);
    }
}

static char *ReadDMXConfig()
{
    // TODO: This should be chosen based on gamemode == commercial:

    int lumpnum = W_CheckNumForName("DMXGUS");

    if (lumpnum < 0)
    {
        lumpnum = W_GetNumForName("DMXGUSC");
    }

    size_t len  = W_LumpLength(lumpnum);
    char *data = zmalloc<char *>(len + 1, PU_STATIC, nullptr);
    W_ReadLump(lumpnum, data);

    data[len] = '\0';
    return data;
}

static bool WriteTimidityConfig(char *path, gus_config_t *config)
{
    FILE *fstream = fopen(path, "w");

    if (fstream == nullptr)
    {
        return false;
    }

    fprintf(fstream, "# Autogenerated Timidity config.\n\n");

    fprintf(fstream, "dir %s\n", g_gusconf_globals->gus_patch_path);

    fprintf(fstream, "\nbank 0\n\n");

    for (unsigned int i = 0; i < 128; ++i)
    {
        if (config->mapping[i] >= 0 && config->mapping[i] < MAX_INSTRUMENTS
            && config->patch_names[config->mapping[i]] != nullptr)
        {
            fprintf(fstream, "%u %s\n",
                i, config->patch_names[config->mapping[i]]);
        }
    }

    fprintf(fstream, "\ndrumset 0\n\n");

    for (unsigned int i = 128 + 35; i <= 128 + 81; ++i)
    {
        if (config->mapping[i] >= 0 && config->mapping[i] < MAX_INSTRUMENTS
            && config->patch_names[config->mapping[i]] != nullptr)
        {
            fprintf(fstream, "%u %s\n",
                i - 128, config->patch_names[config->mapping[i]]);
        }
    }

    fprintf(fstream, "\n");

    fclose(fstream);

    return true;
}

bool GUS_WriteConfig(char *path)
{
    if (!strcmp(g_gusconf_globals->gus_patch_path, ""))
    {
        printf("You haven't configured gus_patch_path.\n");
        printf("gus_patch_path needs to point to the location of "
               "your GUS patch set.\n"
               "To get a copy of the \"standard\" GUS patches, "
               "download a copy of dgguspat.zip.\n");

        return false;
    }

    gus_config_t config;
    char *dmxconf = ReadDMXConfig();
    ParseDMXConfig(dmxconf, &config);

    bool result = WriteTimidityConfig(path, &config);

    FreeDMXConfig(&config);
    Z_Free(dmxconf);

    return result;
}
