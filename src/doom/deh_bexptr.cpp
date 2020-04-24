//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014 Fabian Greffrath
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
// Parses [CODEPTR] sections in BEX files
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>

#include "info.hpp"

#include "deh_io.hpp"
#include "deh_main.hpp"
#include "event_function_decls.hpp"

struct bex_codeptr_t {
    const char *mnemonic;
    const actionf_t pointer;
};

static constexpr bex_codeptr_t bex_codeptrtable[] = {
    {"Light0", actionf_t{A_Light0}},
    {"WeaponReady", actionf_t{A_WeaponReady}},
    {"Lower", actionf_t{A_Lower}},
    {"Raise", actionf_t{A_Raise}},
    {"Punch", actionf_t{A_Punch}},
    {"ReFire", actionf_t{A_ReFire}},
    {"FirePistol", actionf_t{A_FirePistol}},
    {"Light1", actionf_t{A_Light1}},
    {"FireShotgun", actionf_t{A_FireShotgun}},
    {"Light2", actionf_t{A_Light2}},
    {"FireShotgun2", actionf_t{A_FireShotgun2}},
    {"CheckReload", actionf_t{A_CheckReload}},
    {"OpenShotgun2", actionf_t{A_OpenShotgun2}},
    {"LoadShotgun2", actionf_t{A_LoadShotgun2}},
    {"CloseShotgun2", actionf_t{A_CloseShotgun2}},
    {"FireCGun", actionf_t{A_FireCGun}},
    {"GunFlash", actionf_t{A_GunFlash}},
    {"FireMissile", actionf_t{A_FireMissile}},
    {"Saw", actionf_t{A_Saw}},
    {"FirePlasma", actionf_t{A_FirePlasma}},
    {"BFGsound", actionf_t{A_BFGsound}},
    {"FireBFG", actionf_t{A_FireBFG}},
    {"BFGSpray", actionf_t{A_BFGSpray}},
    {"Explode", actionf_t{A_Explode}},
    {"Pain", actionf_t{A_Pain}},
    {"PlayerScream", actionf_t{A_PlayerScream}},
    {"Fall", actionf_t{A_Fall}},
    {"XScream", actionf_t{A_XScream}},
    {"Look", actionf_t{A_Look}},
    {"Chase", actionf_t{A_Chase}},
    {"FaceTarget", actionf_t{A_FaceTarget}},
    {"PosAttack", actionf_t{A_PosAttack}},
    {"Scream", actionf_t{A_Scream}},
    {"SPosAttack", actionf_t{A_SPosAttack}},
    {"VileChase", actionf_t{A_VileChase}},
    {"VileStart", actionf_t{A_VileStart}},
    {"VileTarget", actionf_t{A_VileTarget}},
    {"VileAttack", actionf_t{A_VileAttack}},
    {"StartFire", actionf_t{A_StartFire}},
    {"Fire", actionf_t{A_Fire}},
    {"FireCrackle", actionf_t{A_FireCrackle}},
    {"Tracer", actionf_t{A_Tracer}},
    {"SkelWhoosh", actionf_t{A_SkelWhoosh}},
    {"SkelFist", actionf_t{A_SkelFist}},
    {"SkelMissile", actionf_t{A_SkelMissile}},
    {"FatRaise", actionf_t{A_FatRaise}},
    {"FatAttack1", actionf_t{A_FatAttack1}},
    {"FatAttack2", actionf_t{A_FatAttack2}},
    {"FatAttack3", actionf_t{A_FatAttack3}},
    {"BossDeath", actionf_t{A_BossDeath}},
    {"CPosAttack", actionf_t{A_CPosAttack}},
    {"CPosRefire", actionf_t{A_CPosRefire}},
    {"TroopAttack", actionf_t{A_TroopAttack}},
    {"SargAttack", actionf_t{A_SargAttack}},
    {"HeadAttack", actionf_t{A_HeadAttack}},
    {"BruisAttack", actionf_t{A_BruisAttack}},
    {"SkullAttack", actionf_t{A_SkullAttack}},
    {"Metal", actionf_t{A_Metal}},
    {"SpidRefire", actionf_t{A_SpidRefire}},
    {"BabyMetal", actionf_t{A_BabyMetal}},
    {"BspiAttack", actionf_t{A_BspiAttack}},
    {"Hoof", actionf_t{A_Hoof}},
    {"CyberAttack", actionf_t{A_CyberAttack}},
    {"PainAttack", actionf_t{A_PainAttack}},
    {"PainDie", actionf_t{A_PainDie}},
    {"KeenDie", actionf_t{A_KeenDie}},
    {"BrainPain", actionf_t{A_BrainPain}},
    {"BrainScream", actionf_t{A_BrainScream}},
    {"BrainDie", actionf_t{A_BrainDie}},
    {"BrainAwake", actionf_t{A_BrainAwake}},
    {"BrainSpit", actionf_t{A_BrainSpit}},
    {"SpawnSound", actionf_t{A_SpawnSound}},
    {"SpawnFly", actionf_t{A_SpawnFly}},
    {"BrainExplode", actionf_t{A_BrainExplode}},
    // [crispy] additional BOOM and MBF states, sprites and code pointers
    {"Stop", actionf_t{A_Stop}},
    {"Die", actionf_t{A_Die}},
    {"FireOldBFG", actionf_t{A_FireOldBFG}},
    {"Detonate", actionf_t{A_Detonate}},
    {"Mushroom", actionf_t{A_Mushroom}},
    {"BetaSkullAttack", actionf_t{A_BetaSkullAttack}},
    // [crispy] more MBF code pointers
    {"Spawn", actionf_t{A_Spawn}},
    {"Turn", actionf_t{A_Turn}},
    {"Face", actionf_t{A_Face}},
    {"Scratch", actionf_t{A_Scratch}},
    {"PlaySound", actionf_t{A_PlaySound}},
    {"RandomJump", actionf_t{A_RandomJump}},
    {"LineEffect", actionf_t{A_LineEffect}},
    {"NULL", {}},
};


