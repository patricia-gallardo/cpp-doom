/*

 Copyright(C) 2005-2014 Simon Howard

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 --

 Functions for presenting the information captured from the statistics
 buffer to a file.

 */

#include <cstdio>
#include <cstring>

#include <fmt/printf.h>

#include "d_player.hpp"
#include "d_mode.hpp"
#include "m_argv.hpp"

#include "statdump.hpp"

/* Par times for E1M1-E1M9. */
static const int doom1_par_times[] = {
  30,
  75,
  120,
  90,
  165,
  180,
  180,
  30,
  165,
};

/* Par times for MAP01-MAP09. */
static const int doom2_par_times[] = {
  30,
  90,
  120,
  120,
  90,
  150,
  120,
  120,
  270,
};

/* Player colors. */
static const char *player_colors[] = {
  "Green",
  "Indigo",
  "Brown",
  "Red"
};

// Array of end-of-level statistics that have been captured.

constexpr auto         MAX_CAPTURES = 32;
static wbstartstruct_t captured_stats[MAX_CAPTURES];
static int             num_captured_stats = 0;

static GameMission_t discovered_gamemission = none;

/* Try to work out whether this is a Doom 1 or Doom 2 game, by looking
 * at the episode and map, and the par times.  This is used to decide
 * how to format the level name.  Unfortunately, in some cases it is
 * impossible to determine whether this is Doom 1 or Doom 2. */

static void DiscoverGamemode(const wbstartstruct_t *stats, int num_stats) {
  if (discovered_gamemission != none) {
    return;
  }

  for (int i = 0; i < num_stats; ++i) {
    int level = stats[i].last;

    /* If episode 2, 3 or 4, this is Doom 1. */

    if (stats[i].epsd > 0) {
      discovered_gamemission = doom;
      return;
    }

    /* This is episode 1.  If this is level 10 or higher,
       it must be Doom 2. */

    if (level >= 9) {
      discovered_gamemission = doom2;
      return;
    }

    /* Try to work out if this is Doom 1 or Doom 2 by looking
       at the par time. */

    int partime = stats[i].partime;

    if (partime == doom1_par_times[level] * TICRATE
        && partime != doom2_par_times[level] * TICRATE) {
      discovered_gamemission = doom;
      return;
    }

    if (partime != doom1_par_times[level] * TICRATE
        && partime == doom2_par_times[level] * TICRATE) {
      discovered_gamemission = doom2;
      return;
    }
  }
}

/* Returns the number of players active in the given stats buffer. */

static int GetNumPlayers(const wbstartstruct_t *stats) {
  int num_players = 0;

  for (int i = 0; i < MAXPLAYERS; ++i) {
    if (stats->plyr[i].in) {
      ++num_players;
    }
  }

  return num_players;
}

static void PrintBanner(FILE *stream) {
  fmt::fprintf(stream, "===========================================\n");
}

static void PrintPercentage(FILE *stream, int amount, int total) {
  if (total == 0) {
    fmt::fprintf(stream, "0");
  } else {
    fmt::fprintf(stream, "%i / %i", amount, total);

    // statdump.exe is a 16-bit program, so very occasionally an
    // integer overflow can occur when doing this calculation with
    // a large value. Therefore, cast to short to give the same
    // output.

    fmt::fprintf(stream, " (%i%%)", static_cast<short>(amount * 100) / total);
  }
}

/* Display statistics for a single player. */

static void PrintPlayerStats(FILE *stream, const wbstartstruct_t *stats, int player_num) {
  const wbplayerstruct_t *player = &stats->plyr[player_num];

  fmt::fprintf(stream, "Player %i (%s):\n", player_num + 1, player_colors[player_num]);

  /* Kills percentage */

  fmt::fprintf(stream, "\tKills: ");
  PrintPercentage(stream, player->skills, stats->maxkills);
  fmt::fprintf(stream, "\n");

  /* Items percentage */

  fmt::fprintf(stream, "\tItems: ");
  PrintPercentage(stream, player->sitems, stats->maxitems);
  fmt::fprintf(stream, "\n");

  /* Secrets percentage */

  fmt::fprintf(stream, "\tSecrets: ");
  PrintPercentage(stream, player->ssecret, stats->maxsecret);
  fmt::fprintf(stream, "\n");
}

