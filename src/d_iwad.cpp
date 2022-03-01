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
//     Search for and locate an IWAD file, and initialize according
//     to the IWAD type.
//

#include <array>
#include <cstdlib>
#include <cstring>

#include "d_iwad.hpp"
#include "deh_str.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_misc.hpp"
#include "memory.hpp"
#ifdef WIN32
#include "m_config.hpp"
#endif

static const iwad_t iwads[] = {
  {"doom2.wad",      doom2,     commercial, "Doom II"                        },
  { "plutonia.wad",  pack_plut, commercial, "Final Doom: Plutonia Experiment"},
  { "tnt.wad",       pack_tnt,  commercial, "Final Doom: TNT: Evilution"     },
  { "doom.wad",      doom,      retail,     "Doom"                           },
  { "doom1.wad",     doom,      shareware,  "Doom Shareware"                 },
  { "chex.wad",      pack_chex, retail,     "Chex Quest"                     },
  { "hacx.wad",      pack_hacx, commercial, "Hacx"                           },
  { "freedoom2.wad", doom2,     commercial, "Freedoom: Phase 2"              },
  { "freedoom1.wad", doom,      retail,     "Freedoom: Phase 1"              },
  { "freedm.wad",    doom2,     commercial, "FreeDM"                         },
  { "heretic.wad",   heretic,   retail,     "Heretic"                        },
  { "heretic1.wad",  heretic,   shareware,  "Heretic Shareware"              },
  { "hexen.wad",     hexen,     commercial, "Hexen"                          },
 //{ "strife0.wad",  strife,    commercial, "Strife" }, // haleyjd: STRIFE-FIXME
  { "strife1.wad",   strife,    commercial, "Strife"                         },
};

bool D_IsIWADName(cstring_view name) {
  for (const auto & iwad : iwads) {
    if (iequals(name, iwad.name)) {
      return true;
    }
  }

  return false;
}

// Array of locations to search for IWAD files
//
// "128 IWAD search directories should be enough for anybody".

constexpr auto MAX_IWAD_DIRS = 128;

static bool   iwad_dirs_built = false;
static char * iwad_dirs[MAX_IWAD_DIRS];
static int    num_iwad_dirs = 0;

static void AddIWADDir(char * dir) {
  if (num_iwad_dirs < MAX_IWAD_DIRS) {
    iwad_dirs[num_iwad_dirs] = dir;
    ++num_iwad_dirs;
  }
}

// This is Windows-specific code that automatically finds the location
// of installed IWAD files.  The registry is inspected to find special
// keys installed by the Windows installers for various CD versions
// of Doom.  From these keys we can deduce where to find an IWAD.

#if defined(_WIN32) && !defined(_WIN32_WCE)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// intentional newline to avoid reordering by Clang Format

struct registry_value_t {
  HKEY   root;
  char * path;
  char * value;
};

#define UNINSTALLER_STRING "\\uninstl.exe /S "

// Keys installed by the various CD editions.  These are actually the
// commands to invoke the uninstaller and look like this:
//
// C:\Program Files\Path\uninstl.exe /S C:\Program Files\Path
//
// With some munging we can find where Doom was installed.

// [AlexMax] From the persepctive of a 64-bit executable, 32-bit registry
// keys are located in a different spot.
#if _WIN64
#define SOFTWARE_KEY "Software\\Wow6432Node"
#else
#define SOFTWARE_KEY "Software"
#endif

static registry_value_t uninstall_values[] = {
  // Ultimate Doom, CD version (Depths of Doom trilogy)

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
                                      "Uninstall\\Ultimate Doom for Windows 95"),
   const_cast<char *>("UninstallString"),
   },

 // Doom II, CD version (Depths of Doom trilogy)

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
                                      "Uninstall\\Doom II for Windows 95"),
   const_cast<char *>("UninstallString"),
   },

 // Final Doom

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
                                      "Uninstall\\Final Doom for Windows 95"),
   const_cast<char *>("UninstallString"),
   },

 // Shareware version

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
                                      "Uninstall\\Doom Shareware for Windows 95"),
   const_cast<char *>("UninstallString"),
   },
};