extern actionf_t codeptrs[NUMSTATES];

static void *DEH_BEXPtrStart(deh_context_t *context, char *line)
{
    char s[10];

    if (sscanf(line, "%9s", s) == 0 || strcmp("[CODEPTR]", s))
    {
	DEH_Warning(context, "Parse error on section start");
    }

    return NULL;
}

static void DEH_BEXPtrParseLine(deh_context_t *context, char *line, void *tag)
{
    state_t *state;
    char *variable_name, *value, frame_str[6];
    int frame_number, i;

    // parse "FRAME nn = mnemonic", where
    // variable_name = "FRAME nn" and value = "mnemonic"
    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
	DEH_Warning(context, "Failed to parse assignment: %s", line);
	return;
    }

    // parse "FRAME nn", where frame_number = "nn"
    if (sscanf(variable_name, "%5s %32d", frame_str, &frame_number) != 2 ||
        strcasecmp(frame_str, "FRAME"))
    {
	DEH_Warning(context, "Failed to parse assignment: %s", variable_name);
	return;
    }

    if (frame_number < 0 || frame_number >= NUMSTATES)
    {
	DEH_Warning(context, "Invalid frame number: %i", frame_number);
	return;
    }

    state = (state_t *) &states[frame_number];

    for (i = 0; i < arrlen(bex_codeptrtable); i++)
    {
	if (!strcasecmp(bex_codeptrtable[i].mnemonic, value))
	{
	    state->action = bex_codeptrtable[i].pointer;
	    return;
	}
    }

    DEH_Warning(context, "Invalid mnemonic '%s'", value);
}

deh_section_t deh_section_bexptr =
{
    "[CODEPTR]",
    NULL,
    DEH_BEXPtrStart,
    DEH_BEXPtrParseLine,
    NULL,
    NULL,
};
