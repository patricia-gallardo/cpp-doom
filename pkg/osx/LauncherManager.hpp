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

#ifndef LAUNCHER_LAUNCHERMANAGER_H
#define LAUNCHER_LAUNCHERMANAGER_H

#include <AppKit/AppKit.h>
#include <AppKit/NSNibLoading.h>
#include "IWADController.hpp"

@interface LauncherManager : NSObject
{
    IWADController *iwadController;

    id launcherWindow;
    id launchButton;

    id commandLineArguments;
}

- () launch: (id)sender;
- () runSetup: (id)sender;
- () awakeFromNib;
- () clearCommandLine;
- (BOOL) addIWADPath: (NSString *) path;
- () addFileToCommandLine: (NSString *) fileName
         forArgument: (NSString *) args;
- (BOOL) selectGameByName: (const char *) name;
- () openTerminal: (id) sender;

- () openREADME: (id) sender;
- () openINSTALL: (id) sender;
- () openCMDLINE: (id) sender;
- () openCOPYING: (id) sender;
- () openDocumentation: (id) sender;

@end

#endif /* #ifndef LAUNCHER_LAUNCHERMANAGER_H */