// Values installed by the GOG.com and Collector's Edition versions

static registry_value_t root_path_keys[] = {
  // Doom Collector's Edition

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\Activision\\DOOM Collector's Edition\\v1.0"),
   const_cast<char *>("INSTALLPATH"),
   },

 // Doom II

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\GOG.com\\Games\\1435848814"),
   const_cast<char *>("PATH"),
   },

 // Doom 3: BFG Edition

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\GOG.com\\Games\\1135892318"),
   const_cast<char *>("PATH"),
   },

 // Final Doom

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\GOG.com\\Games\\1435848742"),
   const_cast<char *>("PATH"),
   },

 // Ultimate Doom

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\GOG.com\\Games\\1435827232"),
   const_cast<char *>("PATH"),
   },

 // Strife: Veteran Edition

  {
   HKEY_LOCAL_MACHINE,
   const_cast<char *>(SOFTWARE_KEY "\\GOG.com\\Games\\1432899949"),
   const_cast<char *>("PATH"),
   },
};

// Subdirectories of the above install path, where IWADs are installed.

static char * root_path_subdirs[] = {
  const_cast<char *>("."),
  const_cast<char *>("Doom2"),
  const_cast<char *>("Final Doom"),
  const_cast<char *>("Ultimate Doom"),
  const_cast<char *>("Plutonia"),
  const_cast<char *>("TNT"),
  const_cast<char *>("base\\wads"),
};

// Location where Steam is installed

static registry_value_t steam_install_location = {
  HKEY_LOCAL_MACHINE,
  const_cast<char *>(SOFTWARE_KEY "\\Valve\\Steam"),
  const_cast<char *>("InstallPath"),
};

// Subdirs of the steam install directory where IWADs are found

static char * steam_install_subdirs[] = {
  const_cast<char *>(R"(steamapps\common\doom 2\base)"),
  const_cast<char *>(R"(steamapps\common\final doom\base)"),
  const_cast<char *>(R"(steamapps\common\ultimate doom\base)"),
  const_cast<char *>(R"(steamapps\common\heretic shadow of the serpent riders\base)"),
  const_cast<char *>(R"(steamapps\common\hexen\base)"),
  const_cast<char *>(R"(steamapps\common\hexen deathkings of the dark citadel\base)"),

  // From Doom 3: BFG Edition:

  const_cast<char *>(R"(steamapps\common\DOOM 3 BFG Edition\base\wads)"),

  // From Strife: Veteran Edition:

  const_cast<char *>("steamapps\\common\\Strife"),
};

#define STEAM_BFG_GUS_PATCHES \
  "steamapps\\common\\DOOM 3 BFG Edition\\base\\classicmusic\\instruments"

static char * GetRegistryString(registry_value_t * reg_val) {
  HKEY  key     = nullptr;
  DWORD len     = 0;
  DWORD valtype = 0;

  // Open the key (directory where the value is stored)

  if (RegOpenKeyEx(reg_val->root, reg_val->path, 0, KEY_READ, &key)
      != ERROR_SUCCESS) {
    return nullptr;
  }

  char * result = nullptr;

  // Find the type and length of the string, and only accept strings.

  if (RegQueryValueEx(key, reg_val->value, nullptr, &valtype, nullptr, &len)
          == ERROR_SUCCESS
      && valtype == REG_SZ) {
    // Allocate a buffer for the value and read the value

    result = static_cast<char *>(malloc(len + 1));

    if (RegQueryValueEx(key, reg_val->value, nullptr, &valtype, reinterpret_cast<unsigned char *>(result), &len)
        != ERROR_SUCCESS) {
      free(result);
      result = nullptr;
    } else {
      // Ensure the value is null-terminated
      result[len] = '\0';
    }
  }

  // Close the key

  RegCloseKey(key);

  return result;
}

