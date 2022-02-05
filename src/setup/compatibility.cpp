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

// Sound control menu

#include <cstdlib>

#include "crispy.hpp"
#include "m_config.hpp"
#include "textscreen.hpp"
#include "mode.hpp"

#include "compatibility.hpp"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-compat"

int vanilla_savegame_limit = 0;
int vanilla_demo_limit = 0;

void CompatibilitySettings(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    // [crispy]
    if (gamemission == doom)
    {
        TXT_MessageBox(NULL, "Please refer to the in-game Crispness menu.");
    }
    else
    {
    window = TXT_NewWindow("Compatibility");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Vanilla savegame limit",
                                   &vanilla_savegame_limit),
                   TXT_NewCheckBox("Vanilla demo limit",
                                   &vanilla_demo_limit),
                   NULL);
    }
}

void BindCompatibilityVariables()
{
    // [crispy]
    if (gamemission == doom)
    {
        M_BindIntVariable("crispy_automapoverlay",  &crispy->automapoverlay);
        M_BindIntVariable("crispy_automaprotate",   &crispy->automaprotate);
        M_BindIntVariable("crispy_automapstats",    &crispy->automapstats);
        M_BindIntVariable("crispy_bobfactor",       &crispy->bobfactor);
        M_BindIntVariable("crispy_brightmaps",      &crispy->brightmaps);
        M_BindIntVariable("crispy_centerweapon",    &crispy->centerweapon);
        M_BindIntVariable("crispy_coloredblood",    &crispy->coloredblood);
        M_BindIntVariable("crispy_coloredhud",      &crispy->coloredhud);
        M_BindIntVariable("crispy_crosshair",       &crispy->crosshair);
        M_BindIntVariable("crispy_crosshairhealth", &crispy->crosshairhealth);
        M_BindIntVariable("crispy_crosshairtarget", &crispy->crosshairtarget);
        M_BindIntVariable("crispy_crosshairtype",   &crispy->crosshairtype);
        M_BindIntVariable("crispy_demobar",         &crispy->demobar);
        M_BindIntVariable("crispy_demotimer",       &crispy->demotimer);
        M_BindIntVariable("crispy_demotimerdir",    &crispy->demotimerdir);
        M_BindIntVariable("crispy_extautomap",      &crispy->extautomap);
        M_BindIntVariable("crispy_extsaveg",        &crispy->extsaveg);
        M_BindIntVariable("crispy_flipcorpses",     &crispy->flipcorpses);
        M_BindIntVariable("crispy_freeaim",         &crispy->freeaim);
        M_BindIntVariable("crispy_freelook",        &crispy->freelook);
        M_BindIntVariable("crispy_hires",           &crispy->hires);
        M_BindIntVariable("crispy_jump",            &crispy->jump);
        M_BindIntVariable("crispy_leveltime",       &crispy->leveltime);
        M_BindIntVariable("crispy_mouselook",       &crispy->mouselook);
        M_BindIntVariable("crispy_neghealth",       &crispy->neghealth);
        M_BindIntVariable("crispy_overunder",       &crispy->overunder);
        M_BindIntVariable("crispy_pitch",           &crispy->pitch);
        M_BindIntVariable("crispy_playercoords",    &crispy->playercoords);
        M_BindIntVariable("crispy_recoil",          &crispy->recoil);
        M_BindIntVariable("crispy_secretmessage",   &crispy->secretmessage);
        M_BindIntVariable("crispy_smoothlight",     &crispy->smoothlight);
        M_BindIntVariable("crispy_smoothscaling",   &crispy->smoothscaling);
        M_BindIntVariable("crispy_soundfix",        &crispy->soundfix);
        M_BindIntVariable("crispy_soundfull",       &crispy->soundfull);
        M_BindIntVariable("crispy_soundmono",       &crispy->soundmono);
        M_BindIntVariable("crispy_translucency",    &crispy->translucency);
#ifdef CRISPY_TRUECOLOR
        M_BindIntVariable("crispy_truecolor",       &crispy->truecolor);
#endif
        M_BindIntVariable("crispy_uncapped",        &crispy->uncapped);
        M_BindIntVariable("crispy_vsync",           &crispy->vsync);
        M_BindIntVariable("crispy_weaponsquat",     &crispy->weaponsquat);
        M_BindIntVariable("crispy_widescreen",      &crispy->widescreen);
    }
    else
    {
    M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
    M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    }
}