/* Frags table for multiplayer games. */

static void PrintFragsTable(FILE *stream, const wbstartstruct_t *stats) {
  fmt::fprintf(stream, "Frags:\n");

  /* Print header */

  fmt::fprintf(stream, "\t\t");

  for (int x = 0; x < MAXPLAYERS; ++x) {

    if (!stats->plyr[x].in) {
      continue;
    }

    fmt::fprintf(stream, "%s\t", player_colors[x]);
  }

  fmt::fprintf(stream, "\n");

  fmt::fprintf(stream, "\t\t-------------------------------- VICTIMS\n");

  /* Print table */

  for (int y = 0; y < MAXPLAYERS; ++y) {
    if (!stats->plyr[y].in) {
      continue;
    }

    fmt::fprintf(stream, "\t%s\t|", player_colors[y]);

    for (int x = 0; x < MAXPLAYERS; ++x) {
      if (!stats->plyr[x].in) {
        continue;
      }

      fmt::fprintf(stream, "%i\t", stats->plyr[y].frags[x]);
    }

    fmt::fprintf(stream, "\n");
  }

  fmt::fprintf(stream, "\t\t|\n");
  fmt::fprintf(stream, "\t     KILLERS\n");
}

/* Displays the level name: MAPxy or ExMy, depending on game mode. */

static void PrintLevelName(FILE *stream, int episode, int level) {
  PrintBanner(stream);

  switch (discovered_gamemission) {

  case doom:
    fmt::fprintf(stream, "E%iM%i\n", episode + 1, level + 1);
    break;
  case doom2:
    fmt::fprintf(stream, "MAP%02i\n", level + 1);
    break;
  default:
  case none:
    fmt::fprintf(stream, "E%iM%i / MAP%02i\n", episode + 1, level + 1, level + 1);
    break;
  }

  PrintBanner(stream);
}

/* Print details of a statistics buffer to the given file. */

static void PrintStats(FILE *stream, const wbstartstruct_t *stats) {
  PrintLevelName(stream, stats->epsd, stats->last);
  fmt::fprintf(stream, "\n");

  auto leveltime = static_cast<short>(stats->plyr[0].stime / TICRATE);
  auto partime   = static_cast<short>(stats->partime / TICRATE);
  fmt::fprintf(stream, "Time: %i:%02i", leveltime / 60, leveltime % 60);
  fmt::fprintf(stream, " (par: %i:%02i)\n", partime / 60, partime % 60);
  fmt::fprintf(stream, "\n");

  for (int i = 0; i < MAXPLAYERS; ++i) {
    if (stats->plyr[i].in) {
      PrintPlayerStats(stream, stats, i);
    }
  }

  if (GetNumPlayers(stats) >= 2) {
    PrintFragsTable(stream, stats);
  }

  fmt::fprintf(stream, "\n");
}

void StatCopy(const wbstartstruct_t *stats) {
  if (M_ParmExists("-statdump") && num_captured_stats < MAX_CAPTURES) {
    std::memcpy(&captured_stats[num_captured_stats], stats, sizeof(wbstartstruct_t));
    ++num_captured_stats;
  }
}

void StatDump() {
  //!
  // @category compat
  // @arg <filename>
  //
  // Dump statistics information to the specified file on the levels
  // that were played. The output from this option matches the output
  // from statdump.exe (see ctrlapi.zip in the /idgames archive).
  //

  int index_of_arg = M_CheckParmWithArgs("-statdump", 1);

  if (index_of_arg > 0) {
    fmt::printf("Statistics captured for %i level(s)\n", num_captured_stats);

    // We actually know what the real gamemission is, but this has
    // to match the output from statdump.exe.

    DiscoverGamemode(captured_stats, num_captured_stats);

    // Allow "-" as output file, for stdout.

    FILE *dumpfile = nullptr;

    char *filename = myargv[index_of_arg + 1];

    if (strcmp(filename, "-") != 0) {
      dumpfile = fopen(filename, "w");
    } else {
      dumpfile = stdout;
    }

    for (int i = 0; i < num_captured_stats; ++i) {
      PrintStats(dumpfile, &captured_stats[i]);
    }

    if (dumpfile != stdout) {
      fclose(dumpfile);
    }
  }
}