// Check for the uninstall strings from the CD versions

static void CheckUninstallStrings() {
  for (auto & uninstall_value : uninstall_values) {
    char * val = GetRegistryString(&uninstall_value);

    if (val == nullptr) {
      continue;
    }

    char * unstr = strstr(val, UNINSTALLER_STRING);

    if (unstr == nullptr) {
      free(val);
    } else {
      char * path = unstr + strlen(UNINSTALLER_STRING);

      AddIWADDir(path);
    }
  }
}

// Check for GOG.com and Doom: Collector's Edition

static void CheckInstallRootPaths() {
  for (auto & root_path_key : root_path_keys) {
    char * install_path = GetRegistryString(&root_path_key);

    if (install_path == nullptr) {
      continue;
    }

    for (auto & root_path_subdir : root_path_subdirs) {
      char * subpath = M_StringJoin(install_path, DIR_SEPARATOR_S, root_path_subdir, nullptr);
      AddIWADDir(subpath);
    }

    free(install_path);
  }
}

// Check for Doom downloaded via Steam

static void CheckSteamEdition() {
  char * install_path = GetRegistryString(&steam_install_location);

  if (install_path == nullptr) {
    return;
  }

  for (auto & steam_install_subdir : steam_install_subdirs) {
    char * subpath = M_StringJoin(install_path, DIR_SEPARATOR_S, steam_install_subdir, nullptr);
    AddIWADDir(subpath);
  }

  free(install_path);
}

// The BFG edition ships with a full set of GUS patches. If we find them,
// we can autoconfigure to use them.

static void CheckSteamGUSPatches() {
  // Already configured? Don't stomp on the user's choices.
  const char * current_path = M_GetStringVariable("gus_patch_path");
  if (current_path != nullptr && strlen(current_path) > 0) {
    return;
  }

  char * install_path = GetRegistryString(&steam_install_location);

  if (install_path == nullptr) {
    return;
  }

  char * patch_path      = M_StringJoin(install_path, "\\", STEAM_BFG_GUS_PATCHES, nullptr);
  char * test_patch_path = M_StringJoin(patch_path, "\\ACBASS.PAT", nullptr);

  // Does acbass.pat exist? If so, then set gus_patch_path.
  if (M_FileExists(test_patch_path)) {
    M_SetVariable("gus_patch_path", patch_path);
  }

  free(test_patch_path);
  free(patch_path);
  free(install_path);
}

// Default install directories for DOS Doom

static void CheckDOSDefaults() {
  // These are the default install directories used by the deice
  // installer program:

  AddIWADDir(const_cast<char *>("\\doom2"));    // Doom II
  AddIWADDir(const_cast<char *>("\\plutonia")); // Final Doom
  AddIWADDir(const_cast<char *>("\\tnt"));
  AddIWADDir(const_cast<char *>("\\doom_se")); // Ultimate Doom
  AddIWADDir(const_cast<char *>("\\doom"));    // Shareware / Registered Doom
  AddIWADDir(const_cast<char *>("\\dooms"));   // Shareware versions
  AddIWADDir(const_cast<char *>("\\doomsw"));

  AddIWADDir(const_cast<char *>("\\heretic"));  // Heretic
  AddIWADDir(const_cast<char *>("\\hrtic_se")); // Heretic Shareware from Quake disc

  AddIWADDir(const_cast<char *>("\\hexen"));   // Hexen
  AddIWADDir(const_cast<char *>("\\hexendk")); // Hexen Deathkings of the Dark Citadel

  AddIWADDir(const_cast<char *>("\\strife")); // Strife
}

#endif

// Returns true if the specified path is a path to a file
// of the specified name.

static bool DirIsFile(cstring_view path, const char * filename) {
  return strchr(path.c_str(), DIR_SEPARATOR) != nullptr
         && iequals(M_BaseName(path), filename);
}

