# Crispy Cpp Doom
[![Crispy Doom Icon](https://www.chocolate-doom.org/wiki/images/b/be/Crispy-doom.png)](https://github.com/fabiangreffrath/crispy-doom)

[![Top Language](https://img.shields.io/github/languages/top/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom)
[![Code Size](https://img.shields.io/github/languages/code-size/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom)
[![License](https://img.shields.io/github/license/fabiangreffrath/crispy-doom.svg?style=flat&logo=gnu)](https://github.com/fabiangreffrath/crispy-doom/blob/master/COPYING.md)
[![Release](https://img.shields.io/github/release/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Release Date](https://img.shields.io/github/release-date/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Downloads](https://img.shields.io/github/downloads/fabiangreffrath/crispy-doom/latest/total.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Commits](https://img.shields.io/github/commits-since/fabiangreffrath/crispy-doom/latest.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/commits/master)
[![Last Commit](https://img.shields.io/github/last-commit/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/commits/master)
[![Travis Build Status](https://img.shields.io/travis/com/fabiangreffrath/crispy-doom.svg?style=flat&logo=travis)](https://travis-ci.com/fabiangreffrath/crispy-doom/)

Crispy Doom is a limit-removing enhanced-resolution Doom source port based on [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom).

Its name means that its internal 640x400 resolution looks "crisp" and is also a [slight reference](http://www.mathsisfun.com/recipie.html) to its origin.

## Synopsis

Crispy Doom is a friendly fork of [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom) that provides a higher display resolution, removes the [static limits](https://doomwiki.org/wiki/Static_limits) of the Doom engine and offers further optional visual, tactical and physical enhancements while remaining entirely config file, savegame, netplay and demo compatible with the original.

## Objectives and features

Crispy Doom is a source port that aims to provide a faithful Doom gaming experience while also featuring some user-requested improvements and enhancements. It is forked off of Chocolate Doom to take advantage of its free and open-source code base, portability, accuracy and compatibility with Vanilla Doom.

Its core features are:

 * Enhanced 640x400 display resolution, with the original 320x200 resolution still available in the "High Resolution Rendering: Off" mode.
 * Widescreen rendering for using all the available horizontal space of screens with aspect ratios up to 24:9.
 * Uncapped rendering framerate with interpolation and optional vertical synchronization (VSync) with the screen refresh rate.
 * Intermediate gamma correction levels (0.5, 1.5, 2.5 and 3.5).
 * Removal of all static engine limits, or at least raising of the less crucial ones.
 * Full support for the "Doom Classic" WADs shipped with the "Doom 3: BFG Edition", especially the "No Rest For The Living" episode shipped in the NERVE.WAD file.
 * Support for all versions of John Romero's Episode 5: Sigil for Ultimate Doom.

Furthermore, the following optional user-visible and audible features are available:

 * Jumping.
 * Free vertical looking, including mouse look and vertical aiming.
 * Aiming support by a crosshair that may get directly rendered into the game world.
 * A new minimal Crispy HUD, displaying only the status bar numbers.
 * Clean Screenshot feature, enabling to take screenshots without HUD elements and even without status bar numbers and weapon sprites at higher screen sizes.
 * Colorized status bar numbers, HUD texts and blood sprites for certain monsters.
 * Translucency for certain sprites and status bar elements in the Crispy HUD.
 * Randomly mirrored death animations and corpse sprites.
 * Command line options to allow for playing with flipped player weapon sprites and/or entirely flipped level geometry.
 * Players may walk over or under monsters and hanging corpses.
 * Centered Weapons when firing, weapon recoil thrust and pitch.
 * Reports whenever a secret is revealed.
 * Level statistics and extended coloring in the Automap.
 * Playing sounds in full length, and misc. other sound fixes.
 * Demo recording and/or playback timers and progress bar.
 * Demo continue and take-over features, handing controls over to the player when demo playback is finished or interrupted.

Most of these features are disabled by default and need to get enabled either in the in-game "Crispness" menu, in the crispy-doom-setup tool or as command line parameters. They are implemented in a way that preserves demo-compatibility with Vanilla Doom and network game compatibility with Chocolate Doom. Furthermore, Crispy Doom's savegames and config files are compatible, though not identical (see the [Compatibility section in the Wiki](https://github.com/fabiangreffrath/crispy-doom/wiki/Compatibility)), to Vanilla Doom's.

Crispy Doom strives for maximum compatibility with all "limit-removing Vanilla" maps -- but not Boom or ZDoom maps. More specifically, Crispy Doom supports some select advanced features such as [ANIMATED](https://doomwiki.org/wiki/ANIMATED) and [SWITCHES](https://doomwiki.org/wiki/SWITCHES) lumps, MBF sky transfers, SMMU swirling flats and [MUSINFO](https://doomwiki.org/wiki/MUSINFO) -- but neither generalized linedef and sector types nor DECORATE and MAPINFO.

Many additional less user-visible features have been implemented, e.g. fixed engine limitations and crashes, fixed rendering bugs, fixed harmless game logic bugs, full support for DEHACKED files and lumps in BEX format, additional and improved cheat codes, an improved Automap, and many more! Due to the extra DEHACKED states added from [MBF](https://doomwiki.org/wiki/MBF), Crispy Doom supports [enhancer](https://www.doomworld.com/forum/topic/84859-black-ops-smooth-weapons-dehacked-mod) [mods](https://www.doomworld.com/forum/topic/85991-smoothed-smooth-monsters-for-doom-retro-and-crispy-doom) that can make the gameplay even more pleasing to the eyes. For a detailed list of features and changes please refer to the release notes below.

### New controls (with default bindings)

 * Move Forward (alt.) <kbd>W</kbd>
 * Move Backward (alt.) <kbd>S</kbd>
 * Strafe Left (alt.) <kbd>A</kbd>
 * Strafe Right (alt.) <kbd>D</kbd>
 * Jump (bindable to joystick and mouse buttons as well) <kbd>/</kbd> (as in Hexen and Strife)
 * Quick Reverse (bindable to mouse buttons as well)
 * Mouse Look (bindable to mouse buttons or permanent)
 * Look up (bindable to joystick axes as well) <kbd>PgDn</kbd> (as in Heretic)
 * Look down (bindable to joystick axes as well) <kbd>Del</kbd> (as in Heretic)
 * Center view <kbd>End</kbd> (as in Heretic)
 * Toggle always run <kbd>&#8682;</kbd>
 * Toggle vertical mouse movement (new in 5.4)
 * Delete savegame <kbd>Del</kbd>
 * Go to next level
 * Reload current level
 * Save a clean screenshot
 * Toggle Automap overlay mode <kbd>O</kbd>
 * Toggle Automap rotate mode <kbd>R</kbd>
 * Resurrect from savegame (single player mode only) "Run" + "Use"

### New command line parameters

 * `-dm3` specifies the Deathmatch 3.0 rules (weapons stay, items respawn) for netgames (since 4.1).
 * `-episode 1` launches Hell on Earth and `-episode 2` launches No Rest for the Living episode if the Doom 2 IWAD shipped with the Doom 3: BFG Edition is used.
 * `-warp 1a` warps to the secret level E1M10: Sewers of XBox Doom IWAD (since 2.3).
 * `-mergedump <file>` merges the PWAD file(s) given on the command line with the IWAD file and writes the resulting data into the `<file>` given as argument. May be considered as a replacement for the `DEUSF.EXE` tool (since 2.3).
 * `-lumpdump` dumps raw content of a lump into a file (since 5.7).
 * `-blockmap` forces a (re-)building of the BLOCKMAP lumps for loaded maps (since 2.3).
 * `-playdemo demoname -warp N` plays back fast-forward up to the requested map (since 3.0).
 * `-loadgame N -record demoname` and `-loadgame N -playdemo demoname` allow to record and play demos starting from a savegame instead of the level start (since 4.0).
 * `-playdemo demoname1 -record demoname2` plays back fast-forward until the end of demoname1 and continues recording as demoname2 (new in 5.5).
 * `-fliplevels` loads mirrored versions of the maps (this was the default on April 1st up to version 5.0).
 * `-flipweapons` flips the player's weapons (new in 5.3).

### New cheat codes

 * `TNTWEAP` followed by a weapon number gives or removes this weapon (8 = Chainsaw, 9 = SSG). `TNTWEAP0` takes away all weapons and ammo except for the pistol and 50 bullets. Try to load Doom 1 with `DOOM2.WAD` as a PWAD and type `TNTWEAP9` to play the SSG in Doom 1.
 * `TNTEM`, `KILLEM` or `FHHALL` kill all monsters on the current map (and disables all cube spitters).
 * `SPECHITS` triggers all [Linedef actions](https://doomwiki.org/wiki/Linedef_type) on a map at once, no matter if they are enabled by pushing, walking over or shooting or whether they require a key or not. It also triggers all boss monster and Commander Keen actions if possible.
 * `NOTARGET` or `FHSHH` toggle deaf and blind monsters that do not act until attacked.
 * `TNTHOM` toggles the flashing [HOM](https://doomwiki.org/wiki/Hall_of_mirrors_effect) indicator (disabled by default).
 * `SHOWFPS` or `IDRATE` toggle printing the FPS in the upper right corner.
 * `NOMOMENTUM` toggles a debug aid for pixel-perfect positioning on a map (not recommended to use in-game).
 * `GOOBERS` triggers an easter egg, i.e. an "homage to an old friend". ;-)
 * `IDBEHOLD0` disables all currently active power-ups (since 2.2).
 * `IDCLEV00` restarts the current level (since 2.0).
 * `IDMUS00` restarts the current music (new in 5.1).
 * `VERSION` shows the engine version, build date and SDL version (new in 5.1).
 * `SKILL` shows the current skill level (new in 5.5.2).

## Download

Binaries for Windows XP / Vista / 7 / 8.1 / 10 (32-bit binaries compatible with both x86 and x64 editions) are available here:
https://github.com/fabiangreffrath/crispy-doom/releases/download/crispy-doom-5.7.2/crispy-doom-5.7.2-win32.zip

Daily builds of Crispy Doom can be found here:
http://latest.chocolate-doom.org/

Crispy Doom can play nearly all variants of Doom. If you don't own any, you may download the [Shareware version of Doom](http://cdn.debian.net/debian/pool/non-free/d/doom-wad-shareware/doom-wad-shareware_1.9.fixed.orig.tar.gz), extract it and copy the DOOM1.WAD file into your Crispy Doom directory. Alternatively, you may want to play Crispy Doom with [Freedoom](https://www.chocolate-doom.org/wiki/index.php/Freedoom) and a MegaWAD.

### Sources
[![Open Hub](https://www.openhub.net/p/crispy-doom/widgets/project_thin_badge?style=flat&format=gif)](https://www.openhub.net/p/crispy-doom)

The Crispy Doom source code is available at GitHub: https://github.com/fabiangreffrath/crispy-doom.
It can be [downloaded in either ZIP or TAR.GZ format](https://github.com/fabiangreffrath/crispy-doom/releases) 
or cloned via

```
 git clone https://github.com/fabiangreffrath/crispy-doom.git
```

Brief instructions to set up a build system on Windows can be found [in the Crispy Doom Wiki](https://github.com/fabiangreffrath/crispy-doom/wiki/Building-on-Windows). A much more detailed guide is provided [in the Chocolate Doom Wiki](https://www.chocolate-doom.org/wiki/index.php/Building_Chocolate_Doom_on_Windows), but applies to Crispy Doom as well for most parts.

Compilation on Debian systems (Debian 10 "buster" or later) should be as simple as

```
 sudo apt install build-essential automake git
 sudo apt build-dep crispy-doom
```

to install the prerequisites and then

```
 cd crispy-doom
 autoreconf -fiv
 ./configure
 make
```

After successful compilation the resulting binaries can be found in the `src/` directory.

## News

### Crispy Doom 5.7.2

Crispy Doom 5.7.2 has been released on March 13, 2020 to introduce the Compact HUD for widescreen rendering mode and bring some general improvements and fixes.

**Features**

 * A new "Compact HUD" mode has been introduced for widescreen rendering mode which leaves the status bar widgets at the same position as in regular rendering mode instead of moving them to the edges of the screen. The latter is now available in the "Wide HUD" mode.

**Improvements**

 * Compatibility of extended savegames has been improved across forks which change the project name away from "crispy-doom". Zodomaniac noticed this issue with his So Doom fork.
 * No 'Oof' sounds are played anymore upon attempt to manipulate an inactive Crispness menu item, on Zodomaniac's remark.

**Bug Fixes**

 * HUD widgets are now re-initialized immediately after graphics initialization to mitigate the alignment issues that JNechaevsky and Zodomaniac experienced on launching a map from command line.
 * Previous colorization of "percent" status bar widgets is now remembered. This applies to the health and armor widget and forces them to update not only if their value changes, but also if their colorization changes, e.g. when entering the IDDQD cheat. Thanks to unerxai for the report.
 * Horizontal location of the "PAUSE" graphic in widescreen mode has been fixed, thanks to JNechaevsky for spotting this.
 * The widescreen rendering setting is not changed anymore by disabling aspect ratio correction, requested by Zodomaniac.
 * An undetected merge conflict has been fixed in `hexen/r_things.c`, thanks to NickZ for the bug report.

Crispy Doom 5.7.2 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f1fa7faf`](https://github.com/chocolate-doom/chocolate-doom/commit/f1fa7faf566e3e325c1b1fbf8ace8d5ac2db2b1e).

### Crispy Doom 5.7.1

Crispy Doom 5.7.1 has been released on March 03, 2020 to fix some bugs related to widescreen rendering.

**Improvements**

* In order to improve the discoverability of the "A Secret is Revealed!" feature, its corresponding Crispness menu item has been renamed to "Report Revealed Secrets", based on a discussion with oprypin.
* Widescreen rendering is now disabled in case of disabled aspect ratio correction, as Zodomaniac proved it made no sense.

**Bug Fixes**

* A crash when switching to high resolution or widescreen rendering during a screen wipe has been fixed, reported and investigated by Zodomaniac and JNechaevsky.
* The obtrusive MAP/WAD automap text widget is not drawn anymore in widescreen mode, thanks to plumsinus.
* Horizontal coordinates of automap markers in widescreen mode have been fixed, thanks to unerxai from Doomworld forums.

Crispy Doom 5.7.1 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f1fa7faf`](https://github.com/chocolate-doom/chocolate-doom/commit/f1fa7faf566e3e325c1b1fbf8ace8d5ac2db2b1e).

### Crispy Doom 5.7 

Crispy Doom 5.7 has been released on February 21, 2020. This release brings proper widescreen rendering as well as other improvements requested by the speed-running community.

**Features**

* Widescreen rendering has been implemented, thanks to JNechaevsky and ice44 for spotting the bugs that had been surfacing in the progress.
* Loading 16 bit RIFF wavs in .wads has been enabled, contributed by SmileTheory.
* `-lumpdump` command line parameter has been added that dumps raw content of a lump into a file.

**Improvements**

* `IDDT` cheat is now reset when re-starting map during demo recording, thanks to Looper for the suggestion.
* Time in demo timer widget is now printed in centiseconds instead of tics. Thanks to Looper and ZeroMaster010 for the proposal.

**Bug Fixes**

* Fix for GUS emulation in presence of midiproc.exe has been pulled from Chocolate Doom, whereto it was contributed by JNechaevsky.

Crispy Doom 5.7 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f1fa7faf`](https://github.com/chocolate-doom/chocolate-doom/commit/f1fa7faf566e3e325c1b1fbf8ace8d5ac2db2b1e).

### Crispy Doom 5.6.4 

Crispy Doom 5.6.4 has been released on January 14, 2020. This release addresses the community feedback received after 5.6.3.

**Features**

 * Toggling full screen using Enter on num. keyboard is now possible (inherited from Chocolate Doom).
 * `IDCLEV` is now allowed during demo playback and warps to the requested map (starting a new demo).
 * If a new game is started (current level is reloaded/next level is loaded on pressing the respective key) during demo recording, a new demo is started. Thanks to Looper from Doomworld forums for the feature request.
 * 'Intermediate Crispy HUD' without the status bar but with the face and its background in place has been introduced.

**Improvements**

 * `leveltime` is now shown in the demo timer widget during recording instead of the accumulated demo time, thanks to Looper from Doomworld forums for the input.
 * Windows binaries being 32-bit has been clarified, thanks to RetroDoomKid for the remark.
 * Demo timer widget is now reset when restarting a demo during recording.
 * `gamemap` is now set to `startmap` when restarting a demo during recording.
 * Blood splats and projectile puffs are now drawn as small squares in the Automap.
 * Level/demo restart key description has been adjusted to become self-explanatory.
 * Static demo file name suffix counter has been added. This avoids checks for an increasing number of file names being available by just remembering the latest given suffix number. Thanks to Looper for the suggestion.
 * Demos are now restarted from the map they were started, thanks to Looper for the input.
 * OSX: Freedoom single player IWADs have been added to launcher (inherited from Chocolate Doom).
 * The smooth chainsaw idle animation has been brought back.
 * Weapon bobbing has been reworked and made adjustable.
 * Disallowing the vertical mouse movement now disables controlling the menus with the mouse, thanks to bryc for requesting this.
 * TNTWEAP0 now removes the berserk strength, all weapons and ammo except for pistol and 50 bullets.
 * Early exit from the tally screen after ExM8 is now forced, which enables demos to progress to the end of game sequence.
 * TNTWEAP2 now removes the pistol.
 * Some clipping optimizations taken from JNechaevsky's Russian Doom (and there from MBF respectively) have been implemented.
 * Savegame name is automatically overridden on saving if it already starts with a map identifier, proposed by zebzorb.
 * Status bar optimizations, including numbers to be only redrawn if necessary, on JNechaevsky's suggestion.
 * In automap overlay mode the automap is now drawn on top of everything as JNechaevsky suggested, not beneath the bezel for decreased screen sizes.

**Bug Fixes**

 * Missing prototype for `calloc()` in `r_data.c` causing memory corruption on 64bit in Windows/MSVC builds has been fixed, thanks to zx64 for spotting this. 
 * Crash when the flag for the berserk pack sprite patch memory zone is changed has been fixed, thanks to IsBebs for the bug report and Zodomaniac, JNechaevsky and turol for helping with the analysis.
 * Zombie player crash on SELFDEAD has been fixed, thanks to tpoppins for the report and turol for the analysis.
 * `"doomstat.h"` is now included instead of `<doomstat.h>` in `doom/r_swirl.c`, fixing compilation with MSVC2017, and packed attribute for structs when compiling with MSVC has been fixed. Thanks to drfrag666 for reporting and confirming the fix.
 * Shadowed menu and text drawing has been removed, as it is bugged in wipe screens.
 * The par time for MAP33 is now determined correctly (inherited from Chocolate Doom).
 * Subsequent calls to `A_BrainAwake()` to reset the `braintargeton` variable are now allowed. This fixes demo sync for maps with more than one brain, e.g. PL2.
 * Player viewheight in NOMOMENTUM mode has been fixed on Zodomaniac's report.
 * The revenant sync bug (with homing or non-homing missiles) with New Game demos has hopefully been fixed.
 * No statdump output is generated now for ExM8, and updating the Archvile fire's `floorz` and `ceilingz` values has been reverted, which fixes demo desyncs that fraggle discovered. Thanks a lot!
 * Fuzz effect animation remaining static in one case has been fixed, this happened if the number of pixels to apply the fuzz effect to was an integer multiple of FUZZTABLE. Thanks to JNechaevsky for the suggestion!
 * Status bar face expression staying across level changes has been removed, thanks to JNechaevsky for pointing this out.
 * Automap panning in flippedlevels mode has been fixed, thanks to JNechaevsky for reporting.
 * Self-repeating states in `P_LatestSafeState()` are now handled.
 * Max-sized background buffer is now allocated for the bezel. This fixes a crash when the game is started with `crispy->hires == 0` and `scaledviewwidth != SCREENWIDTH` and then `crispy->hires` is switched to `1`.
 * Switching to the fist after typing a cheat expecting two parameters has been fixed. This affects IDMUSx1 and IDCLEVx1, thanks to maxmanium for pointing this out.
 * Automap marker coordinate for flipped levels has been fixed.
 * Loss of grid lines near the automap boundary has been fixed, spotted by JNechaevsky.
 * Overlayed automap blinking one tic on screen borders has been fixed, noticed by JNechaevsky.

Crispy Doom 5.6.4 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f81b5c7b`](https://github.com/chocolate-doom/chocolate-doom/commit/f81b5c7b3e7c24364fce681ad3d7ba18119a867b).

## Documentation

 * **[Changelog](https://github.com/fabiangreffrath/crispy-doom/wiki/Changelog)**
 * **[Compatibility](https://github.com/fabiangreffrath/crispy-doom/wiki/Compatibility)**
 * **[Crispness](https://github.com/fabiangreffrath/crispy-doom/wiki/Crispness)**
 * **[FAQ](https://github.com/fabiangreffrath/crispy-doom/wiki/FAQ)**

## Versioning

Crispy Doom's major version number is increased whenever a new Chocolate Doom (pre-)release got merged into its code base. The minor version number is increased for intermediate releases that do only contain Crispy-specific changes or unreleased changes to the Chocolate Doom code base. The micro or patch version is reserved for post-release hotfixes, it remained unused until the 5.5 release.

## Contact

The canonical homepage for Crispy Doom is https://github.com/fabiangreffrath/crispy-doom

Crispy Doom is maintained by [Fabian Greffrath](mailto:fabian@greffXremovethisXrath.com). 

Please report any bugs, glitches or crashes that you encounter to the GitHub [Issue Tracker](https://github.com/fabiangreffrath/crispy-doom/issues).

## Acknowledgement

Although I have played the thought of hacking on Chocolate Doom's renderer for quite some time already, it was Brad Harding's [Doom Retro](https://www.chocolate-doom.org/wiki/index.php/Doom_Retro) that provided the incentive to finally do it. However, his fork aims at a different direction and I did not take a single line of code from it. Lee Killough's [MBF](https://doomwiki.org/wiki/WinMBF) was studied and used to debug the code, especially in the form of Team Eternity's [WinMBF](https://doomwiki.org/wiki/WinMBF) source port, which made it easier to compile and run on my machine. And of course there is fraggle's [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom) with its exceptionally clean and legible source code. Please let me take this opportunity to appreciate all these authors for their work!

Also, thanks to plums of the [Doomworld forums](https://www.doomworld.com/vb/) for beta testing, "release manager" Zodomaniac and "art director" JNechaevsky for the continuous flow of support and inspiration during the post-3.x development cycle and (last but not the least) [Cacodemon9000](http://www.moddb.com/members/cacodemon9000) for his [Infested Outpost](http://www.moddb.com/games/doom-ii/addons/infested-outpost) map that helped to track down quite a few bugs!

Furthermore, thanks to VGA for his aid with adding support for his two mods: [PerK & NightFright's Black Ops smooth weapons add-on converted to DEHACKED](https://www.doomworld.com/forum/topic/84859-black-ops-smooth-weapons-dehacked-mod) and [Gifty's Smooth Doom smooth monster animations converted to DEHACKED](https://www.doomworld.com/forum/topic/85991-smoothed-smooth-monsters-for-doom-retro-and-crispy-doom) that can make the gameplay even more pleasing to the eyes.

## Legalese

Doom is © 1993-1996 Id Software, Inc.; 
Boom 2.02 is © 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman;
PrBoom+ is © 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman,
© 1999-2000 Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze,
© 2005-2006 Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko;
Chocolate Doom is © 1993-1996 Id Software, Inc., © 2005 Simon Howard; 
Chocolate Hexen is © 1993-1996 Id Software, Inc., © 1993-2008 Raven Software, © 2008 Simon Howard;
Strawberry Doom is © 1993-1996 Id Software, Inc., © 2005 Simon Howard, © 2008-2010 GhostlyDeath; 
Crispy Doom is additionally © 2014-2019 Fabian Greffrath;
all of the above are released under the [GPL-2+](https://www.gnu.org/licenses/gpl-2.0.html).

SDL 2.0, SDL_mixer 2.0 and SDL_net 2.0 are © 1997-2016 Sam Lantinga and are released under the [zlib license](http://www.gzip.org/zlib/zlib_license.html).

Secret Rabbit Code (libsamplerate) is © 2002-2011 Erik de Castro Lopo and is released under the [GPL-2+](http://www.gnu.org/licenses/gpl-2.0.html).
Libpng is © 1998-2014 Glenn Randers-Pehrson, © 1996-1997 Andreas Dilger, © 1995-1996 Guy Eric Schalnat, Group 42, Inc. and is released under the [libpng license](http://www.libpng.org/pub/png/src/libpng-LICENSE.txt).
Zlib is © 1995-2013 Jean-loup Gailly and Mark Adler and is released under the [zlib license](http://www.zlib.net/zlib_license.html).

The Crispy Doom icon (as shown at the top of this page) is composed of the [Chocolate Doom icon](https://www.chocolate-doom.org/wiki/images/7/77/Chocolate-logo.png) and a [photo](https://en.wikipedia.org/wiki/File:Potato-Chips.jpg) of potato crisps (Utz-brand, grandma's kettle-cooked style) by [Evan-Amos](https://commons.wikimedia.org/wiki/User:Evan-Amos) who kindly released it into the [public domain](https://en.wikipedia.org/wiki/Public_domain). The current high-resolution version of this icon has been contributed by JNechaevsky (formerly by Zodomaniac).