// Check if the specified directory contains the specified IWAD
// file, returning the full path to the IWAD if found, or NULL
// if not found.

static char * CheckDirectoryHasIWAD(cstring_view dir, const char * iwadname) {
  // As a special case, the "directory" may refer directly to an
  // IWAD file if the path comes from DOOMWADDIR or DOOMWADPATH.

  char * probe = M_FileCaseExists(dir);
  if (DirIsFile(dir, iwadname) && probe != nullptr) {
    return probe;
  }

  // Construct the full path to the IWAD if it is located in
  // this directory, and check if it exists.
  char * filename = nullptr;

  if (!strcmp(dir.c_str(), ".")) {
    filename = M_StringDuplicate(iwadname);
  } else {
    filename = M_StringJoin(dir.c_str(), DIR_SEPARATOR_S, iwadname, nullptr);
  }

  free(probe);
  probe = M_FileCaseExists(filename);
  free(filename);
  if (probe != nullptr) {
    return probe;
  }

  return nullptr;
}

// Search a directory to try to find an IWAD
// Returns the location of the IWAD if found, otherwise NULL.

static char * SearchDirectoryForIWAD(cstring_view dir, int mask, GameMission_t * mission) {
  for (const auto & iwad : iwads) {
    if (((1 << iwad.mission) & mask) == 0) {
      continue;
    }

    char * filename = CheckDirectoryHasIWAD(dir, DEH_String(iwad.name));

    if (filename != nullptr) {
      *mission = iwad.mission;

      return filename;
    }
  }

  return nullptr;
}

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.

static GameMission_t IdentifyIWADByName(cstring_view name, int mask) {
  name                  = M_BaseName(name);
  GameMission_t mission = none;

  for (const auto & iwad : iwads) {
    // Check if the filename is this IWAD name.

    // Only use supported missions:

    if (((1 << iwad.mission) & mask) == 0)
      continue;

    // Check if it ends in this IWAD name.

    if (iequals(name, iwad.name)) {
      mission = iwad.mission;
      break;
    }
  }

  return mission;
}

// Add IWAD directories parsed from splitting a path string containing
// paths separated by PATH_SEPARATOR. 'suffix' is a string to concatenate
// to the end of the paths before adding them.
static void AddIWADPath(cstring_view path, const char * suffix) {
  char * dup_path = M_StringDuplicate(path);

  // Split into individual dirs within the list.
  char * left = dup_path;

  for (;;) {
    char * p = strchr(left, PATH_SEPARATOR);
    if (p != nullptr) {
      // Break at the separator and use the left hand side
      // as another iwad dir
      *p = '\0';

      AddIWADDir(M_StringJoin(left, suffix, nullptr));
      left = p + 1;
    } else {
      break;
    }
  }

  AddIWADDir(M_StringJoin(left, suffix, nullptr));

  free(dup_path);
}

#ifndef _WIN32
// Add standard directories where IWADs are located on Unix systems.
// To respect the freedesktop.org specification we support overriding
// using standard environment variables. See the XDG Base Directory
// Specification:
// <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
static void AddXdgDirs() {
  // Quote:
  // > $XDG_DATA_HOME defines the base directory relative to which
  // > user specific data files should be stored. If $XDG_DATA_HOME
  // > is either not set or empty, a default equal to
  // > $HOME/.local/share should be used.
  char * env     = getenv("XDG_DATA_HOME");
  char * tmp_env = nullptr;

  if (env == nullptr) {
    char * homedir = getenv("HOME");
    if (homedir == nullptr) {
      homedir = const_cast<char *>("/");
    }

    tmp_env = M_StringJoin(homedir, "/.local/share", nullptr);
    env     = tmp_env;
  }

  // We support $XDG_DATA_HOME/games/doom (which will usually be
  // ~/.local/share/games/doom) as a user-writeable extension to
  // the usual /usr/share/games/doom location.
  AddIWADDir(M_StringJoin(env, "/games/doom", nullptr));
  free(tmp_env);

  // Quote:
  // > $XDG_DATA_DIRS defines the preference-ordered set of base
  // > directories to search for data files in addition to the
  // > $XDG_DATA_HOME base directory. The directories in $XDG_DATA_DIRS
  // > should be seperated with a colon ':'.
  // >
  // > If $XDG_DATA_DIRS is either not set or empty, a value equal to
  // > /usr/local/share/:/usr/share/ should be used.
  env = getenv("XDG_DATA_DIRS");
  if (env == nullptr) {
    // (Trailing / omitted from paths, as it is added below)
    env = const_cast<char *>("/usr/local/share:/usr/share");
  }

  // The "standard" location for IWADs on Unix that is supported by most
  // source ports is /usr/share/games/doom - we support this through the
  // XDG_DATA_DIRS mechanism, through which it can be overridden.
  AddIWADPath(env, "/games/doom");

  // The convention set by RBDOOM-3-BFG is to install Doom 3: BFG
  // Edition into this directory, under which includes the Doom
  // Classic WADs.
  AddIWADPath(env, "/games/doom3bfg/base/wads");
}

#ifndef __MACOSX__
// Steam on Linux allows installing some select Windows games,
// including the classic Doom series (running DOSBox via Wine).  We
// could parse *.vdf files to more accurately detect installation
// locations, but the defaults are likely to be good enough for just
// about everyone.
static void AddSteamDirs() {
  char * homedir = getenv("HOME");
  if (homedir == nullptr) {
    homedir = const_cast<char *>("/");
  }
  char * steampath = M_StringJoin(homedir, "/.steam/root/steamapps/common", nullptr);

  AddIWADPath(steampath, "/Doom 2/base");
  AddIWADPath(steampath, "/Master Levels of Doom/doom2");
  AddIWADPath(steampath, "/Ultimate Doom/base");
  AddIWADPath(steampath, "/Final Doom/base");
  AddIWADPath(steampath, "/DOOM 3 BFG Edition/base/wads");
  AddIWADPath(steampath, "/Heretic Shadow of the Serpent Riders/base");
  AddIWADPath(steampath, "/Hexen/base");
  AddIWADPath(steampath, "/Hexen Deathkings of the Dark Citadel/base");
  AddIWADPath(steampath, "/Strife");
  free(steampath);
}
#endif // __MACOSX__
#endif // !_WIN32

//
// Build a list of IWAD files
//

static void BuildIWADDirList() {
  if (iwad_dirs_built) {
    return;
  }

  // Look in the current directory.  Doom always does this.
  AddIWADDir(const_cast<char *>("."));

  // Next check the directory where the executable is located. This might
  // be different from the current directory.
  AddIWADDir(M_DirName(myargv[0]));

  // Add DOOMWADDIR if it is in the environment
  char * env = getenv("DOOMWADDIR");
  if (env != nullptr) {
    AddIWADDir(env);
  }

  // Add dirs from DOOMWADPATH:
  env = getenv("DOOMWADPATH");
  if (env != nullptr) {
    AddIWADPath(env, "");
  }

#ifdef _WIN32

  // Search the registry and find where IWADs have been installed.

  CheckUninstallStrings();
  CheckInstallRootPaths();
  CheckSteamEdition();
  CheckDOSDefaults();

  // Check for GUS patches installed with the BFG edition!

  CheckSteamGUSPatches();

#else
  AddXdgDirs();
#ifndef __MACOSX__
  AddSteamDirs();
#endif
#endif

  // Don't run this function again.

  iwad_dirs_built = true;
}

//
// Searches WAD search paths for an WAD with a specific filename.
//

char * D_FindWADByName(cstring_view name) {
  // Absolute path?

  char * probe = M_FileCaseExists(name);
  if (probe != nullptr) {
    return probe;
  }

  BuildIWADDirList();

  // Search through all IWAD paths for a file with the given name.

  for (int i = 0; i < num_iwad_dirs; ++i) {
    // As a special case, if this is in DOOMWADDIR or DOOMWADPATH,
    // the "directory" may actually refer directly to an IWAD
    // file.

    probe = M_FileCaseExists(iwad_dirs[i]);
    if (DirIsFile(iwad_dirs[i], name.c_str()) && probe != nullptr) {
      return probe;
    }
    free(probe);

    // Construct a string for the full path

    char * path = M_StringJoin(iwad_dirs[i], DIR_SEPARATOR_S, name, nullptr);

    probe = M_FileCaseExists(path);
    if (probe != nullptr) {
      return probe;
    }

    free(path);
  }

  // File not found

  return nullptr;
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or returns a copy of the filename
// if not found.
//

char * D_TryFindWADByName(cstring_view filename) {
  char * result = D_FindWADByName(filename);

  if (result != nullptr) {
    return result;
  } else {
    return M_StringDuplicate(filename);
  }
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//

char * D_FindIWAD(int mask, GameMission_t * mission) {
  // Check for the -iwad parameter

  //!
  // Specify an IWAD file to use.
  //
  // @arg <file>
  //

  int    iwadparm = M_CheckParmWithArgs("-iwad", 1);
  char * result   = nullptr;
  if (iwadparm) {
    // Search through IWAD dirs for an IWAD with the given name.

    char * iwadfile = myargv[iwadparm + 1];

    result = D_FindWADByName(iwadfile);

    if (result == nullptr) {
      I_Error("IWAD file '%s' not found!", iwadfile);
    }

    *mission = IdentifyIWADByName(result, mask);
  } else {
    // Search through the list and look for an IWAD

    result = nullptr;

    BuildIWADDirList();

    for (int i = 0; result == nullptr && i < num_iwad_dirs; ++i) {
      result = SearchDirectoryForIWAD(iwad_dirs[i], mask, mission);
    }
  }

  return result;
}

// Find all IWADs in the IWAD search path matching the given mask.

const iwad_t ** D_FindAllIWADs(int mask) {
  auto result = create_struct<iwad_t const * [std::size(iwads) + 1]>();
  //    result = malloc(sizeof(iwad_t *) * (std::size(iwads) + 1));
  int result_len = 0;

  // Try to find all IWADs

  for (const auto & iwad : iwads) {
    if (((1 << iwad.mission) & mask) == 0) {
      continue;
    }

    char * filename = D_FindWADByName(iwad.name);

    if (filename != nullptr) {
      result[result_len] = &iwad;
      ++result_len;
    }
  }

  // End of list

  result[result_len] = nullptr;

  return result;
}

//
// Get the IWAD name used for savegames.
//

const char * D_SaveGameIWADName(GameMission_t gamemission) {
  // Determine the IWAD name to use for savegames.
  // This determines the directory the savegame files get put into.
  //
  // Note that we match on gamemission rather than on IWAD name.
  // This ensures that doom1.wad and doom.wad saves are stored
  // in the same place.

  for (const auto & iwad : iwads) {
    if (gamemission == iwad.mission) {
      return iwad.name;
    }
  }

  // Default fallback:

  return "unknown.wad";
}

const char * D_SuggestIWADName(GameMission_t mission, GameMode_t mode) {
  for (const auto & iwad : iwads) {
    if (iwad.mission == mission && iwad.mode == mode) {
      return iwad.name;
    }
  }

  return "unknown.wad";
}

const char * D_SuggestGameName(GameMission_t mission, GameMode_t mode) {
  for (const auto & iwad : iwads) {
    if (iwad.mission == mission
        && (mode == indetermined || iwad.mode == mode)) {
      return iwad.description;
    }
  }

  return "Unknown game?";
}
