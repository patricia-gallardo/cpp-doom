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
#include "doomdef.hpp"
#include "p_action.hpp"

const char *sprnames[] = {
    "IMPX","ACLO","PTN1","SHLD","SHD2","BAGH","SPMP","INVS","PTN2","SOAR",
    "INVU","PWBK","EGGC","EGGM","FX01","SPHL","TRCH","FBMB","XPL1","ATLP",
    "PPOD","AMG1","SPSH","LVAS","SLDG","SKH1","SKH2","SKH3","SKH4","CHDL",
    "SRTC","SMPL","STGS","STGL","STCS","STCL","KFR1","BARL","BRPL","MOS1",
    "MOS2","WTRH","HCOR","KGZ1","KGZB","KGZG","KGZY","VLCO","VFBL","VTFB",
    "SFFI","TGLT","TELE","STFF","PUF3","PUF4","BEAK","WGNT","GAUN","PUF1",
    "WBLS","BLSR","FX18","FX17","WMCE","MACE","FX02","WSKL","HROD","FX00",
    "FX20","FX21","FX22","FX23","GWND","PUF2","WPHX","PHNX","FX04","FX08",
    "FX09","WBOW","CRBW","FX03","BLOD","PLAY","FDTH","BSKL","CHKN","MUMM",
    "FX15","BEAS","FRB1","SNKE","SNFX","HEAD","FX05","FX06","FX07","CLNK",
    "WZRD","FX11","FX10","KNIG","SPAX","RAXE","SRCR","FX14","SOR2","SDTH",
    "FX16","MNTR","FX12","FX13","AKYY","BKYY","CKYY","AMG2","AMM1","AMM2",
    "AMC1","AMC2","AMS1","AMS2","AMP1","AMP2","AMB1","AMB2", 
    nullptr
};

state_t states[NUMSTATES] = {
    {SPR_IMPX, 0, -1, null_hook(), S_NULL, 0, 0},      // S_NULL
    {SPR_ACLO, 4, 1050, A_FreeTargMobj, S_NULL, 0, 0},  // S_FREETARGMOBJ
    {SPR_PTN1, 0, 3, null_hook(), S_ITEM_PTN1_2, 0, 0},        // S_ITEM_PTN1_1
    {SPR_PTN1, 1, 3, null_hook(), S_ITEM_PTN1_3, 0, 0},        // S_ITEM_PTN1_2
    {SPR_PTN1, 2, 3, null_hook(), S_ITEM_PTN1_1, 0, 0},        // S_ITEM_PTN1_3
    {SPR_SHLD, 0, -1, null_hook(), S_NULL, 0, 0},      // S_ITEM_SHLD1
    {SPR_SHD2, 0, -1, null_hook(), S_NULL, 0, 0},      // S_ITEM_SHD2_1
    {SPR_BAGH, 0, -1, null_hook(), S_NULL, 0, 0},      // S_ITEM_BAGH1
    {SPR_SPMP, 0, -1, null_hook(), S_NULL, 0, 0},      // S_ITEM_SPMP1
    {SPR_ACLO, 4, 1400, null_hook(), S_HIDESPECIAL2, 0, 0},    // S_HIDESPECIAL1
    {SPR_ACLO, 0, 4, A_RestoreSpecialThing1, S_HIDESPECIAL3, 0, 0},     // S_HIDESPECIAL2
    {SPR_ACLO, 1, 4, null_hook(), S_HIDESPECIAL4, 0, 0},       // S_HIDESPECIAL3
    {SPR_ACLO, 0, 4, null_hook(), S_HIDESPECIAL5, 0, 0},       // S_HIDESPECIAL4
    {SPR_ACLO, 1, 4, null_hook(), S_HIDESPECIAL6, 0, 0},       // S_HIDESPECIAL5
    {SPR_ACLO, 2, 4, null_hook(), S_HIDESPECIAL7, 0, 0},       // S_HIDESPECIAL6
    {SPR_ACLO, 1, 4, null_hook(), S_HIDESPECIAL8, 0, 0},       // S_HIDESPECIAL7
    {SPR_ACLO, 2, 4, null_hook(), S_HIDESPECIAL9, 0, 0},       // S_HIDESPECIAL8
    {SPR_ACLO, 3, 4, null_hook(), S_HIDESPECIAL10, 0, 0},      // S_HIDESPECIAL9
    {SPR_ACLO, 2, 4, null_hook(), S_HIDESPECIAL11, 0, 0},      // S_HIDESPECIAL10
    {SPR_ACLO, 3, 4, A_RestoreSpecialThing2, S_NULL, 0, 0},     // S_HIDESPECIAL11
    {SPR_ACLO, 3, 3, null_hook(), S_DORMANTARTI2, 0, 0},       // S_DORMANTARTI1
    {SPR_ACLO, 2, 3, null_hook(), S_DORMANTARTI3, 0, 0},       // S_DORMANTARTI2
    {SPR_ACLO, 3, 3, null_hook(), S_DORMANTARTI4, 0, 0},       // S_DORMANTARTI3
    {SPR_ACLO, 2, 3, null_hook(), S_DORMANTARTI5, 0, 0},       // S_DORMANTARTI4
    {SPR_ACLO, 1, 3, null_hook(), S_DORMANTARTI6, 0, 0},       // S_DORMANTARTI5
    {SPR_ACLO, 2, 3, null_hook(), S_DORMANTARTI7, 0, 0},       // S_DORMANTARTI6
    {SPR_ACLO, 1, 3, null_hook(), S_DORMANTARTI8, 0, 0},       // S_DORMANTARTI7
    {SPR_ACLO, 0, 3, null_hook(), S_DORMANTARTI9, 0, 0},       // S_DORMANTARTI8
    {SPR_ACLO, 1, 3, null_hook(), S_DORMANTARTI10, 0, 0},      // S_DORMANTARTI9
    {SPR_ACLO, 0, 3, null_hook(), S_DORMANTARTI11, 0, 0},      // S_DORMANTARTI10
    {SPR_ACLO, 0, 1400, A_HideThing, S_DORMANTARTI12, 0, 0},    // S_DORMANTARTI11
    {SPR_ACLO, 0, 3, A_UnHideThing, S_DORMANTARTI13, 0, 0},     // S_DORMANTARTI12
    {SPR_ACLO, 1, 3, null_hook(), S_DORMANTARTI14, 0, 0},      // S_DORMANTARTI13
    {SPR_ACLO, 0, 3, null_hook(), S_DORMANTARTI15, 0, 0},      // S_DORMANTARTI14
    {SPR_ACLO, 1, 3, null_hook(), S_DORMANTARTI16, 0, 0},      // S_DORMANTARTI15
    {SPR_ACLO, 2, 3, null_hook(), S_DORMANTARTI17, 0, 0},      // S_DORMANTARTI16
    {SPR_ACLO, 1, 3, null_hook(), S_DORMANTARTI18, 0, 0},      // S_DORMANTARTI17
    {SPR_ACLO, 2, 3, null_hook(), S_DORMANTARTI19, 0, 0},      // S_DORMANTARTI18
    {SPR_ACLO, 3, 3, null_hook(), S_DORMANTARTI20, 0, 0},      // S_DORMANTARTI19
    {SPR_ACLO, 2, 3, null_hook(), S_DORMANTARTI21, 0, 0},      // S_DORMANTARTI20
    {SPR_ACLO, 3, 3, A_RestoreArtifact, S_NULL, 0, 0},  // S_DORMANTARTI21
    {SPR_ACLO, 3, 3, null_hook(), S_DEADARTI2, 0, 0},  // S_DEADARTI1
    {SPR_ACLO, 2, 3, null_hook(), S_DEADARTI3, 0, 0},  // S_DEADARTI2
    {SPR_ACLO, 3, 3, null_hook(), S_DEADARTI4, 0, 0},  // S_DEADARTI3
    {SPR_ACLO, 2, 3, null_hook(), S_DEADARTI5, 0, 0},  // S_DEADARTI4
    {SPR_ACLO, 1, 3, null_hook(), S_DEADARTI6, 0, 0},  // S_DEADARTI5
    {SPR_ACLO, 2, 3, null_hook(), S_DEADARTI7, 0, 0},  // S_DEADARTI6
    {SPR_ACLO, 1, 3, null_hook(), S_DEADARTI8, 0, 0},  // S_DEADARTI7
    {SPR_ACLO, 0, 3, null_hook(), S_DEADARTI9, 0, 0},  // S_DEADARTI8
    {SPR_ACLO, 1, 3, null_hook(), S_DEADARTI10, 0, 0}, // S_DEADARTI9
    {SPR_ACLO, 0, 3, null_hook(), S_NULL, 0, 0},       // S_DEADARTI10
    {SPR_INVS, 32768, 350, null_hook(), S_ARTI_INVS1, 0, 0},   // S_ARTI_INVS1
    {SPR_PTN2, 0, 4, null_hook(), S_ARTI_PTN2_2, 0, 0},        // S_ARTI_PTN2_1
    {SPR_PTN2, 1, 4, null_hook(), S_ARTI_PTN2_3, 0, 0},        // S_ARTI_PTN2_2
    {SPR_PTN2, 2, 4, null_hook(), S_ARTI_PTN2_1, 0, 0},        // S_ARTI_PTN2_3
    {SPR_SOAR, 0, 5, null_hook(), S_ARTI_SOAR2, 0, 0}, // S_ARTI_SOAR1
    {SPR_SOAR, 1, 5, null_hook(), S_ARTI_SOAR3, 0, 0}, // S_ARTI_SOAR2
    {SPR_SOAR, 2, 5, null_hook(), S_ARTI_SOAR4, 0, 0}, // S_ARTI_SOAR3
    {SPR_SOAR, 1, 5, null_hook(), S_ARTI_SOAR1, 0, 0}, // S_ARTI_SOAR4
    {SPR_INVU, 0, 3, null_hook(), S_ARTI_INVU2, 0, 0}, // S_ARTI_INVU1
    {SPR_INVU, 1, 3, null_hook(), S_ARTI_INVU3, 0, 0}, // S_ARTI_INVU2
    {SPR_INVU, 2, 3, null_hook(), S_ARTI_INVU4, 0, 0}, // S_ARTI_INVU3
    {SPR_INVU, 3, 3, null_hook(), S_ARTI_INVU1, 0, 0}, // S_ARTI_INVU4
    {SPR_PWBK, 0, 350, null_hook(), S_ARTI_PWBK1, 0, 0},       // S_ARTI_PWBK1
    {SPR_EGGC, 0, 6, null_hook(), S_ARTI_EGGC2, 0, 0}, // S_ARTI_EGGC1
    {SPR_EGGC, 1, 6, null_hook(), S_ARTI_EGGC3, 0, 0}, // S_ARTI_EGGC2
    {SPR_EGGC, 2, 6, null_hook(), S_ARTI_EGGC4, 0, 0}, // S_ARTI_EGGC3
    {SPR_EGGC, 1, 6, null_hook(), S_ARTI_EGGC1, 0, 0}, // S_ARTI_EGGC4
    {SPR_EGGM, 0, 4, null_hook(), S_EGGFX2, 0, 0},     // S_EGGFX1
    {SPR_EGGM, 1, 4, null_hook(), S_EGGFX3, 0, 0},     // S_EGGFX2
    {SPR_EGGM, 2, 4, null_hook(), S_EGGFX4, 0, 0},     // S_EGGFX3
    {SPR_EGGM, 3, 4, null_hook(), S_EGGFX5, 0, 0},     // S_EGGFX4
    {SPR_EGGM, 4, 4, null_hook(), S_EGGFX1, 0, 0},     // S_EGGFX5
    {SPR_FX01, 32772, 3, null_hook(), S_EGGFXI1_2, 0, 0},      // S_EGGFXI1_1
    {SPR_FX01, 32773, 3, null_hook(), S_EGGFXI1_3, 0, 0},      // S_EGGFXI1_2
    {SPR_FX01, 32774, 3, null_hook(), S_EGGFXI1_4, 0, 0},      // S_EGGFXI1_3
    {SPR_FX01, 32775, 3, null_hook(), S_NULL, 0, 0},   // S_EGGFXI1_4
    {SPR_SPHL, 0, 350, null_hook(), S_ARTI_SPHL1, 0, 0},       // S_ARTI_SPHL1
    {SPR_TRCH, 32768, 3, null_hook(), S_ARTI_TRCH2, 0, 0},     // S_ARTI_TRCH1
    {SPR_TRCH, 32769, 3, null_hook(), S_ARTI_TRCH3, 0, 0},     // S_ARTI_TRCH2
    {SPR_TRCH, 32770, 3, null_hook(), S_ARTI_TRCH1, 0, 0},     // S_ARTI_TRCH3
    {SPR_FBMB, 4, 350, null_hook(), S_ARTI_FBMB1, 0, 0},       // S_ARTI_FBMB1
    {SPR_FBMB, 0, 10, null_hook(), S_FIREBOMB2, 0, 0}, // S_FIREBOMB1
    {SPR_FBMB, 1, 10, null_hook(), S_FIREBOMB3, 0, 0}, // S_FIREBOMB2
    {SPR_FBMB, 2, 10, null_hook(), S_FIREBOMB4, 0, 0}, // S_FIREBOMB3
    {SPR_FBMB, 3, 10, null_hook(), S_FIREBOMB5, 0, 0}, // S_FIREBOMB4
    {SPR_FBMB, 4, 6, A_Scream, S_FIREBOMB6, 0, 0},      // S_FIREBOMB5
    {SPR_XPL1, 32768, 4, A_Explode, S_FIREBOMB7, 0, 0}, // S_FIREBOMB6
    {SPR_XPL1, 32769, 4, null_hook(), S_FIREBOMB8, 0, 0},      // S_FIREBOMB7
    {SPR_XPL1, 32770, 4, null_hook(), S_FIREBOMB9, 0, 0},      // S_FIREBOMB8
    {SPR_XPL1, 32771, 4, null_hook(), S_FIREBOMB10, 0, 0},     // S_FIREBOMB9
    {SPR_XPL1, 32772, 4, null_hook(), S_FIREBOMB11, 0, 0},     // S_FIREBOMB10
    {SPR_XPL1, 32773, 4, null_hook(), S_NULL, 0, 0},   // S_FIREBOMB11
    {SPR_ATLP, 0, 4, null_hook(), S_ARTI_ATLP2, 0, 0}, // S_ARTI_ATLP1
    {SPR_ATLP, 1, 4, null_hook(), S_ARTI_ATLP3, 0, 0}, // S_ARTI_ATLP2
    {SPR_ATLP, 2, 4, null_hook(), S_ARTI_ATLP4, 0, 0}, // S_ARTI_ATLP3
    {SPR_ATLP, 1, 4, null_hook(), S_ARTI_ATLP1, 0, 0}, // S_ARTI_ATLP4
    {SPR_PPOD, 0, 10, null_hook(), S_POD_WAIT1, 0, 0}, // S_POD_WAIT1
    {SPR_PPOD, 1, 14, A_PodPain, S_POD_WAIT1, 0, 0},    // S_POD_PAIN1
    {SPR_PPOD, 32770, 5, A_RemovePod, S_POD_DIE2, 0, 0},        // S_POD_DIE1
    {SPR_PPOD, 32771, 5, A_Scream, S_POD_DIE3, 0, 0},   // S_POD_DIE2
    {SPR_PPOD, 32772, 5, A_Explode, S_POD_DIE4, 0, 0},  // S_POD_DIE3
    {SPR_PPOD, 32773, 10, null_hook(), S_FREETARGMOBJ, 0, 0},  // S_POD_DIE4
    {SPR_PPOD, 8, 3, null_hook(), S_POD_GROW2, 0, 0},  // S_POD_GROW1
    {SPR_PPOD, 9, 3, null_hook(), S_POD_GROW3, 0, 0},  // S_POD_GROW2
    {SPR_PPOD, 10, 3, null_hook(), S_POD_GROW4, 0, 0}, // S_POD_GROW3
    {SPR_PPOD, 11, 3, null_hook(), S_POD_GROW5, 0, 0}, // S_POD_GROW4
    {SPR_PPOD, 12, 3, null_hook(), S_POD_GROW6, 0, 0}, // S_POD_GROW5
    {SPR_PPOD, 13, 3, null_hook(), S_POD_GROW7, 0, 0}, // S_POD_GROW6
    {SPR_PPOD, 14, 3, null_hook(), S_POD_GROW8, 0, 0}, // S_POD_GROW7
    {SPR_PPOD, 15, 3, null_hook(), S_POD_WAIT1, 0, 0}, // S_POD_GROW8
    {SPR_PPOD, 6, 8, null_hook(), S_PODGOO2, 0, 0},    // S_PODGOO1
    {SPR_PPOD, 7, 8, null_hook(), S_PODGOO1, 0, 0},    // S_PODGOO2
    {SPR_PPOD, 6, 10, null_hook(), S_NULL, 0, 0},      // S_PODGOOX
    {SPR_AMG1, 0, 35, A_MakePod, S_PODGENERATOR, 0, 0}, // S_PODGENERATOR
    {SPR_SPSH, 0, 8, null_hook(), S_SPLASH2, 0, 0},    // S_SPLASH1
    {SPR_SPSH, 1, 8, null_hook(), S_SPLASH3, 0, 0},    // S_SPLASH2
    {SPR_SPSH, 2, 8, null_hook(), S_SPLASH4, 0, 0},    // S_SPLASH3
    {SPR_SPSH, 3, 16, null_hook(), S_NULL, 0, 0},      // S_SPLASH4
    {SPR_SPSH, 3, 10, null_hook(), S_NULL, 0, 0},      // S_SPLASHX
    {SPR_SPSH, 4, 5, null_hook(), S_SPLASHBASE2, 0, 0},        // S_SPLASHBASE1
    {SPR_SPSH, 5, 5, null_hook(), S_SPLASHBASE3, 0, 0},        // S_SPLASHBASE2
    {SPR_SPSH, 6, 5, null_hook(), S_SPLASHBASE4, 0, 0},        // S_SPLASHBASE3
    {SPR_SPSH, 7, 5, null_hook(), S_SPLASHBASE5, 0, 0},        // S_SPLASHBASE4
    {SPR_SPSH, 8, 5, null_hook(), S_SPLASHBASE6, 0, 0},        // S_SPLASHBASE5
    {SPR_SPSH, 9, 5, null_hook(), S_SPLASHBASE7, 0, 0},        // S_SPLASHBASE6
    {SPR_SPSH, 10, 5, null_hook(), S_NULL, 0, 0},      // S_SPLASHBASE7
    {SPR_LVAS, 32768, 5, null_hook(), S_LAVASPLASH2, 0, 0},    // S_LAVASPLASH1
    {SPR_LVAS, 32769, 5, null_hook(), S_LAVASPLASH3, 0, 0},    // S_LAVASPLASH2
    {SPR_LVAS, 32770, 5, null_hook(), S_LAVASPLASH4, 0, 0},    // S_LAVASPLASH3
    {SPR_LVAS, 32771, 5, null_hook(), S_LAVASPLASH5, 0, 0},    // S_LAVASPLASH4
    {SPR_LVAS, 32772, 5, null_hook(), S_LAVASPLASH6, 0, 0},    // S_LAVASPLASH5
    {SPR_LVAS, 32773, 5, null_hook(), S_NULL, 0, 0},   // S_LAVASPLASH6
    {SPR_LVAS, 32774, 5, null_hook(), S_LAVASMOKE2, 0, 0},     // S_LAVASMOKE1
    {SPR_LVAS, 32775, 5, null_hook(), S_LAVASMOKE3, 0, 0},     // S_LAVASMOKE2
    {SPR_LVAS, 32776, 5, null_hook(), S_LAVASMOKE4, 0, 0},     // S_LAVASMOKE3
    {SPR_LVAS, 32777, 5, null_hook(), S_LAVASMOKE5, 0, 0},     // S_LAVASMOKE4
    {SPR_LVAS, 32778, 5, null_hook(), S_NULL, 0, 0},   // S_LAVASMOKE5
    {SPR_SLDG, 0, 8, null_hook(), S_SLUDGECHUNK2, 0, 0},       // S_SLUDGECHUNK1
    {SPR_SLDG, 1, 8, null_hook(), S_SLUDGECHUNK3, 0, 0},       // S_SLUDGECHUNK2
    {SPR_SLDG, 2, 8, null_hook(), S_SLUDGECHUNK4, 0, 0},       // S_SLUDGECHUNK3
    {SPR_SLDG, 3, 8, null_hook(), S_NULL, 0, 0},       // S_SLUDGECHUNK4
    {SPR_SLDG, 3, 6, null_hook(), S_NULL, 0, 0},       // S_SLUDGECHUNKX
    {SPR_SLDG, 4, 5, null_hook(), S_SLUDGESPLASH2, 0, 0},      // S_SLUDGESPLASH1
    {SPR_SLDG, 5, 5, null_hook(), S_SLUDGESPLASH3, 0, 0},      // S_SLUDGESPLASH2
    {SPR_SLDG, 6, 5, null_hook(), S_SLUDGESPLASH4, 0, 0},      // S_SLUDGESPLASH3
    {SPR_SLDG, 7, 5, null_hook(), S_NULL, 0, 0},       // S_SLUDGESPLASH4
    {SPR_SKH1, 0, -1, null_hook(), S_NULL, 0, 0},      // S_SKULLHANG70_1
    {SPR_SKH2, 0, -1, null_hook(), S_NULL, 0, 0},      // S_SKULLHANG60_1
    {SPR_SKH3, 0, -1, null_hook(), S_NULL, 0, 0},      // S_SKULLHANG45_1
    {SPR_SKH4, 0, -1, null_hook(), S_NULL, 0, 0},      // S_SKULLHANG35_1
    {SPR_CHDL, 0, 4, null_hook(), S_CHANDELIER2, 0, 0},        // S_CHANDELIER1
    {SPR_CHDL, 1, 4, null_hook(), S_CHANDELIER3, 0, 0},        // S_CHANDELIER2
    {SPR_CHDL, 2, 4, null_hook(), S_CHANDELIER1, 0, 0},        // S_CHANDELIER3
    {SPR_SRTC, 0, 4, null_hook(), S_SERPTORCH2, 0, 0}, // S_SERPTORCH1
    {SPR_SRTC, 1, 4, null_hook(), S_SERPTORCH3, 0, 0}, // S_SERPTORCH2
    {SPR_SRTC, 2, 4, null_hook(), S_SERPTORCH1, 0, 0}, // S_SERPTORCH3
    {SPR_SMPL, 0, -1, null_hook(), S_NULL, 0, 0},      // S_SMALLPILLAR
    {SPR_STGS, 0, -1, null_hook(), S_NULL, 0, 0},      // S_STALAGMITESMALL
    {SPR_STGL, 0, -1, null_hook(), S_NULL, 0, 0},      // S_STALAGMITELARGE
    {SPR_STCS, 0, -1, null_hook(), S_NULL, 0, 0},      // S_STALACTITESMALL
    {SPR_STCL, 0, -1, null_hook(), S_NULL, 0, 0},      // S_STALACTITELARGE
    {SPR_KFR1, 32768, 3, null_hook(), S_FIREBRAZIER2, 0, 0},   // S_FIREBRAZIER1
    {SPR_KFR1, 32769, 3, null_hook(), S_FIREBRAZIER3, 0, 0},   // S_FIREBRAZIER2
    {SPR_KFR1, 32770, 3, null_hook(), S_FIREBRAZIER4, 0, 0},   // S_FIREBRAZIER3
    {SPR_KFR1, 32771, 3, null_hook(), S_FIREBRAZIER5, 0, 0},   // S_FIREBRAZIER4
    {SPR_KFR1, 32772, 3, null_hook(), S_FIREBRAZIER6, 0, 0},   // S_FIREBRAZIER5
    {SPR_KFR1, 32773, 3, null_hook(), S_FIREBRAZIER7, 0, 0},   // S_FIREBRAZIER6
    {SPR_KFR1, 32774, 3, null_hook(), S_FIREBRAZIER8, 0, 0},   // S_FIREBRAZIER7
    {SPR_KFR1, 32775, 3, null_hook(), S_FIREBRAZIER1, 0, 0},   // S_FIREBRAZIER8
    {SPR_BARL, 0, -1, null_hook(), S_NULL, 0, 0},      // S_BARREL
    {SPR_BRPL, 0, -1, null_hook(), S_NULL, 0, 0},      // S_BRPILLAR
    {SPR_MOS1, 0, -1, null_hook(), S_NULL, 0, 0},      // S_MOSS1
    {SPR_MOS2, 0, -1, null_hook(), S_NULL, 0, 0},      // S_MOSS2
    {SPR_WTRH, 32768, 6, null_hook(), S_WALLTORCH2, 0, 0},     // S_WALLTORCH1
    {SPR_WTRH, 32769, 6, null_hook(), S_WALLTORCH3, 0, 0},     // S_WALLTORCH2
    {SPR_WTRH, 32770, 6, null_hook(), S_WALLTORCH1, 0, 0},     // S_WALLTORCH3
    {SPR_HCOR, 0, -1, null_hook(), S_NULL, 0, 0},      // S_HANGINGCORPSE
    {SPR_KGZ1, 0, 1, null_hook(), S_KEYGIZMO2, 0, 0},  // S_KEYGIZMO1
    {SPR_KGZ1, 0, 1, A_InitKeyGizmo, S_KEYGIZMO3, 0, 0},        // S_KEYGIZMO2
    {SPR_KGZ1, 0, -1, null_hook(), S_NULL, 0, 0},      // S_KEYGIZMO3
    {SPR_KGZB, 0, 1, null_hook(), S_KGZ_START, 0, 0},  // S_KGZ_START
    {SPR_KGZB, 32768, -1, null_hook(), S_NULL, 0, 0},  // S_KGZ_BLUEFLOAT1
    {SPR_KGZG, 32768, -1, null_hook(), S_NULL, 0, 0},  // S_KGZ_GREENFLOAT1
    {SPR_KGZY, 32768, -1, null_hook(), S_NULL, 0, 0},  // S_KGZ_YELLOWFLOAT1
    {SPR_VLCO, 0, 350, null_hook(), S_VOLCANO2, 0, 0}, // S_VOLCANO1
    {SPR_VLCO, 0, 35, A_VolcanoSet, S_VOLCANO3, 0, 0},  // S_VOLCANO2
    {SPR_VLCO, 1, 3, null_hook(), S_VOLCANO4, 0, 0},   // S_VOLCANO3
    {SPR_VLCO, 2, 3, null_hook(), S_VOLCANO5, 0, 0},   // S_VOLCANO4
    {SPR_VLCO, 3, 3, null_hook(), S_VOLCANO6, 0, 0},   // S_VOLCANO5
    {SPR_VLCO, 1, 3, null_hook(), S_VOLCANO7, 0, 0},   // S_VOLCANO6
    {SPR_VLCO, 2, 3, null_hook(), S_VOLCANO8, 0, 0},   // S_VOLCANO7
    {SPR_VLCO, 3, 3, null_hook(), S_VOLCANO9, 0, 0},   // S_VOLCANO8
    {SPR_VLCO, 4, 10, A_VolcanoBlast, S_VOLCANO2, 0, 0},        // S_VOLCANO9
    {SPR_VFBL, 0, 4, A_BeastPuff, S_VOLCANOBALL2, 0, 0},        // S_VOLCANOBALL1
    {SPR_VFBL, 1, 4, A_BeastPuff, S_VOLCANOBALL1, 0, 0},        // S_VOLCANOBALL2
    {SPR_XPL1, 0, 4, A_VolcBallImpact, S_VOLCANOBALLX2, 0, 0},  // S_VOLCANOBALLX1
    {SPR_XPL1, 1, 4, null_hook(), S_VOLCANOBALLX3, 0, 0},      // S_VOLCANOBALLX2
    {SPR_XPL1, 2, 4, null_hook(), S_VOLCANOBALLX4, 0, 0},      // S_VOLCANOBALLX3
    {SPR_XPL1, 3, 4, null_hook(), S_VOLCANOBALLX5, 0, 0},      // S_VOLCANOBALLX4
    {SPR_XPL1, 4, 4, null_hook(), S_VOLCANOBALLX6, 0, 0},      // S_VOLCANOBALLX5
    {SPR_XPL1, 5, 4, null_hook(), S_NULL, 0, 0},       // S_VOLCANOBALLX6
    {SPR_VTFB, 0, 4, null_hook(), S_VOLCANOTBALL2, 0, 0},      // S_VOLCANOTBALL1
    {SPR_VTFB, 1, 4, null_hook(), S_VOLCANOTBALL1, 0, 0},      // S_VOLCANOTBALL2
    {SPR_SFFI, 2, 4, null_hook(), S_VOLCANOTBALLX2, 0, 0},     // S_VOLCANOTBALLX1
    {SPR_SFFI, 1, 4, null_hook(), S_VOLCANOTBALLX3, 0, 0},     // S_VOLCANOTBALLX2
    {SPR_SFFI, 0, 4, null_hook(), S_VOLCANOTBALLX4, 0, 0},     // S_VOLCANOTBALLX3
    {SPR_SFFI, 1, 4, null_hook(), S_VOLCANOTBALLX5, 0, 0},     // S_VOLCANOTBALLX4
    {SPR_SFFI, 2, 4, null_hook(), S_VOLCANOTBALLX6, 0, 0},     // S_VOLCANOTBALLX5
    {SPR_SFFI, 3, 4, null_hook(), S_VOLCANOTBALLX7, 0, 0},     // S_VOLCANOTBALLX6
    {SPR_SFFI, 4, 4, null_hook(), S_NULL, 0, 0},       // S_VOLCANOTBALLX7
    {SPR_TGLT, 0, 8, A_SpawnTeleGlitter, S_TELEGLITGEN1, 0, 0}, // S_TELEGLITGEN1
    {SPR_TGLT, 5, 8, A_SpawnTeleGlitter2, S_TELEGLITGEN2, 0, 0},        // S_TELEGLITGEN2
    {SPR_TGLT, 32768, 2, null_hook(), S_TELEGLITTER1_2, 0, 0}, // S_TELEGLITTER1_1
    {SPR_TGLT, 32769, 2, A_AccTeleGlitter, S_TELEGLITTER1_3, 0, 0},     // S_TELEGLITTER1_2
    {SPR_TGLT, 32770, 2, null_hook(), S_TELEGLITTER1_4, 0, 0}, // S_TELEGLITTER1_3
    {SPR_TGLT, 32771, 2, A_AccTeleGlitter, S_TELEGLITTER1_5, 0, 0},     // S_TELEGLITTER1_4
    {SPR_TGLT, 32772, 2, null_hook(), S_TELEGLITTER1_1, 0, 0}, // S_TELEGLITTER1_5
    {SPR_TGLT, 32773, 2, null_hook(), S_TELEGLITTER2_2, 0, 0}, // S_TELEGLITTER2_1
    {SPR_TGLT, 32774, 2, A_AccTeleGlitter, S_TELEGLITTER2_3, 0, 0},     // S_TELEGLITTER2_2
    {SPR_TGLT, 32775, 2, null_hook(), S_TELEGLITTER2_4, 0, 0}, // S_TELEGLITTER2_3
    {SPR_TGLT, 32776, 2, A_AccTeleGlitter, S_TELEGLITTER2_5, 0, 0},     // S_TELEGLITTER2_4
    {SPR_TGLT, 32777, 2, null_hook(), S_TELEGLITTER2_1, 0, 0}, // S_TELEGLITTER2_5
    {SPR_TELE, 32768, 6, null_hook(), S_TFOG2, 0, 0},  // S_TFOG1
    {SPR_TELE, 32769, 6, null_hook(), S_TFOG3, 0, 0},  // S_TFOG2
    {SPR_TELE, 32770, 6, null_hook(), S_TFOG4, 0, 0},  // S_TFOG3
    {SPR_TELE, 32771, 6, null_hook(), S_TFOG5, 0, 0},  // S_TFOG4
    {SPR_TELE, 32772, 6, null_hook(), S_TFOG6, 0, 0},  // S_TFOG5
    {SPR_TELE, 32773, 6, null_hook(), S_TFOG7, 0, 0},  // S_TFOG6
    {SPR_TELE, 32774, 6, null_hook(), S_TFOG8, 0, 0},  // S_TFOG7
    {SPR_TELE, 32775, 6, null_hook(), S_TFOG9, 0, 0},  // S_TFOG8
    {SPR_TELE, 32774, 6, null_hook(), S_TFOG10, 0, 0}, // S_TFOG9
    {SPR_TELE, 32773, 6, null_hook(), S_TFOG11, 0, 0}, // S_TFOG10
    {SPR_TELE, 32772, 6, null_hook(), S_TFOG12, 0, 0}, // S_TFOG11
    {SPR_TELE, 32771, 6, null_hook(), S_TFOG13, 0, 0}, // S_TFOG12
    {SPR_TELE, 32770, 6, null_hook(), S_NULL, 0, 0},   // S_TFOG13
    {SPR_STFF, 0, 0, A_Light0, S_NULL, 0, 0},   // S_LIGHTDONE
    {SPR_STFF, 0, 1, A_WeaponReady, S_STAFFREADY, 0, 0},        // S_STAFFREADY
    {SPR_STFF, 0, 1, A_Lower, S_STAFFDOWN, 0, 0},       // S_STAFFDOWN
    {SPR_STFF, 0, 1, A_Raise, S_STAFFUP, 0, 0}, // S_STAFFUP
    {SPR_STFF, 3, 4, A_WeaponReady, S_STAFFREADY2_2, 0, 0},     // S_STAFFREADY2_1
    {SPR_STFF, 4, 4, A_WeaponReady, S_STAFFREADY2_3, 0, 0},     // S_STAFFREADY2_2
    {SPR_STFF, 5, 4, A_WeaponReady, S_STAFFREADY2_1, 0, 0},     // S_STAFFREADY2_3
    {SPR_STFF, 3, 1, A_Lower, S_STAFFDOWN2, 0, 0},      // S_STAFFDOWN2
    {SPR_STFF, 3, 1, A_Raise, S_STAFFUP2, 0, 0},        // S_STAFFUP2
    {SPR_STFF, 1, 6, null_hook(), S_STAFFATK1_2, 0, 0},        // S_STAFFATK1_1
    {SPR_STFF, 2, 8, A_StaffAttackPL1, S_STAFFATK1_3, 0, 0},    // S_STAFFATK1_2
    {SPR_STFF, 1, 8, A_ReFire, S_STAFFREADY, 0, 0},     // S_STAFFATK1_3
    {SPR_STFF, 6, 6, null_hook(), S_STAFFATK2_2, 0, 0},        // S_STAFFATK2_1
    {SPR_STFF, 7, 8, A_StaffAttackPL2, S_STAFFATK2_3, 0, 0},    // S_STAFFATK2_2
    {SPR_STFF, 6, 8, A_ReFire, S_STAFFREADY2_1, 0, 0},  // S_STAFFATK2_3
    {SPR_PUF3, 32768, 4, null_hook(), S_STAFFPUFF2, 0, 0},     // S_STAFFPUFF1
    {SPR_PUF3, 1, 4, null_hook(), S_STAFFPUFF3, 0, 0}, // S_STAFFPUFF2
    {SPR_PUF3, 2, 4, null_hook(), S_STAFFPUFF4, 0, 0}, // S_STAFFPUFF3
    {SPR_PUF3, 3, 4, null_hook(), S_NULL, 0, 0},       // S_STAFFPUFF4
    {SPR_PUF4, 32768, 4, null_hook(), S_STAFFPUFF2_2, 0, 0},   // S_STAFFPUFF2_1
    {SPR_PUF4, 32769, 4, null_hook(), S_STAFFPUFF2_3, 0, 0},   // S_STAFFPUFF2_2
    {SPR_PUF4, 32770, 4, null_hook(), S_STAFFPUFF2_4, 0, 0},   // S_STAFFPUFF2_3
    {SPR_PUF4, 32771, 4, null_hook(), S_STAFFPUFF2_5, 0, 0},   // S_STAFFPUFF2_4
    {SPR_PUF4, 32772, 4, null_hook(), S_STAFFPUFF2_6, 0, 0},   // S_STAFFPUFF2_5
    {SPR_PUF4, 32773, 4, null_hook(), S_NULL, 0, 0},   // S_STAFFPUFF2_6
    {SPR_BEAK, 0, 1, A_BeakReady, S_BEAKREADY, 0, 0},   // S_BEAKREADY
    {SPR_BEAK, 0, 1, A_Lower, S_BEAKDOWN, 0, 0},        // S_BEAKDOWN
    {SPR_BEAK, 0, 1, A_BeakRaise, S_BEAKUP, 0, 0},      // S_BEAKUP
    {SPR_BEAK, 0, 18, A_BeakAttackPL1, S_BEAKREADY, 0, 0},      // S_BEAKATK1_1
    {SPR_BEAK, 0, 12, A_BeakAttackPL2, S_BEAKREADY, 0, 0},      // S_BEAKATK2_1
    {SPR_WGNT, 0, -1, null_hook(), S_NULL, 0, 0},      // S_WGNT
    {SPR_GAUN, 0, 1, A_WeaponReady, S_GAUNTLETREADY, 0, 0},     // S_GAUNTLETREADY
    {SPR_GAUN, 0, 1, A_Lower, S_GAUNTLETDOWN, 0, 0},    // S_GAUNTLETDOWN
    {SPR_GAUN, 0, 1, A_Raise, S_GAUNTLETUP, 0, 0},      // S_GAUNTLETUP
    {SPR_GAUN, 6, 4, A_WeaponReady, S_GAUNTLETREADY2_2, 0, 0},  // S_GAUNTLETREADY2_1
    {SPR_GAUN, 7, 4, A_WeaponReady, S_GAUNTLETREADY2_3, 0, 0},  // S_GAUNTLETREADY2_2
    {SPR_GAUN, 8, 4, A_WeaponReady, S_GAUNTLETREADY2_1, 0, 0},  // S_GAUNTLETREADY2_3
    {SPR_GAUN, 6, 1, A_Lower, S_GAUNTLETDOWN2, 0, 0},   // S_GAUNTLETDOWN2
    {SPR_GAUN, 6, 1, A_Raise, S_GAUNTLETUP2, 0, 0},     // S_GAUNTLETUP2
    {SPR_GAUN, 1, 4, null_hook(), S_GAUNTLETATK1_2, 0, 0},     // S_GAUNTLETATK1_1
    {SPR_GAUN, 2, 4, null_hook(), S_GAUNTLETATK1_3, 0, 0},     // S_GAUNTLETATK1_2
    {SPR_GAUN, 32771, 4, A_GauntletAttack, S_GAUNTLETATK1_4, 0, 0},     // S_GAUNTLETATK1_3
    {SPR_GAUN, 32772, 4, A_GauntletAttack, S_GAUNTLETATK1_5, 0, 0},     // S_GAUNTLETATK1_4
    {SPR_GAUN, 32773, 4, A_GauntletAttack, S_GAUNTLETATK1_6, 0, 0},     // S_GAUNTLETATK1_5
    {SPR_GAUN, 2, 4, A_ReFire, S_GAUNTLETATK1_7, 0, 0}, // S_GAUNTLETATK1_6
    {SPR_GAUN, 1, 4, A_Light0, S_GAUNTLETREADY, 0, 0},  // S_GAUNTLETATK1_7
    {SPR_GAUN, 9, 4, null_hook(), S_GAUNTLETATK2_2, 0, 0},     // S_GAUNTLETATK2_1
    {SPR_GAUN, 10, 4, null_hook(), S_GAUNTLETATK2_3, 0, 0},    // S_GAUNTLETATK2_2
    {SPR_GAUN, 32779, 4, A_GauntletAttack, S_GAUNTLETATK2_4, 0, 0},     // S_GAUNTLETATK2_3
    {SPR_GAUN, 32780, 4, A_GauntletAttack, S_GAUNTLETATK2_5, 0, 0},     // S_GAUNTLETATK2_4
    {SPR_GAUN, 32781, 4, A_GauntletAttack, S_GAUNTLETATK2_6, 0, 0},     // S_GAUNTLETATK2_5
    {SPR_GAUN, 10, 4, A_ReFire, S_GAUNTLETATK2_7, 0, 0},        // S_GAUNTLETATK2_6
    {SPR_GAUN, 9, 4, A_Light0, S_GAUNTLETREADY2_1, 0, 0},       // S_GAUNTLETATK2_7
    {SPR_PUF1, 32768, 4, null_hook(), S_GAUNTLETPUFF1_2, 0, 0},        // S_GAUNTLETPUFF1_1
    {SPR_PUF1, 32769, 4, null_hook(), S_GAUNTLETPUFF1_3, 0, 0},        // S_GAUNTLETPUFF1_2
    {SPR_PUF1, 32770, 4, null_hook(), S_GAUNTLETPUFF1_4, 0, 0},        // S_GAUNTLETPUFF1_3
    {SPR_PUF1, 32771, 4, null_hook(), S_NULL, 0, 0},   // S_GAUNTLETPUFF1_4
    {SPR_PUF1, 32772, 4, null_hook(), S_GAUNTLETPUFF2_2, 0, 0},        // S_GAUNTLETPUFF2_1
    {SPR_PUF1, 32773, 4, null_hook(), S_GAUNTLETPUFF2_3, 0, 0},        // S_GAUNTLETPUFF2_2
    {SPR_PUF1, 32774, 4, null_hook(), S_GAUNTLETPUFF2_4, 0, 0},        // S_GAUNTLETPUFF2_3
    {SPR_PUF1, 32775, 4, null_hook(), S_NULL, 0, 0},   // S_GAUNTLETPUFF2_4
    {SPR_WBLS, 0, -1, null_hook(), S_NULL, 0, 0},      // S_BLSR
    {SPR_BLSR, 0, 1, A_WeaponReady, S_BLASTERREADY, 0, 0},      // S_BLASTERREADY
    {SPR_BLSR, 0, 1, A_Lower, S_BLASTERDOWN, 0, 0},     // S_BLASTERDOWN
    {SPR_BLSR, 0, 1, A_Raise, S_BLASTERUP, 0, 0},       // S_BLASTERUP
    {SPR_BLSR, 1, 3, null_hook(), S_BLASTERATK1_2, 0, 0},      // S_BLASTERATK1_1
    {SPR_BLSR, 2, 3, null_hook(), S_BLASTERATK1_3, 0, 0},      // S_BLASTERATK1_2
    {SPR_BLSR, 3, 2, A_FireBlasterPL1, S_BLASTERATK1_4, 0, 0},  // S_BLASTERATK1_3
    {SPR_BLSR, 2, 2, null_hook(), S_BLASTERATK1_5, 0, 0},      // S_BLASTERATK1_4
    {SPR_BLSR, 1, 2, null_hook(), S_BLASTERATK1_6, 0, 0},      // S_BLASTERATK1_5
    {SPR_BLSR, 0, 0, A_ReFire, S_BLASTERREADY, 0, 0},   // S_BLASTERATK1_6
    {SPR_BLSR, 1, 0, null_hook(), S_BLASTERATK2_2, 0, 0},      // S_BLASTERATK2_1
    {SPR_BLSR, 2, 0, null_hook(), S_BLASTERATK2_3, 0, 0},      // S_BLASTERATK2_2
    {SPR_BLSR, 3, 3, A_FireBlasterPL2, S_BLASTERATK2_4, 0, 0},  // S_BLASTERATK2_3
    {SPR_BLSR, 2, 4, null_hook(), S_BLASTERATK2_5, 0, 0},      // S_BLASTERATK2_4
    {SPR_BLSR, 1, 4, null_hook(), S_BLASTERATK2_6, 0, 0},      // S_BLASTERATK2_5
    {SPR_BLSR, 0, 0, A_ReFire, S_BLASTERREADY, 0, 0},   // S_BLASTERATK2_6
    {SPR_ACLO, 4, 200, null_hook(), S_BLASTERFX1_1, 0, 0},     // S_BLASTERFX1_1
    {SPR_FX18, 32768, 3, A_SpawnRippers, S_BLASTERFXI1_2, 0, 0},        // S_BLASTERFXI1_1
    {SPR_FX18, 32769, 3, null_hook(), S_BLASTERFXI1_3, 0, 0},  // S_BLASTERFXI1_2
    {SPR_FX18, 32770, 4, null_hook(), S_BLASTERFXI1_4, 0, 0},  // S_BLASTERFXI1_3
    {SPR_FX18, 32771, 4, null_hook(), S_BLASTERFXI1_5, 0, 0},  // S_BLASTERFXI1_4
    {SPR_FX18, 32772, 4, null_hook(), S_BLASTERFXI1_6, 0, 0},  // S_BLASTERFXI1_5
    {SPR_FX18, 32773, 4, null_hook(), S_BLASTERFXI1_7, 0, 0},  // S_BLASTERFXI1_6
    {SPR_FX18, 32774, 4, null_hook(), S_NULL, 0, 0},   // S_BLASTERFXI1_7
    {SPR_FX18, 7, 4, null_hook(), S_BLASTERSMOKE2, 0, 0},      // S_BLASTERSMOKE1
    {SPR_FX18, 8, 4, null_hook(), S_BLASTERSMOKE3, 0, 0},      // S_BLASTERSMOKE2
    {SPR_FX18, 9, 4, null_hook(), S_BLASTERSMOKE4, 0, 0},      // S_BLASTERSMOKE3
    {SPR_FX18, 10, 4, null_hook(), S_BLASTERSMOKE5, 0, 0},     // S_BLASTERSMOKE4
    {SPR_FX18, 11, 4, null_hook(), S_NULL, 0, 0},      // S_BLASTERSMOKE5
    {SPR_FX18, 12, 4, null_hook(), S_RIPPER2, 0, 0},   // S_RIPPER1
    {SPR_FX18, 13, 5, null_hook(), S_RIPPER1, 0, 0},   // S_RIPPER2
    {SPR_FX18, 32782, 4, null_hook(), S_RIPPERX2, 0, 0},       // S_RIPPERX1
    {SPR_FX18, 32783, 4, null_hook(), S_RIPPERX3, 0, 0},       // S_RIPPERX2
    {SPR_FX18, 32784, 4, null_hook(), S_RIPPERX4, 0, 0},       // S_RIPPERX3
    {SPR_FX18, 32785, 4, null_hook(), S_RIPPERX5, 0, 0},       // S_RIPPERX4
    {SPR_FX18, 32786, 4, null_hook(), S_NULL, 0, 0},   // S_RIPPERX5
    {SPR_FX17, 32768, 4, null_hook(), S_BLASTERPUFF1_2, 0, 0}, // S_BLASTERPUFF1_1
    {SPR_FX17, 32769, 4, null_hook(), S_BLASTERPUFF1_3, 0, 0}, // S_BLASTERPUFF1_2
    {SPR_FX17, 32770, 4, null_hook(), S_BLASTERPUFF1_4, 0, 0}, // S_BLASTERPUFF1_3
    {SPR_FX17, 32771, 4, null_hook(), S_BLASTERPUFF1_5, 0, 0}, // S_BLASTERPUFF1_4
    {SPR_FX17, 32772, 4, null_hook(), S_NULL, 0, 0},   // S_BLASTERPUFF1_5
    {SPR_FX17, 32773, 3, null_hook(), S_BLASTERPUFF2_2, 0, 0}, // S_BLASTERPUFF2_1
    {SPR_FX17, 32774, 3, null_hook(), S_BLASTERPUFF2_3, 0, 0}, // S_BLASTERPUFF2_2
    {SPR_FX17, 32775, 4, null_hook(), S_BLASTERPUFF2_4, 0, 0}, // S_BLASTERPUFF2_3
    {SPR_FX17, 32776, 4, null_hook(), S_BLASTERPUFF2_5, 0, 0}, // S_BLASTERPUFF2_4
    {SPR_FX17, 32777, 4, null_hook(), S_BLASTERPUFF2_6, 0, 0}, // S_BLASTERPUFF2_5
    {SPR_FX17, 32778, 4, null_hook(), S_BLASTERPUFF2_7, 0, 0}, // S_BLASTERPUFF2_6
    {SPR_FX17, 32779, 4, null_hook(), S_NULL, 0, 0},   // S_BLASTERPUFF2_7
    {SPR_WMCE, 0, -1, null_hook(), S_NULL, 0, 0},      // S_WMCE
    {SPR_MACE, 0, 1, A_WeaponReady, S_MACEREADY, 0, 0}, // S_MACEREADY
    {SPR_MACE, 0, 1, A_Lower, S_MACEDOWN, 0, 0},        // S_MACEDOWN
    {SPR_MACE, 0, 1, A_Raise, S_MACEUP, 0, 0},  // S_MACEUP
    {SPR_MACE, 1, 4, null_hook(), S_MACEATK1_2, 0, 0}, // S_MACEATK1_1
    {SPR_MACE, 2, 3, A_FireMacePL1, S_MACEATK1_3, 0, 0},        // S_MACEATK1_2
    {SPR_MACE, 3, 3, A_FireMacePL1, S_MACEATK1_4, 0, 0},        // S_MACEATK1_3
    {SPR_MACE, 4, 3, A_FireMacePL1, S_MACEATK1_5, 0, 0},        // S_MACEATK1_4
    {SPR_MACE, 5, 3, A_FireMacePL1, S_MACEATK1_6, 0, 0},        // S_MACEATK1_5
    {SPR_MACE, 2, 4, A_ReFire, S_MACEATK1_7, 0, 0},     // S_MACEATK1_6
    {SPR_MACE, 3, 4, null_hook(), S_MACEATK1_8, 0, 0}, // S_MACEATK1_7
    {SPR_MACE, 4, 4, null_hook(), S_MACEATK1_9, 0, 0}, // S_MACEATK1_8
    {SPR_MACE, 5, 4, null_hook(), S_MACEATK1_10, 0, 0},        // S_MACEATK1_9
    {SPR_MACE, 1, 4, null_hook(), S_MACEREADY, 0, 0},  // S_MACEATK1_10
    {SPR_MACE, 1, 4, null_hook(), S_MACEATK2_2, 0, 0}, // S_MACEATK2_1
    {SPR_MACE, 3, 4, A_FireMacePL2, S_MACEATK2_3, 0, 0},        // S_MACEATK2_2
    {SPR_MACE, 1, 4, null_hook(), S_MACEATK2_4, 0, 0}, // S_MACEATK2_3
    {SPR_MACE, 0, 8, A_ReFire, S_MACEREADY, 0, 0},      // S_MACEATK2_4
    {SPR_FX02, 0, 4, A_MacePL1Check, S_MACEFX1_2, 0, 0},        // S_MACEFX1_1
    {SPR_FX02, 1, 4, A_MacePL1Check, S_MACEFX1_1, 0, 0},        // S_MACEFX1_2
    {SPR_FX02, 32773, 4, A_MaceBallImpact, S_MACEFXI1_2, 0, 0}, // S_MACEFXI1_1
    {SPR_FX02, 32774, 4, null_hook(), S_MACEFXI1_3, 0, 0},     // S_MACEFXI1_2
    {SPR_FX02, 32775, 4, null_hook(), S_MACEFXI1_4, 0, 0},     // S_MACEFXI1_3
    {SPR_FX02, 32776, 4, null_hook(), S_MACEFXI1_5, 0, 0},     // S_MACEFXI1_4
    {SPR_FX02, 32777, 4, null_hook(), S_NULL, 0, 0},   // S_MACEFXI1_5
    {SPR_FX02, 2, 4, null_hook(), S_MACEFX2_2, 0, 0},  // S_MACEFX2_1
    {SPR_FX02, 3, 4, null_hook(), S_MACEFX2_1, 0, 0},  // S_MACEFX2_2
    {SPR_FX02, 32773, 4, A_MaceBallImpact2, S_MACEFXI1_2, 0, 0},        // S_MACEFXI2_1
    {SPR_FX02, 0, 4, null_hook(), S_MACEFX3_2, 0, 0},  // S_MACEFX3_1
    {SPR_FX02, 1, 4, null_hook(), S_MACEFX3_1, 0, 0},  // S_MACEFX3_2
    {SPR_FX02, 4, 99, null_hook(), S_MACEFX4_1, 0, 0}, // S_MACEFX4_1
    {SPR_FX02, 32770, 4, A_DeathBallImpact, S_MACEFXI1_2, 0, 0},        // S_MACEFXI4_1
    {SPR_WSKL, 0, -1, null_hook(), S_NULL, 0, 0},      // S_WSKL
    {SPR_HROD, 0, 1, A_WeaponReady, S_HORNRODREADY, 0, 0},      // S_HORNRODREADY
    {SPR_HROD, 0, 1, A_Lower, S_HORNRODDOWN, 0, 0},     // S_HORNRODDOWN
    {SPR_HROD, 0, 1, A_Raise, S_HORNRODUP, 0, 0},       // S_HORNRODUP
    {SPR_HROD, 0, 4, A_FireSkullRodPL1, S_HORNRODATK1_2, 0, 0}, // S_HORNRODATK1_1
    {SPR_HROD, 1, 4, A_FireSkullRodPL1, S_HORNRODATK1_3, 0, 0}, // S_HORNRODATK1_2
    {SPR_HROD, 1, 0, A_ReFire, S_HORNRODREADY, 0, 0},   // S_HORNRODATK1_3
    {SPR_HROD, 2, 2, null_hook(), S_HORNRODATK2_2, 0, 0},      // S_HORNRODATK2_1
    {SPR_HROD, 3, 3, null_hook(), S_HORNRODATK2_3, 0, 0},      // S_HORNRODATK2_2
    {SPR_HROD, 4, 2, null_hook(), S_HORNRODATK2_4, 0, 0},      // S_HORNRODATK2_3
    {SPR_HROD, 5, 3, null_hook(), S_HORNRODATK2_5, 0, 0},      // S_HORNRODATK2_4
    {SPR_HROD, 6, 4, A_FireSkullRodPL2, S_HORNRODATK2_6, 0, 0}, // S_HORNRODATK2_5
    {SPR_HROD, 5, 2, null_hook(), S_HORNRODATK2_7, 0, 0},      // S_HORNRODATK2_6
    {SPR_HROD, 4, 3, null_hook(), S_HORNRODATK2_8, 0, 0},      // S_HORNRODATK2_7
    {SPR_HROD, 3, 2, null_hook(), S_HORNRODATK2_9, 0, 0},      // S_HORNRODATK2_8
    {SPR_HROD, 2, 2, A_ReFire, S_HORNRODREADY, 0, 0},   // S_HORNRODATK2_9
    {SPR_FX00, 32768, 6, null_hook(), S_HRODFX1_2, 0, 0},      // S_HRODFX1_1
    {SPR_FX00, 32769, 6, null_hook(), S_HRODFX1_1, 0, 0},      // S_HRODFX1_2
    {SPR_FX00, 32775, 5, null_hook(), S_HRODFXI1_2, 0, 0},     // S_HRODFXI1_1
    {SPR_FX00, 32776, 5, null_hook(), S_HRODFXI1_3, 0, 0},     // S_HRODFXI1_2
    {SPR_FX00, 32777, 4, null_hook(), S_HRODFXI1_4, 0, 0},     // S_HRODFXI1_3
    {SPR_FX00, 32778, 4, null_hook(), S_HRODFXI1_5, 0, 0},     // S_HRODFXI1_4
    {SPR_FX00, 32779, 3, null_hook(), S_HRODFXI1_6, 0, 0},     // S_HRODFXI1_5
    {SPR_FX00, 32780, 3, null_hook(), S_NULL, 0, 0},   // S_HRODFXI1_6
    {SPR_FX00, 32770, 3, null_hook(), S_HRODFX2_2, 0, 0},      // S_HRODFX2_1
    {SPR_FX00, 32771, 3, A_SkullRodPL2Seek, S_HRODFX2_3, 0, 0}, // S_HRODFX2_2
    {SPR_FX00, 32772, 3, null_hook(), S_HRODFX2_4, 0, 0},      // S_HRODFX2_3
    {SPR_FX00, 32773, 3, A_SkullRodPL2Seek, S_HRODFX2_1, 0, 0}, // S_HRODFX2_4
    {SPR_FX00, 32775, 5, A_AddPlayerRain, S_HRODFXI2_2, 0, 0},  // S_HRODFXI2_1
    {SPR_FX00, 32776, 5, null_hook(), S_HRODFXI2_3, 0, 0},     // S_HRODFXI2_2
    {SPR_FX00, 32777, 4, null_hook(), S_HRODFXI2_4, 0, 0},     // S_HRODFXI2_3
    {SPR_FX00, 32778, 3, null_hook(), S_HRODFXI2_5, 0, 0},     // S_HRODFXI2_4
    {SPR_FX00, 32779, 3, null_hook(), S_HRODFXI2_6, 0, 0},     // S_HRODFXI2_5
    {SPR_FX00, 32780, 3, null_hook(), S_HRODFXI2_7, 0, 0},     // S_HRODFXI2_6
    {SPR_FX00, 6, 1, A_HideInCeiling, S_HRODFXI2_8, 0, 0},      // S_HRODFXI2_7
    {SPR_FX00, 6, 1, A_SkullRodStorm, S_HRODFXI2_8, 0, 0},      // S_HRODFXI2_8
    {SPR_FX20, 32768, -1, null_hook(), S_NULL, 0, 0},  // S_RAINPLR1_1
    {SPR_FX21, 32768, -1, null_hook(), S_NULL, 0, 0},  // S_RAINPLR2_1
    {SPR_FX22, 32768, -1, null_hook(), S_NULL, 0, 0},  // S_RAINPLR3_1
    {SPR_FX23, 32768, -1, null_hook(), S_NULL, 0, 0},  // S_RAINPLR4_1
    {SPR_FX20, 32769, 4, A_RainImpact, S_RAINPLR1X_2, 0, 0},    // S_RAINPLR1X_1
    {SPR_FX20, 32770, 4, null_hook(), S_RAINPLR1X_3, 0, 0},    // S_RAINPLR1X_2
    {SPR_FX20, 32771, 4, null_hook(), S_RAINPLR1X_4, 0, 0},    // S_RAINPLR1X_3
    {SPR_FX20, 32772, 4, null_hook(), S_RAINPLR1X_5, 0, 0},    // S_RAINPLR1X_4
    {SPR_FX20, 32773, 4, null_hook(), S_NULL, 0, 0},   // S_RAINPLR1X_5
    {SPR_FX21, 32769, 4, A_RainImpact, S_RAINPLR2X_2, 0, 0},    // S_RAINPLR2X_1
    {SPR_FX21, 32770, 4, null_hook(), S_RAINPLR2X_3, 0, 0},    // S_RAINPLR2X_2
    {SPR_FX21, 32771, 4, null_hook(), S_RAINPLR2X_4, 0, 0},    // S_RAINPLR2X_3
    {SPR_FX21, 32772, 4, null_hook(), S_RAINPLR2X_5, 0, 0},    // S_RAINPLR2X_4
    {SPR_FX21, 32773, 4, null_hook(), S_NULL, 0, 0},   // S_RAINPLR2X_5
    {SPR_FX22, 32769, 4, A_RainImpact, S_RAINPLR3X_2, 0, 0},    // S_RAINPLR3X_1
    {SPR_FX22, 32770, 4, null_hook(), S_RAINPLR3X_3, 0, 0},    // S_RAINPLR3X_2
    {SPR_FX22, 32771, 4, null_hook(), S_RAINPLR3X_4, 0, 0},    // S_RAINPLR3X_3
    {SPR_FX22, 32772, 4, null_hook(), S_RAINPLR3X_5, 0, 0},    // S_RAINPLR3X_4
    {SPR_FX22, 32773, 4, null_hook(), S_NULL, 0, 0},   // S_RAINPLR3X_5
    {SPR_FX23, 32769, 4, A_RainImpact, S_RAINPLR4X_2, 0, 0},    // S_RAINPLR4X_1
    {SPR_FX23, 32770, 4, null_hook(), S_RAINPLR4X_3, 0, 0},    // S_RAINPLR4X_2
    {SPR_FX23, 32771, 4, null_hook(), S_RAINPLR4X_4, 0, 0},    // S_RAINPLR4X_3
    {SPR_FX23, 32772, 4, null_hook(), S_RAINPLR4X_5, 0, 0},    // S_RAINPLR4X_4
    {SPR_FX23, 32773, 4, null_hook(), S_NULL, 0, 0},   // S_RAINPLR4X_5
    {SPR_FX20, 32774, 4, null_hook(), S_RAINAIRXPLR1_2, 0, 0}, // S_RAINAIRXPLR1_1
    {SPR_FX21, 32774, 4, null_hook(), S_RAINAIRXPLR2_2, 0, 0}, // S_RAINAIRXPLR2_1
    {SPR_FX22, 32774, 4, null_hook(), S_RAINAIRXPLR3_2, 0, 0}, // S_RAINAIRXPLR3_1
    {SPR_FX23, 32774, 4, null_hook(), S_RAINAIRXPLR4_2, 0, 0}, // S_RAINAIRXPLR4_1
    {SPR_FX20, 32775, 4, null_hook(), S_RAINAIRXPLR1_3, 0, 0}, // S_RAINAIRXPLR1_2
    {SPR_FX21, 32775, 4, null_hook(), S_RAINAIRXPLR2_3, 0, 0}, // S_RAINAIRXPLR2_2
    {SPR_FX22, 32775, 4, null_hook(), S_RAINAIRXPLR3_3, 0, 0}, // S_RAINAIRXPLR3_2
    {SPR_FX23, 32775, 4, null_hook(), S_RAINAIRXPLR4_3, 0, 0}, // S_RAINAIRXPLR4_2
    {SPR_FX20, 32776, 4, null_hook(), S_NULL, 0, 0},   // S_RAINAIRXPLR1_3
    {SPR_FX21, 32776, 4, null_hook(), S_NULL, 0, 0},   // S_RAINAIRXPLR2_3
    {SPR_FX22, 32776, 4, null_hook(), S_NULL, 0, 0},   // S_RAINAIRXPLR3_3
    {SPR_FX23, 32776, 4, null_hook(), S_NULL, 0, 0},   // S_RAINAIRXPLR4_3
    {SPR_GWND, 0, 1, A_WeaponReady, S_GOLDWANDREADY, 0, 0},     // S_GOLDWANDREADY
    {SPR_GWND, 0, 1, A_Lower, S_GOLDWANDDOWN, 0, 0},    // S_GOLDWANDDOWN
    {SPR_GWND, 0, 1, A_Raise, S_GOLDWANDUP, 0, 0},      // S_GOLDWANDUP
    {SPR_GWND, 1, 3, null_hook(), S_GOLDWANDATK1_2, 0, 0},     // S_GOLDWANDATK1_1
    {SPR_GWND, 2, 5, A_FireGoldWandPL1, S_GOLDWANDATK1_3, 0, 0},        // S_GOLDWANDATK1_2
    {SPR_GWND, 3, 3, null_hook(), S_GOLDWANDATK1_4, 0, 0},     // S_GOLDWANDATK1_3
    {SPR_GWND, 3, 0, A_ReFire, S_GOLDWANDREADY, 0, 0},  // S_GOLDWANDATK1_4
    {SPR_GWND, 1, 3, null_hook(), S_GOLDWANDATK2_2, 0, 0},     // S_GOLDWANDATK2_1
    {SPR_GWND, 2, 4, A_FireGoldWandPL2, S_GOLDWANDATK2_3, 0, 0},        // S_GOLDWANDATK2_2
    {SPR_GWND, 3, 3, null_hook(), S_GOLDWANDATK2_4, 0, 0},     // S_GOLDWANDATK2_3
    {SPR_GWND, 3, 0, A_ReFire, S_GOLDWANDREADY, 0, 0},  // S_GOLDWANDATK2_4
    {SPR_FX01, 32768, 6, null_hook(), S_GWANDFX1_2, 0, 0},     // S_GWANDFX1_1
    {SPR_FX01, 32769, 6, null_hook(), S_GWANDFX1_1, 0, 0},     // S_GWANDFX1_2
    {SPR_FX01, 32772, 3, null_hook(), S_GWANDFXI1_2, 0, 0},    // S_GWANDFXI1_1
    {SPR_FX01, 32773, 3, null_hook(), S_GWANDFXI1_3, 0, 0},    // S_GWANDFXI1_2
    {SPR_FX01, 32774, 3, null_hook(), S_GWANDFXI1_4, 0, 0},    // S_GWANDFXI1_3
    {SPR_FX01, 32775, 3, null_hook(), S_NULL, 0, 0},   // S_GWANDFXI1_4
    {SPR_FX01, 32770, 6, null_hook(), S_GWANDFX2_2, 0, 0},     // S_GWANDFX2_1
    {SPR_FX01, 32771, 6, null_hook(), S_GWANDFX2_1, 0, 0},     // S_GWANDFX2_2
    {SPR_PUF2, 32768, 3, null_hook(), S_GWANDPUFF1_2, 0, 0},   // S_GWANDPUFF1_1
    {SPR_PUF2, 32769, 3, null_hook(), S_GWANDPUFF1_3, 0, 0},   // S_GWANDPUFF1_2
    {SPR_PUF2, 32770, 3, null_hook(), S_GWANDPUFF1_4, 0, 0},   // S_GWANDPUFF1_3
    {SPR_PUF2, 32771, 3, null_hook(), S_GWANDPUFF1_5, 0, 0},   // S_GWANDPUFF1_4
    {SPR_PUF2, 32772, 3, null_hook(), S_NULL, 0, 0},   // S_GWANDPUFF1_5
    {SPR_WPHX, 0, -1, null_hook(), S_NULL, 0, 0},      // S_WPHX
    {SPR_PHNX, 0, 1, A_WeaponReady, S_PHOENIXREADY, 0, 0},      // S_PHOENIXREADY
    {SPR_PHNX, 0, 1, A_Lower, S_PHOENIXDOWN, 0, 0},     // S_PHOENIXDOWN
    {SPR_PHNX, 0, 1, A_Raise, S_PHOENIXUP, 0, 0},       // S_PHOENIXUP
    {SPR_PHNX, 1, 5, null_hook(), S_PHOENIXATK1_2, 0, 0},      // S_PHOENIXATK1_1
    {SPR_PHNX, 2, 7, A_FirePhoenixPL1, S_PHOENIXATK1_3, 0, 0},  // S_PHOENIXATK1_2
    {SPR_PHNX, 3, 4, null_hook(), S_PHOENIXATK1_4, 0, 0},      // S_PHOENIXATK1_3
    {SPR_PHNX, 1, 4, null_hook(), S_PHOENIXATK1_5, 0, 0},      // S_PHOENIXATK1_4
    {SPR_PHNX, 1, 0, A_ReFire, S_PHOENIXREADY, 0, 0},   // S_PHOENIXATK1_5
    {SPR_PHNX, 1, 3, A_InitPhoenixPL2, S_PHOENIXATK2_2, 0, 0},  // S_PHOENIXATK2_1
    {SPR_PHNX, 32770, 1, A_FirePhoenixPL2, S_PHOENIXATK2_3, 0, 0},      // S_PHOENIXATK2_2
    {SPR_PHNX, 1, 4, A_ReFire, S_PHOENIXATK2_4, 0, 0},  // S_PHOENIXATK2_3
    {SPR_PHNX, 1, 4, A_ShutdownPhoenixPL2, S_PHOENIXREADY, 0, 0},       // S_PHOENIXATK2_4
    {SPR_FX04, 32768, 4, A_PhoenixPuff, S_PHOENIXFX1_1, 0, 0},  // S_PHOENIXFX1_1
    {SPR_FX08, 32768, 6, A_Explode, S_PHOENIXFXI1_2, 0, 0},     // S_PHOENIXFXI1_1
    {SPR_FX08, 32769, 5, null_hook(), S_PHOENIXFXI1_3, 0, 0},  // S_PHOENIXFXI1_2
    {SPR_FX08, 32770, 5, null_hook(), S_PHOENIXFXI1_4, 0, 0},  // S_PHOENIXFXI1_3
    {SPR_FX08, 32771, 4, null_hook(), S_PHOENIXFXI1_5, 0, 0},  // S_PHOENIXFXI1_4
    {SPR_FX08, 32772, 4, null_hook(), S_PHOENIXFXI1_6, 0, 0},  // S_PHOENIXFXI1_5
    {SPR_FX08, 32773, 4, null_hook(), S_PHOENIXFXI1_7, 0, 0},  // S_PHOENIXFXI1_6
    {SPR_FX08, 32774, 4, null_hook(), S_PHOENIXFXI1_8, 0, 0},  // S_PHOENIXFXI1_7
    {SPR_FX08, 32775, 4, null_hook(), S_NULL, 0, 0},   // S_PHOENIXFXI1_8
    {SPR_FX08, 32776, 8, null_hook(), S_PHOENIXFXIX_1, 0, 0 }, // S_PHOENIXFXIX_1
    {SPR_FX08, 32777, 8, A_RemovedPhoenixFunc, S_PHOENIXFXIX_2, 0, 0 },  // S_PHOENIXFXIX_2
    {SPR_FX08, 32778, 8, null_hook(), S_NULL, 0, 0 },          // S_PHOENIXFXIX_3
    {SPR_FX04, 1, 4, null_hook(), S_PHOENIXPUFF2, 0, 0},       // S_PHOENIXPUFF1
    {SPR_FX04, 2, 4, null_hook(), S_PHOENIXPUFF3, 0, 0},       // S_PHOENIXPUFF2
    {SPR_FX04, 3, 4, null_hook(), S_PHOENIXPUFF4, 0, 0},       // S_PHOENIXPUFF3
    {SPR_FX04, 4, 4, null_hook(), S_PHOENIXPUFF5, 0, 0},       // S_PHOENIXPUFF4
    {SPR_FX04, 5, 4, null_hook(), S_NULL, 0, 0},       // S_PHOENIXPUFF5
    {SPR_FX09, 32768, 2, null_hook(), S_PHOENIXFX2_2, 0, 0},   // S_PHOENIXFX2_1
    {SPR_FX09, 32769, 2, null_hook(), S_PHOENIXFX2_3, 0, 0},   // S_PHOENIXFX2_2
    {SPR_FX09, 32768, 2, null_hook(), S_PHOENIXFX2_4, 0, 0},   // S_PHOENIXFX2_3
    {SPR_FX09, 32769, 2, null_hook(), S_PHOENIXFX2_5, 0, 0},   // S_PHOENIXFX2_4
    {SPR_FX09, 32768, 2, null_hook(), S_PHOENIXFX2_6, 0, 0},   // S_PHOENIXFX2_5
    {SPR_FX09, 32769, 2, A_FlameEnd, S_PHOENIXFX2_7, 0, 0},     // S_PHOENIXFX2_6
    {SPR_FX09, 32770, 2, null_hook(), S_PHOENIXFX2_8, 0, 0},   // S_PHOENIXFX2_7
    {SPR_FX09, 32771, 2, null_hook(), S_PHOENIXFX2_9, 0, 0},   // S_PHOENIXFX2_8
    {SPR_FX09, 32772, 2, null_hook(), S_PHOENIXFX2_10, 0, 0},  // S_PHOENIXFX2_9
    {SPR_FX09, 32773, 2, null_hook(), S_NULL, 0, 0},   // S_PHOENIXFX2_10
    {SPR_FX09, 32774, 3, null_hook(), S_PHOENIXFXI2_2, 0, 0},  // S_PHOENIXFXI2_1
    {SPR_FX09, 32775, 3, A_FloatPuff, S_PHOENIXFXI2_3, 0, 0},   // S_PHOENIXFXI2_2
    {SPR_FX09, 32776, 4, null_hook(), S_PHOENIXFXI2_4, 0, 0},  // S_PHOENIXFXI2_3
    {SPR_FX09, 32777, 5, null_hook(), S_PHOENIXFXI2_5, 0, 0},  // S_PHOENIXFXI2_4
    {SPR_FX09, 32778, 5, null_hook(), S_NULL, 0, 0},   // S_PHOENIXFXI2_5
    {SPR_WBOW, 0, -1, null_hook(), S_NULL, 0, 0},      // S_WBOW
    {SPR_CRBW, 0, 1, A_WeaponReady, S_CRBOW2, 0, 0},    // S_CRBOW1
    {SPR_CRBW, 0, 1, A_WeaponReady, S_CRBOW3, 0, 0},    // S_CRBOW2
    {SPR_CRBW, 0, 1, A_WeaponReady, S_CRBOW4, 0, 0},    // S_CRBOW3
    {SPR_CRBW, 0, 1, A_WeaponReady, S_CRBOW5, 0, 0},    // S_CRBOW4
    {SPR_CRBW, 0, 1, A_WeaponReady, S_CRBOW6, 0, 0},    // S_CRBOW5
    {SPR_CRBW, 0, 1, A_WeaponReady, S_CRBOW7, 0, 0},    // S_CRBOW6
    {SPR_CRBW, 1, 1, A_WeaponReady, S_CRBOW8, 0, 0},    // S_CRBOW7
    {SPR_CRBW, 1, 1, A_WeaponReady, S_CRBOW9, 0, 0},    // S_CRBOW8
    {SPR_CRBW, 1, 1, A_WeaponReady, S_CRBOW10, 0, 0},   // S_CRBOW9
    {SPR_CRBW, 1, 1, A_WeaponReady, S_CRBOW11, 0, 0},   // S_CRBOW10
    {SPR_CRBW, 1, 1, A_WeaponReady, S_CRBOW12, 0, 0},   // S_CRBOW11
    {SPR_CRBW, 1, 1, A_WeaponReady, S_CRBOW13, 0, 0},   // S_CRBOW12
    {SPR_CRBW, 2, 1, A_WeaponReady, S_CRBOW14, 0, 0},   // S_CRBOW13
    {SPR_CRBW, 2, 1, A_WeaponReady, S_CRBOW15, 0, 0},   // S_CRBOW14
    {SPR_CRBW, 2, 1, A_WeaponReady, S_CRBOW16, 0, 0},   // S_CRBOW15
    {SPR_CRBW, 2, 1, A_WeaponReady, S_CRBOW17, 0, 0},   // S_CRBOW16
    {SPR_CRBW, 2, 1, A_WeaponReady, S_CRBOW18, 0, 0},   // S_CRBOW17
    {SPR_CRBW, 2, 1, A_WeaponReady, S_CRBOW1, 0, 0},    // S_CRBOW18
    {SPR_CRBW, 0, 1, A_Lower, S_CRBOWDOWN, 0, 0},       // S_CRBOWDOWN
    {SPR_CRBW, 0, 1, A_Raise, S_CRBOWUP, 0, 0}, // S_CRBOWUP
    {SPR_CRBW, 3, 6, A_FireCrossbowPL1, S_CRBOWATK1_2, 0, 0},   // S_CRBOWATK1_1
    {SPR_CRBW, 4, 3, null_hook(), S_CRBOWATK1_3, 0, 0},        // S_CRBOWATK1_2
    {SPR_CRBW, 5, 3, null_hook(), S_CRBOWATK1_4, 0, 0},        // S_CRBOWATK1_3
    {SPR_CRBW, 6, 3, null_hook(), S_CRBOWATK1_5, 0, 0},        // S_CRBOWATK1_4
    {SPR_CRBW, 7, 3, null_hook(), S_CRBOWATK1_6, 0, 0},        // S_CRBOWATK1_5
    {SPR_CRBW, 0, 4, null_hook(), S_CRBOWATK1_7, 0, 0},        // S_CRBOWATK1_6
    {SPR_CRBW, 1, 4, null_hook(), S_CRBOWATK1_8, 0, 0},        // S_CRBOWATK1_7
    {SPR_CRBW, 2, 5, A_ReFire, S_CRBOW1, 0, 0}, // S_CRBOWATK1_8
    {SPR_CRBW, 3, 5, A_FireCrossbowPL2, S_CRBOWATK2_2, 0, 0},   // S_CRBOWATK2_1
    {SPR_CRBW, 4, 3, null_hook(), S_CRBOWATK2_3, 0, 0},        // S_CRBOWATK2_2
    {SPR_CRBW, 5, 2, null_hook(), S_CRBOWATK2_4, 0, 0},        // S_CRBOWATK2_3
    {SPR_CRBW, 6, 3, null_hook(), S_CRBOWATK2_5, 0, 0},        // S_CRBOWATK2_4
    {SPR_CRBW, 7, 2, null_hook(), S_CRBOWATK2_6, 0, 0},        // S_CRBOWATK2_5
    {SPR_CRBW, 0, 3, null_hook(), S_CRBOWATK2_7, 0, 0},        // S_CRBOWATK2_6
    {SPR_CRBW, 1, 3, null_hook(), S_CRBOWATK2_8, 0, 0},        // S_CRBOWATK2_7
    {SPR_CRBW, 2, 4, A_ReFire, S_CRBOW1, 0, 0}, // S_CRBOWATK2_8
    {SPR_FX03, 32769, 1, null_hook(), S_CRBOWFX1, 0, 0},       // S_CRBOWFX1
    {SPR_FX03, 32775, 8, null_hook(), S_CRBOWFXI1_2, 0, 0},    // S_CRBOWFXI1_1
    {SPR_FX03, 32776, 8, null_hook(), S_CRBOWFXI1_3, 0, 0},    // S_CRBOWFXI1_2
    {SPR_FX03, 32777, 8, null_hook(), S_NULL, 0, 0},   // S_CRBOWFXI1_3
    {SPR_FX03, 32769, 1, A_BoltSpark, S_CRBOWFX2, 0, 0},        // S_CRBOWFX2
    {SPR_FX03, 32768, 1, null_hook(), S_CRBOWFX3, 0, 0},       // S_CRBOWFX3
    {SPR_FX03, 32770, 8, null_hook(), S_CRBOWFXI3_2, 0, 0},    // S_CRBOWFXI3_1
    {SPR_FX03, 32771, 8, null_hook(), S_CRBOWFXI3_3, 0, 0},    // S_CRBOWFXI3_2
    {SPR_FX03, 32772, 8, null_hook(), S_NULL, 0, 0},   // S_CRBOWFXI3_3
    {SPR_FX03, 32773, 8, null_hook(), S_CRBOWFX4_2, 0, 0},     // S_CRBOWFX4_1
    {SPR_FX03, 32774, 8, null_hook(), S_NULL, 0, 0},   // S_CRBOWFX4_2
    {SPR_BLOD, 2, 8, null_hook(), S_BLOOD2, 0, 0},     // S_BLOOD1
    {SPR_BLOD, 1, 8, null_hook(), S_BLOOD3, 0, 0},     // S_BLOOD2
    {SPR_BLOD, 0, 8, null_hook(), S_NULL, 0, 0},       // S_BLOOD3
    {SPR_BLOD, 2, 8, null_hook(), S_BLOODSPLATTER2, 0, 0},     // S_BLOODSPLATTER1
    {SPR_BLOD, 1, 8, null_hook(), S_BLOODSPLATTER3, 0, 0},     // S_BLOODSPLATTER2
    {SPR_BLOD, 0, 8, null_hook(), S_NULL, 0, 0},       // S_BLOODSPLATTER3
    {SPR_BLOD, 0, 6, null_hook(), S_NULL, 0, 0},       // S_BLOODSPLATTERX
    {SPR_PLAY, 0, -1, null_hook(), S_NULL, 0, 0},      // S_PLAY
    {SPR_PLAY, 0, 4, null_hook(), S_PLAY_RUN2, 0, 0},  // S_PLAY_RUN1
    {SPR_PLAY, 1, 4, null_hook(), S_PLAY_RUN3, 0, 0},  // S_PLAY_RUN2
    {SPR_PLAY, 2, 4, null_hook(), S_PLAY_RUN4, 0, 0},  // S_PLAY_RUN3
    {SPR_PLAY, 3, 4, null_hook(), S_PLAY_RUN1, 0, 0},  // S_PLAY_RUN4
    {SPR_PLAY, 4, 12, null_hook(), S_PLAY, 0, 0},      // S_PLAY_ATK1
    {SPR_PLAY, 32773, 6, null_hook(), S_PLAY_ATK1, 0, 0},      // S_PLAY_ATK2
    {SPR_PLAY, 6, 4, null_hook(), S_PLAY_PAIN2, 0, 0}, // S_PLAY_PAIN
    {SPR_PLAY, 6, 4, A_Pain, S_PLAY, 0, 0},     // S_PLAY_PAIN2
    {SPR_PLAY, 7, 6, null_hook(), S_PLAY_DIE2, 0, 0},  // S_PLAY_DIE1
    {SPR_PLAY, 8, 6, A_Scream, S_PLAY_DIE3, 0, 0},      // S_PLAY_DIE2
    {SPR_PLAY, 9, 6, null_hook(), S_PLAY_DIE4, 0, 0},  // S_PLAY_DIE3
    {SPR_PLAY, 10, 6, null_hook(), S_PLAY_DIE5, 0, 0}, // S_PLAY_DIE4
    {SPR_PLAY, 11, 6, A_NoBlocking, S_PLAY_DIE6, 0, 0}, // S_PLAY_DIE5
    {SPR_PLAY, 12, 6, null_hook(), S_PLAY_DIE7, 0, 0}, // S_PLAY_DIE6
    {SPR_PLAY, 13, 6, null_hook(), S_PLAY_DIE8, 0, 0}, // S_PLAY_DIE7
    {SPR_PLAY, 14, 6, null_hook(), S_PLAY_DIE9, 0, 0}, // S_PLAY_DIE8
    {SPR_PLAY, 15, -1, A_AddPlayerCorpse, S_NULL, 0, 0},        // S_PLAY_DIE9
    {SPR_PLAY, 16, 5, A_Scream, S_PLAY_XDIE2, 0, 0},    // S_PLAY_XDIE1
    {SPR_PLAY, 17, 5, A_SkullPop, S_PLAY_XDIE3, 0, 0},  // S_PLAY_XDIE2
    {SPR_PLAY, 18, 5, A_NoBlocking, S_PLAY_XDIE4, 0, 0},        // S_PLAY_XDIE3
    {SPR_PLAY, 19, 5, null_hook(), S_PLAY_XDIE5, 0, 0},        // S_PLAY_XDIE4
    {SPR_PLAY, 20, 5, null_hook(), S_PLAY_XDIE6, 0, 0},        // S_PLAY_XDIE5
    {SPR_PLAY, 21, 5, null_hook(), S_PLAY_XDIE7, 0, 0},        // S_PLAY_XDIE6
    {SPR_PLAY, 22, 5, null_hook(), S_PLAY_XDIE8, 0, 0},        // S_PLAY_XDIE7
    {SPR_PLAY, 23, 5, null_hook(), S_PLAY_XDIE9, 0, 0},        // S_PLAY_XDIE8
    {SPR_PLAY, 24, -1, A_AddPlayerCorpse, S_NULL, 0, 0},        // S_PLAY_XDIE9
    {SPR_FDTH, 32768, 5, A_FlameSnd, S_PLAY_FDTH2, 0, 0},       // S_PLAY_FDTH1
    {SPR_FDTH, 32769, 4, null_hook(), S_PLAY_FDTH3, 0, 0},     // S_PLAY_FDTH2
    {SPR_FDTH, 32770, 5, null_hook(), S_PLAY_FDTH4, 0, 0},     // S_PLAY_FDTH3
    {SPR_FDTH, 32771, 4, A_Scream, S_PLAY_FDTH5, 0, 0}, // S_PLAY_FDTH4
    {SPR_FDTH, 32772, 5, null_hook(), S_PLAY_FDTH6, 0, 0},     // S_PLAY_FDTH5
    {SPR_FDTH, 32773, 4, null_hook(), S_PLAY_FDTH7, 0, 0},     // S_PLAY_FDTH6
    {SPR_FDTH, 32774, 5, A_FlameSnd, S_PLAY_FDTH8, 0, 0},       // S_PLAY_FDTH7
    {SPR_FDTH, 32775, 4, null_hook(), S_PLAY_FDTH9, 0, 0},     // S_PLAY_FDTH8
    {SPR_FDTH, 32776, 5, null_hook(), S_PLAY_FDTH10, 0, 0},    // S_PLAY_FDTH9
    {SPR_FDTH, 32777, 4, null_hook(), S_PLAY_FDTH11, 0, 0},    // S_PLAY_FDTH10
    {SPR_FDTH, 32778, 5, null_hook(), S_PLAY_FDTH12, 0, 0},    // S_PLAY_FDTH11
    {SPR_FDTH, 32779, 4, null_hook(), S_PLAY_FDTH13, 0, 0},    // S_PLAY_FDTH12
    {SPR_FDTH, 32780, 5, null_hook(), S_PLAY_FDTH14, 0, 0},    // S_PLAY_FDTH13
    {SPR_FDTH, 32781, 4, null_hook(), S_PLAY_FDTH15, 0, 0},    // S_PLAY_FDTH14
    {SPR_FDTH, 32782, 5, A_NoBlocking, S_PLAY_FDTH16, 0, 0},    // S_PLAY_FDTH15
    {SPR_FDTH, 32783, 4, null_hook(), S_PLAY_FDTH17, 0, 0},    // S_PLAY_FDTH16
    {SPR_FDTH, 32784, 5, null_hook(), S_PLAY_FDTH18, 0, 0},    // S_PLAY_FDTH17
    {SPR_FDTH, 32785, 4, null_hook(), S_PLAY_FDTH19, 0, 0},    // S_PLAY_FDTH18
    {SPR_ACLO, 4, 35, A_CheckBurnGone, S_PLAY_FDTH19, 0, 0},    // S_PLAY_FDTH19
    {SPR_ACLO, 4, 8, null_hook(), S_NULL, 0, 0},       // S_PLAY_FDTH20
    {SPR_BSKL, 0, 5, A_CheckSkullFloor, S_BLOODYSKULL2, 0, 0},  // S_BLOODYSKULL1
    {SPR_BSKL, 1, 5, A_CheckSkullFloor, S_BLOODYSKULL3, 0, 0},  // S_BLOODYSKULL2
    {SPR_BSKL, 2, 5, A_CheckSkullFloor, S_BLOODYSKULL4, 0, 0},  // S_BLOODYSKULL3
    {SPR_BSKL, 3, 5, A_CheckSkullFloor, S_BLOODYSKULL5, 0, 0},  // S_BLOODYSKULL4
    {SPR_BSKL, 4, 5, A_CheckSkullFloor, S_BLOODYSKULL1, 0, 0},  // S_BLOODYSKULL5
    {SPR_BSKL, 5, 16, A_CheckSkullDone, S_BLOODYSKULLX1, 0, 0}, // S_BLOODYSKULLX1
    {SPR_BSKL, 5, 1050, null_hook(), S_NULL, 0, 0},    // S_BLOODYSKULLX2
    {SPR_CHKN, 0, -1, null_hook(), S_NULL, 0, 0},      // S_CHICPLAY
    {SPR_CHKN, 0, 3, null_hook(), S_CHICPLAY_RUN2, 0, 0},      // S_CHICPLAY_RUN1
    {SPR_CHKN, 1, 3, null_hook(), S_CHICPLAY_RUN3, 0, 0},      // S_CHICPLAY_RUN2
    {SPR_CHKN, 0, 3, null_hook(), S_CHICPLAY_RUN4, 0, 0},      // S_CHICPLAY_RUN3
    {SPR_CHKN, 1, 3, null_hook(), S_CHICPLAY_RUN1, 0, 0},      // S_CHICPLAY_RUN4
    {SPR_CHKN, 2, 12, null_hook(), S_CHICPLAY, 0, 0},  // S_CHICPLAY_ATK1
    {SPR_CHKN, 3, 4, A_Feathers, S_CHICPLAY_PAIN2, 0, 0},       // S_CHICPLAY_PAIN
    {SPR_CHKN, 2, 4, A_Pain, S_CHICPLAY, 0, 0}, // S_CHICPLAY_PAIN2
    {SPR_CHKN, 0, 10, A_ChicLook, S_CHICKEN_LOOK2, 0, 0},       // S_CHICKEN_LOOK1
    {SPR_CHKN, 1, 10, A_ChicLook, S_CHICKEN_LOOK1, 0, 0},       // S_CHICKEN_LOOK2
    {SPR_CHKN, 0, 3, A_ChicChase, S_CHICKEN_WALK2, 0, 0},       // S_CHICKEN_WALK1
    {SPR_CHKN, 1, 3, A_ChicChase, S_CHICKEN_WALK1, 0, 0},       // S_CHICKEN_WALK2
    {SPR_CHKN, 3, 5, A_Feathers, S_CHICKEN_PAIN2, 0, 0},        // S_CHICKEN_PAIN1
    {SPR_CHKN, 2, 5, A_ChicPain, S_CHICKEN_WALK1, 0, 0},        // S_CHICKEN_PAIN2
    {SPR_CHKN, 0, 8, A_FaceTarget, S_CHICKEN_ATK2, 0, 0},       // S_CHICKEN_ATK1
    {SPR_CHKN, 2, 10, A_ChicAttack, S_CHICKEN_WALK1, 0, 0},     // S_CHICKEN_ATK2
    {SPR_CHKN, 4, 6, A_Scream, S_CHICKEN_DIE2, 0, 0},   // S_CHICKEN_DIE1
    {SPR_CHKN, 5, 6, A_Feathers, S_CHICKEN_DIE3, 0, 0}, // S_CHICKEN_DIE2
    {SPR_CHKN, 6, 6, null_hook(), S_CHICKEN_DIE4, 0, 0},       // S_CHICKEN_DIE3
    {SPR_CHKN, 7, 6, A_NoBlocking, S_CHICKEN_DIE5, 0, 0},       // S_CHICKEN_DIE4
    {SPR_CHKN, 8, 6, null_hook(), S_CHICKEN_DIE6, 0, 0},       // S_CHICKEN_DIE5
    {SPR_CHKN, 9, 6, null_hook(), S_CHICKEN_DIE7, 0, 0},       // S_CHICKEN_DIE6
    {SPR_CHKN, 10, 6, null_hook(), S_CHICKEN_DIE8, 0, 0},      // S_CHICKEN_DIE7
    {SPR_CHKN, 11, -1, null_hook(), S_NULL, 0, 0},     // S_CHICKEN_DIE8
    {SPR_CHKN, 12, 3, null_hook(), S_FEATHER2, 0, 0},  // S_FEATHER1
    {SPR_CHKN, 13, 3, null_hook(), S_FEATHER3, 0, 0},  // S_FEATHER2
    {SPR_CHKN, 14, 3, null_hook(), S_FEATHER4, 0, 0},  // S_FEATHER3
    {SPR_CHKN, 15, 3, null_hook(), S_FEATHER5, 0, 0},  // S_FEATHER4
    {SPR_CHKN, 16, 3, null_hook(), S_FEATHER6, 0, 0},  // S_FEATHER5
    {SPR_CHKN, 15, 3, null_hook(), S_FEATHER7, 0, 0},  // S_FEATHER6
    {SPR_CHKN, 14, 3, null_hook(), S_FEATHER8, 0, 0},  // S_FEATHER7
    {SPR_CHKN, 13, 3, null_hook(), S_FEATHER1, 0, 0},  // S_FEATHER8
    {SPR_CHKN, 13, 6, null_hook(), S_NULL, 0, 0},      // S_FEATHERX
    {SPR_MUMM, 0, 10, A_Look, S_MUMMY_LOOK2, 0, 0},     // S_MUMMY_LOOK1
    {SPR_MUMM, 1, 10, A_Look, S_MUMMY_LOOK1, 0, 0},     // S_MUMMY_LOOK2
    {SPR_MUMM, 0, 4, A_Chase, S_MUMMY_WALK2, 0, 0},     // S_MUMMY_WALK1
    {SPR_MUMM, 1, 4, A_Chase, S_MUMMY_WALK3, 0, 0},     // S_MUMMY_WALK2
    {SPR_MUMM, 2, 4, A_Chase, S_MUMMY_WALK4, 0, 0},     // S_MUMMY_WALK3
    {SPR_MUMM, 3, 4, A_Chase, S_MUMMY_WALK1, 0, 0},     // S_MUMMY_WALK4
    {SPR_MUMM, 4, 6, A_FaceTarget, S_MUMMY_ATK2, 0, 0}, // S_MUMMY_ATK1
    {SPR_MUMM, 5, 6, A_MummyAttack, S_MUMMY_ATK3, 0, 0},        // S_MUMMY_ATK2
    {SPR_MUMM, 6, 6, A_FaceTarget, S_MUMMY_WALK1, 0, 0},        // S_MUMMY_ATK3
    {SPR_MUMM, 23, 5, A_FaceTarget, S_MUMMYL_ATK2, 0, 0},       // S_MUMMYL_ATK1
    {SPR_MUMM, 32792, 5, A_FaceTarget, S_MUMMYL_ATK3, 0, 0},    // S_MUMMYL_ATK2
    {SPR_MUMM, 23, 5, A_FaceTarget, S_MUMMYL_ATK4, 0, 0},       // S_MUMMYL_ATK3
    {SPR_MUMM, 32792, 5, A_FaceTarget, S_MUMMYL_ATK5, 0, 0},    // S_MUMMYL_ATK4
    {SPR_MUMM, 23, 5, A_FaceTarget, S_MUMMYL_ATK6, 0, 0},       // S_MUMMYL_ATK5
    {SPR_MUMM, 32792, 15, A_MummyAttack2, S_MUMMY_WALK1, 0, 0}, // S_MUMMYL_ATK6
    {SPR_MUMM, 7, 4, null_hook(), S_MUMMY_PAIN2, 0, 0},        // S_MUMMY_PAIN1
    {SPR_MUMM, 7, 4, A_Pain, S_MUMMY_WALK1, 0, 0},      // S_MUMMY_PAIN2
    {SPR_MUMM, 8, 5, null_hook(), S_MUMMY_DIE2, 0, 0}, // S_MUMMY_DIE1
    {SPR_MUMM, 9, 5, A_Scream, S_MUMMY_DIE3, 0, 0},     // S_MUMMY_DIE2
    {SPR_MUMM, 10, 5, A_MummySoul, S_MUMMY_DIE4, 0, 0}, // S_MUMMY_DIE3
    {SPR_MUMM, 11, 5, null_hook(), S_MUMMY_DIE5, 0, 0},        // S_MUMMY_DIE4
    {SPR_MUMM, 12, 5, A_NoBlocking, S_MUMMY_DIE6, 0, 0},        // S_MUMMY_DIE5
    {SPR_MUMM, 13, 5, null_hook(), S_MUMMY_DIE7, 0, 0},        // S_MUMMY_DIE6
    {SPR_MUMM, 14, 5, null_hook(), S_MUMMY_DIE8, 0, 0},        // S_MUMMY_DIE7
    {SPR_MUMM, 15, -1, null_hook(), S_NULL, 0, 0},     // S_MUMMY_DIE8
    {SPR_MUMM, 16, 5, null_hook(), S_MUMMY_SOUL2, 0, 0},       // S_MUMMY_SOUL1
    {SPR_MUMM, 17, 5, null_hook(), S_MUMMY_SOUL3, 0, 0},       // S_MUMMY_SOUL2
    {SPR_MUMM, 18, 5, null_hook(), S_MUMMY_SOUL4, 0, 0},       // S_MUMMY_SOUL3
    {SPR_MUMM, 19, 9, null_hook(), S_MUMMY_SOUL5, 0, 0},       // S_MUMMY_SOUL4
    {SPR_MUMM, 20, 5, null_hook(), S_MUMMY_SOUL6, 0, 0},       // S_MUMMY_SOUL5
    {SPR_MUMM, 21, 5, null_hook(), S_MUMMY_SOUL7, 0, 0},       // S_MUMMY_SOUL6
    {SPR_MUMM, 22, 5, null_hook(), S_NULL, 0, 0},      // S_MUMMY_SOUL7
    {SPR_FX15, 32768, 5, A_ContMobjSound, S_MUMMYFX1_2, 0, 0},  // S_MUMMYFX1_1
    {SPR_FX15, 32769, 5, A_MummyFX1Seek, S_MUMMYFX1_3, 0, 0},   // S_MUMMYFX1_2
    {SPR_FX15, 32770, 5, null_hook(), S_MUMMYFX1_4, 0, 0},     // S_MUMMYFX1_3
    {SPR_FX15, 32769, 5, A_MummyFX1Seek, S_MUMMYFX1_1, 0, 0},   // S_MUMMYFX1_4
    {SPR_FX15, 32771, 5, null_hook(), S_MUMMYFXI1_2, 0, 0},    // S_MUMMYFXI1_1
    {SPR_FX15, 32772, 5, null_hook(), S_MUMMYFXI1_3, 0, 0},    // S_MUMMYFXI1_2
    {SPR_FX15, 32773, 5, null_hook(), S_MUMMYFXI1_4, 0, 0},    // S_MUMMYFXI1_3
    {SPR_FX15, 32774, 5, null_hook(), S_NULL, 0, 0},   // S_MUMMYFXI1_4
    {SPR_BEAS, 0, 10, A_Look, S_BEAST_LOOK2, 0, 0},     // S_BEAST_LOOK1
    {SPR_BEAS, 1, 10, A_Look, S_BEAST_LOOK1, 0, 0},     // S_BEAST_LOOK2
    {SPR_BEAS, 0, 3, A_Chase, S_BEAST_WALK2, 0, 0},     // S_BEAST_WALK1
    {SPR_BEAS, 1, 3, A_Chase, S_BEAST_WALK3, 0, 0},     // S_BEAST_WALK2
    {SPR_BEAS, 2, 3, A_Chase, S_BEAST_WALK4, 0, 0},     // S_BEAST_WALK3
    {SPR_BEAS, 3, 3, A_Chase, S_BEAST_WALK5, 0, 0},     // S_BEAST_WALK4
    {SPR_BEAS, 4, 3, A_Chase, S_BEAST_WALK6, 0, 0},     // S_BEAST_WALK5
    {SPR_BEAS, 5, 3, A_Chase, S_BEAST_WALK1, 0, 0},     // S_BEAST_WALK6
    {SPR_BEAS, 7, 10, A_FaceTarget, S_BEAST_ATK2, 0, 0},        // S_BEAST_ATK1
    {SPR_BEAS, 8, 10, A_BeastAttack, S_BEAST_WALK1, 0, 0},      // S_BEAST_ATK2
    {SPR_BEAS, 6, 3, null_hook(), S_BEAST_PAIN2, 0, 0},        // S_BEAST_PAIN1
    {SPR_BEAS, 6, 3, A_Pain, S_BEAST_WALK1, 0, 0},      // S_BEAST_PAIN2
    {SPR_BEAS, 17, 6, null_hook(), S_BEAST_DIE2, 0, 0},        // S_BEAST_DIE1
    {SPR_BEAS, 18, 6, A_Scream, S_BEAST_DIE3, 0, 0},    // S_BEAST_DIE2
    {SPR_BEAS, 19, 6, null_hook(), S_BEAST_DIE4, 0, 0},        // S_BEAST_DIE3
    {SPR_BEAS, 20, 6, null_hook(), S_BEAST_DIE5, 0, 0},        // S_BEAST_DIE4
    {SPR_BEAS, 21, 6, null_hook(), S_BEAST_DIE6, 0, 0},        // S_BEAST_DIE5
    {SPR_BEAS, 22, 6, A_NoBlocking, S_BEAST_DIE7, 0, 0},        // S_BEAST_DIE6
    {SPR_BEAS, 23, 6, null_hook(), S_BEAST_DIE8, 0, 0},        // S_BEAST_DIE7
    {SPR_BEAS, 24, 6, null_hook(), S_BEAST_DIE9, 0, 0},        // S_BEAST_DIE8
    {SPR_BEAS, 25, -1, null_hook(), S_NULL, 0, 0},     // S_BEAST_DIE9
    {SPR_BEAS, 9, 5, null_hook(), S_BEAST_XDIE2, 0, 0},        // S_BEAST_XDIE1
    {SPR_BEAS, 10, 6, A_Scream, S_BEAST_XDIE3, 0, 0},   // S_BEAST_XDIE2
    {SPR_BEAS, 11, 5, null_hook(), S_BEAST_XDIE4, 0, 0},       // S_BEAST_XDIE3
    {SPR_BEAS, 12, 6, null_hook(), S_BEAST_XDIE5, 0, 0},       // S_BEAST_XDIE4
    {SPR_BEAS, 13, 5, null_hook(), S_BEAST_XDIE6, 0, 0},       // S_BEAST_XDIE5
    {SPR_BEAS, 14, 6, A_NoBlocking, S_BEAST_XDIE7, 0, 0},       // S_BEAST_XDIE6
    {SPR_BEAS, 15, 5, null_hook(), S_BEAST_XDIE8, 0, 0},       // S_BEAST_XDIE7
    {SPR_BEAS, 16, -1, null_hook(), S_NULL, 0, 0},     // S_BEAST_XDIE8
    {SPR_FRB1, 0, 2, A_BeastPuff, S_BEASTBALL2, 0, 0},  // S_BEASTBALL1
    {SPR_FRB1, 0, 2, A_BeastPuff, S_BEASTBALL3, 0, 0},  // S_BEASTBALL2
    {SPR_FRB1, 1, 2, A_BeastPuff, S_BEASTBALL4, 0, 0},  // S_BEASTBALL3
    {SPR_FRB1, 1, 2, A_BeastPuff, S_BEASTBALL5, 0, 0},  // S_BEASTBALL4
    {SPR_FRB1, 2, 2, A_BeastPuff, S_BEASTBALL6, 0, 0},  // S_BEASTBALL5
    {SPR_FRB1, 2, 2, A_BeastPuff, S_BEASTBALL1, 0, 0},  // S_BEASTBALL6
    {SPR_FRB1, 3, 4, null_hook(), S_BEASTBALLX2, 0, 0},        // S_BEASTBALLX1
    {SPR_FRB1, 4, 4, null_hook(), S_BEASTBALLX3, 0, 0},        // S_BEASTBALLX2
    {SPR_FRB1, 5, 4, null_hook(), S_BEASTBALLX4, 0, 0},        // S_BEASTBALLX3
    {SPR_FRB1, 6, 4, null_hook(), S_BEASTBALLX5, 0, 0},        // S_BEASTBALLX4
    {SPR_FRB1, 7, 4, null_hook(), S_NULL, 0, 0},       // S_BEASTBALLX5
    {SPR_FRB1, 0, 4, null_hook(), S_BURNBALL2, 0, 0},  // S_BURNBALL1
    {SPR_FRB1, 1, 4, null_hook(), S_BURNBALL3, 0, 0},  // S_BURNBALL2
    {SPR_FRB1, 2, 4, null_hook(), S_BURNBALL4, 0, 0},  // S_BURNBALL3
    {SPR_FRB1, 3, 4, null_hook(), S_BURNBALL5, 0, 0},  // S_BURNBALL4
    {SPR_FRB1, 4, 4, null_hook(), S_BURNBALL6, 0, 0},  // S_BURNBALL5
    {SPR_FRB1, 5, 4, null_hook(), S_BURNBALL7, 0, 0},  // S_BURNBALL6
    {SPR_FRB1, 6, 4, null_hook(), S_BURNBALL8, 0, 0},  // S_BURNBALL7
    {SPR_FRB1, 7, 4, null_hook(), S_NULL, 0, 0},       // S_BURNBALL8
    {SPR_FRB1, 32768, 4, null_hook(), S_BURNBALLFB2, 0, 0},    // S_BURNBALLFB1
    {SPR_FRB1, 32769, 4, null_hook(), S_BURNBALLFB3, 0, 0},    // S_BURNBALLFB2
    {SPR_FRB1, 32770, 4, null_hook(), S_BURNBALLFB4, 0, 0},    // S_BURNBALLFB3
    {SPR_FRB1, 32771, 4, null_hook(), S_BURNBALLFB5, 0, 0},    // S_BURNBALLFB4
    {SPR_FRB1, 32772, 4, null_hook(), S_BURNBALLFB6, 0, 0},    // S_BURNBALLFB5
    {SPR_FRB1, 32773, 4, null_hook(), S_BURNBALLFB7, 0, 0},    // S_BURNBALLFB6
    {SPR_FRB1, 32774, 4, null_hook(), S_BURNBALLFB8, 0, 0},    // S_BURNBALLFB7
    {SPR_FRB1, 32775, 4, null_hook(), S_NULL, 0, 0},   // S_BURNBALLFB8
    {SPR_FRB1, 3, 4, null_hook(), S_PUFFY2, 0, 0},     // S_PUFFY1
    {SPR_FRB1, 4, 4, null_hook(), S_PUFFY3, 0, 0},     // S_PUFFY2
    {SPR_FRB1, 5, 4, null_hook(), S_PUFFY4, 0, 0},     // S_PUFFY3
    {SPR_FRB1, 6, 4, null_hook(), S_PUFFY5, 0, 0},     // S_PUFFY4
    {SPR_FRB1, 7, 4, null_hook(), S_NULL, 0, 0},       // S_PUFFY5
    {SPR_SNKE, 0, 10, A_Look, S_SNAKE_LOOK2, 0, 0},     // S_SNAKE_LOOK1
    {SPR_SNKE, 1, 10, A_Look, S_SNAKE_LOOK1, 0, 0},     // S_SNAKE_LOOK2
    {SPR_SNKE, 0, 4, A_Chase, S_SNAKE_WALK2, 0, 0},     // S_SNAKE_WALK1
    {SPR_SNKE, 1, 4, A_Chase, S_SNAKE_WALK3, 0, 0},     // S_SNAKE_WALK2
    {SPR_SNKE, 2, 4, A_Chase, S_SNAKE_WALK4, 0, 0},     // S_SNAKE_WALK3
    {SPR_SNKE, 3, 4, A_Chase, S_SNAKE_WALK1, 0, 0},     // S_SNAKE_WALK4
    {SPR_SNKE, 5, 5, A_FaceTarget, S_SNAKE_ATK2, 0, 0}, // S_SNAKE_ATK1
    {SPR_SNKE, 5, 5, A_FaceTarget, S_SNAKE_ATK3, 0, 0}, // S_SNAKE_ATK2
    {SPR_SNKE, 5, 4, A_SnakeAttack, S_SNAKE_ATK4, 0, 0},        // S_SNAKE_ATK3
    {SPR_SNKE, 5, 4, A_SnakeAttack, S_SNAKE_ATK5, 0, 0},        // S_SNAKE_ATK4
    {SPR_SNKE, 5, 4, A_SnakeAttack, S_SNAKE_ATK6, 0, 0},        // S_SNAKE_ATK5
    {SPR_SNKE, 5, 5, A_FaceTarget, S_SNAKE_ATK7, 0, 0}, // S_SNAKE_ATK6
    {SPR_SNKE, 5, 5, A_FaceTarget, S_SNAKE_ATK8, 0, 0}, // S_SNAKE_ATK7
    {SPR_SNKE, 5, 5, A_FaceTarget, S_SNAKE_ATK9, 0, 0}, // S_SNAKE_ATK8
    {SPR_SNKE, 5, 4, A_SnakeAttack2, S_SNAKE_WALK1, 0, 0},      // S_SNAKE_ATK9
    {SPR_SNKE, 4, 3, null_hook(), S_SNAKE_PAIN2, 0, 0},        // S_SNAKE_PAIN1
    {SPR_SNKE, 4, 3, A_Pain, S_SNAKE_WALK1, 0, 0},      // S_SNAKE_PAIN2
    {SPR_SNKE, 6, 5, null_hook(), S_SNAKE_DIE2, 0, 0}, // S_SNAKE_DIE1
    {SPR_SNKE, 7, 5, A_Scream, S_SNAKE_DIE3, 0, 0},     // S_SNAKE_DIE2
    {SPR_SNKE, 8, 5, null_hook(), S_SNAKE_DIE4, 0, 0}, // S_SNAKE_DIE3
    {SPR_SNKE, 9, 5, null_hook(), S_SNAKE_DIE5, 0, 0}, // S_SNAKE_DIE4
    {SPR_SNKE, 10, 5, null_hook(), S_SNAKE_DIE6, 0, 0},        // S_SNAKE_DIE5
    {SPR_SNKE, 11, 5, null_hook(), S_SNAKE_DIE7, 0, 0},        // S_SNAKE_DIE6
    {SPR_SNKE, 12, 5, A_NoBlocking, S_SNAKE_DIE8, 0, 0},        // S_SNAKE_DIE7
    {SPR_SNKE, 13, 5, null_hook(), S_SNAKE_DIE9, 0, 0},        // S_SNAKE_DIE8
    {SPR_SNKE, 14, 5, null_hook(), S_SNAKE_DIE10, 0, 0},       // S_SNAKE_DIE9
    {SPR_SNKE, 15, -1, null_hook(), S_NULL, 0, 0},     // S_SNAKE_DIE10
    {SPR_SNFX, 32768, 5, null_hook(), S_SNAKEPRO_A2, 0, 0},    // S_SNAKEPRO_A1
    {SPR_SNFX, 32769, 5, null_hook(), S_SNAKEPRO_A3, 0, 0},    // S_SNAKEPRO_A2
    {SPR_SNFX, 32770, 5, null_hook(), S_SNAKEPRO_A4, 0, 0},    // S_SNAKEPRO_A3
    {SPR_SNFX, 32771, 5, null_hook(), S_SNAKEPRO_A1, 0, 0},    // S_SNAKEPRO_A4
    {SPR_SNFX, 32772, 5, null_hook(), S_SNAKEPRO_AX2, 0, 0},   // S_SNAKEPRO_AX1
    {SPR_SNFX, 32773, 5, null_hook(), S_SNAKEPRO_AX3, 0, 0},   // S_SNAKEPRO_AX2
    {SPR_SNFX, 32774, 4, null_hook(), S_SNAKEPRO_AX4, 0, 0},   // S_SNAKEPRO_AX3
    {SPR_SNFX, 32775, 3, null_hook(), S_SNAKEPRO_AX5, 0, 0},   // S_SNAKEPRO_AX4
    {SPR_SNFX, 32776, 3, null_hook(), S_NULL, 0, 0},   // S_SNAKEPRO_AX5
    {SPR_SNFX, 32777, 6, null_hook(), S_SNAKEPRO_B2, 0, 0},    // S_SNAKEPRO_B1
    {SPR_SNFX, 32778, 6, null_hook(), S_SNAKEPRO_B1, 0, 0},    // S_SNAKEPRO_B2
    {SPR_SNFX, 32779, 5, null_hook(), S_SNAKEPRO_BX2, 0, 0},   // S_SNAKEPRO_BX1
    {SPR_SNFX, 32780, 5, null_hook(), S_SNAKEPRO_BX3, 0, 0},   // S_SNAKEPRO_BX2
    {SPR_SNFX, 32781, 4, null_hook(), S_SNAKEPRO_BX4, 0, 0},   // S_SNAKEPRO_BX3
    {SPR_SNFX, 32782, 3, null_hook(), S_NULL, 0, 0},   // S_SNAKEPRO_BX4
    {SPR_HEAD, 0, 10, A_Look, S_HEAD_LOOK, 0, 0},       // S_HEAD_LOOK
    {SPR_HEAD, 0, 4, A_Chase, S_HEAD_FLOAT, 0, 0},      // S_HEAD_FLOAT
    {SPR_HEAD, 0, 5, A_FaceTarget, S_HEAD_ATK2, 0, 0},  // S_HEAD_ATK1
    {SPR_HEAD, 1, 20, A_HeadAttack, S_HEAD_FLOAT, 0, 0},        // S_HEAD_ATK2
    {SPR_HEAD, 0, 4, null_hook(), S_HEAD_PAIN2, 0, 0}, // S_HEAD_PAIN1
    {SPR_HEAD, 0, 4, A_Pain, S_HEAD_FLOAT, 0, 0},       // S_HEAD_PAIN2
    {SPR_HEAD, 2, 7, null_hook(), S_HEAD_DIE2, 0, 0},  // S_HEAD_DIE1
    {SPR_HEAD, 3, 7, A_Scream, S_HEAD_DIE3, 0, 0},      // S_HEAD_DIE2
    {SPR_HEAD, 4, 7, null_hook(), S_HEAD_DIE4, 0, 0},  // S_HEAD_DIE3
    {SPR_HEAD, 5, 7, null_hook(), S_HEAD_DIE5, 0, 0},  // S_HEAD_DIE4
    {SPR_HEAD, 6, 7, A_NoBlocking, S_HEAD_DIE6, 0, 0},  // S_HEAD_DIE5
    {SPR_HEAD, 7, 7, null_hook(), S_HEAD_DIE7, 0, 0},  // S_HEAD_DIE6
    {SPR_HEAD, 8, -1, A_BossDeath, S_NULL, 0, 0},       // S_HEAD_DIE7
    {SPR_FX05, 0, 6, null_hook(), S_HEADFX1_2, 0, 0},  // S_HEADFX1_1
    {SPR_FX05, 1, 6, null_hook(), S_HEADFX1_3, 0, 0},  // S_HEADFX1_2
    {SPR_FX05, 2, 6, null_hook(), S_HEADFX1_1, 0, 0},  // S_HEADFX1_3
    {SPR_FX05, 3, 5, A_HeadIceImpact, S_HEADFXI1_2, 0, 0},      // S_HEADFXI1_1
    {SPR_FX05, 4, 5, null_hook(), S_HEADFXI1_3, 0, 0}, // S_HEADFXI1_2
    {SPR_FX05, 5, 5, null_hook(), S_HEADFXI1_4, 0, 0}, // S_HEADFXI1_3
    {SPR_FX05, 6, 5, null_hook(), S_NULL, 0, 0},       // S_HEADFXI1_4
    {SPR_FX05, 7, 6, null_hook(), S_HEADFX2_2, 0, 0},  // S_HEADFX2_1
    {SPR_FX05, 8, 6, null_hook(), S_HEADFX2_3, 0, 0},  // S_HEADFX2_2
    {SPR_FX05, 9, 6, null_hook(), S_HEADFX2_1, 0, 0},  // S_HEADFX2_3
    {SPR_FX05, 3, 5, null_hook(), S_HEADFXI2_2, 0, 0}, // S_HEADFXI2_1
    {SPR_FX05, 4, 5, null_hook(), S_HEADFXI2_3, 0, 0}, // S_HEADFXI2_2
    {SPR_FX05, 5, 5, null_hook(), S_HEADFXI2_4, 0, 0}, // S_HEADFXI2_3
    {SPR_FX05, 6, 5, null_hook(), S_NULL, 0, 0},       // S_HEADFXI2_4
    {SPR_FX06, 0, 4, A_HeadFireGrow, S_HEADFX3_2, 0, 0},        // S_HEADFX3_1
    {SPR_FX06, 1, 4, A_HeadFireGrow, S_HEADFX3_3, 0, 0},        // S_HEADFX3_2
    {SPR_FX06, 2, 4, A_HeadFireGrow, S_HEADFX3_1, 0, 0},        // S_HEADFX3_3
    {SPR_FX06, 0, 5, null_hook(), S_HEADFX3_5, 0, 0},  // S_HEADFX3_4
    {SPR_FX06, 1, 5, null_hook(), S_HEADFX3_6, 0, 0},  // S_HEADFX3_5
    {SPR_FX06, 2, 5, null_hook(), S_HEADFX3_4, 0, 0},  // S_HEADFX3_6
    {SPR_FX06, 3, 5, null_hook(), S_HEADFXI3_2, 0, 0}, // S_HEADFXI3_1
    {SPR_FX06, 4, 5, null_hook(), S_HEADFXI3_3, 0, 0}, // S_HEADFXI3_2
    {SPR_FX06, 5, 5, null_hook(), S_HEADFXI3_4, 0, 0}, // S_HEADFXI3_3
    {SPR_FX06, 6, 5, null_hook(), S_NULL, 0, 0},       // S_HEADFXI3_4
    {SPR_FX07, 3, 3, null_hook(), S_HEADFX4_2, 0, 0},  // S_HEADFX4_1
    {SPR_FX07, 4, 3, null_hook(), S_HEADFX4_3, 0, 0},  // S_HEADFX4_2
    {SPR_FX07, 5, 3, null_hook(), S_HEADFX4_4, 0, 0},  // S_HEADFX4_3
    {SPR_FX07, 6, 3, null_hook(), S_HEADFX4_5, 0, 0},  // S_HEADFX4_4
    {SPR_FX07, 0, 3, A_WhirlwindSeek, S_HEADFX4_6, 0, 0},       // S_HEADFX4_5
    {SPR_FX07, 1, 3, A_WhirlwindSeek, S_HEADFX4_7, 0, 0},       // S_HEADFX4_6
    {SPR_FX07, 2, 3, A_WhirlwindSeek, S_HEADFX4_5, 0, 0},       // S_HEADFX4_7
    {SPR_FX07, 6, 4, null_hook(), S_HEADFXI4_2, 0, 0}, // S_HEADFXI4_1
    {SPR_FX07, 5, 4, null_hook(), S_HEADFXI4_3, 0, 0}, // S_HEADFXI4_2
    {SPR_FX07, 4, 4, null_hook(), S_HEADFXI4_4, 0, 0}, // S_HEADFXI4_3
    {SPR_FX07, 3, 4, null_hook(), S_NULL, 0, 0},       // S_HEADFXI4_4
    {SPR_CLNK, 0, 10, A_Look, S_CLINK_LOOK2, 0, 0},     // S_CLINK_LOOK1
    {SPR_CLNK, 1, 10, A_Look, S_CLINK_LOOK1, 0, 0},     // S_CLINK_LOOK2
    {SPR_CLNK, 0, 3, A_Chase, S_CLINK_WALK2, 0, 0},     // S_CLINK_WALK1
    {SPR_CLNK, 1, 3, A_Chase, S_CLINK_WALK3, 0, 0},     // S_CLINK_WALK2
    {SPR_CLNK, 2, 3, A_Chase, S_CLINK_WALK4, 0, 0},     // S_CLINK_WALK3
    {SPR_CLNK, 3, 3, A_Chase, S_CLINK_WALK1, 0, 0},     // S_CLINK_WALK4
    {SPR_CLNK, 4, 5, A_FaceTarget, S_CLINK_ATK2, 0, 0}, // S_CLINK_ATK1
    {SPR_CLNK, 5, 4, A_FaceTarget, S_CLINK_ATK3, 0, 0}, // S_CLINK_ATK2
    {SPR_CLNK, 6, 7, A_ClinkAttack, S_CLINK_WALK1, 0, 0},       // S_CLINK_ATK3
    {SPR_CLNK, 7, 3, null_hook(), S_CLINK_PAIN2, 0, 0},        // S_CLINK_PAIN1
    {SPR_CLNK, 7, 3, A_Pain, S_CLINK_WALK1, 0, 0},      // S_CLINK_PAIN2
    {SPR_CLNK, 8, 6, null_hook(), S_CLINK_DIE2, 0, 0}, // S_CLINK_DIE1
    {SPR_CLNK, 9, 6, null_hook(), S_CLINK_DIE3, 0, 0}, // S_CLINK_DIE2
    {SPR_CLNK, 10, 5, A_Scream, S_CLINK_DIE4, 0, 0},    // S_CLINK_DIE3
    {SPR_CLNK, 11, 5, A_NoBlocking, S_CLINK_DIE5, 0, 0},        // S_CLINK_DIE4
    {SPR_CLNK, 12, 5, null_hook(), S_CLINK_DIE6, 0, 0},        // S_CLINK_DIE5
    {SPR_CLNK, 13, 5, null_hook(), S_CLINK_DIE7, 0, 0},        // S_CLINK_DIE6
    {SPR_CLNK, 14, -1, null_hook(), S_NULL, 0, 0},     // S_CLINK_DIE7
    {SPR_WZRD, 0, 10, A_Look, S_WIZARD_LOOK2, 0, 0},    // S_WIZARD_LOOK1
    {SPR_WZRD, 1, 10, A_Look, S_WIZARD_LOOK1, 0, 0},    // S_WIZARD_LOOK2
    {SPR_WZRD, 0, 3, A_Chase, S_WIZARD_WALK2, 0, 0},    // S_WIZARD_WALK1
    {SPR_WZRD, 0, 4, A_Chase, S_WIZARD_WALK3, 0, 0},    // S_WIZARD_WALK2
    {SPR_WZRD, 0, 3, A_Chase, S_WIZARD_WALK4, 0, 0},    // S_WIZARD_WALK3
    {SPR_WZRD, 0, 4, A_Chase, S_WIZARD_WALK5, 0, 0},    // S_WIZARD_WALK4
    {SPR_WZRD, 1, 3, A_Chase, S_WIZARD_WALK6, 0, 0},    // S_WIZARD_WALK5
    {SPR_WZRD, 1, 4, A_Chase, S_WIZARD_WALK7, 0, 0},    // S_WIZARD_WALK6
    {SPR_WZRD, 1, 3, A_Chase, S_WIZARD_WALK8, 0, 0},    // S_WIZARD_WALK7
    {SPR_WZRD, 1, 4, A_Chase, S_WIZARD_WALK1, 0, 0},    // S_WIZARD_WALK8
    {SPR_WZRD, 2, 4, A_WizAtk1, S_WIZARD_ATK2, 0, 0},   // S_WIZARD_ATK1
    {SPR_WZRD, 2, 4, A_WizAtk2, S_WIZARD_ATK3, 0, 0},   // S_WIZARD_ATK2
    {SPR_WZRD, 2, 4, A_WizAtk1, S_WIZARD_ATK4, 0, 0},   // S_WIZARD_ATK3
    {SPR_WZRD, 2, 4, A_WizAtk2, S_WIZARD_ATK5, 0, 0},   // S_WIZARD_ATK4
    {SPR_WZRD, 2, 4, A_WizAtk1, S_WIZARD_ATK6, 0, 0},   // S_WIZARD_ATK5
    {SPR_WZRD, 2, 4, A_WizAtk2, S_WIZARD_ATK7, 0, 0},   // S_WIZARD_ATK6
    {SPR_WZRD, 2, 4, A_WizAtk1, S_WIZARD_ATK8, 0, 0},   // S_WIZARD_ATK7
    {SPR_WZRD, 2, 4, A_WizAtk2, S_WIZARD_ATK9, 0, 0},   // S_WIZARD_ATK8
    {SPR_WZRD, 3, 12, A_WizAtk3, S_WIZARD_WALK1, 0, 0}, // S_WIZARD_ATK9
    {SPR_WZRD, 4, 3, A_GhostOff, S_WIZARD_PAIN2, 0, 0}, // S_WIZARD_PAIN1
    {SPR_WZRD, 4, 3, A_Pain, S_WIZARD_WALK1, 0, 0},     // S_WIZARD_PAIN2
    {SPR_WZRD, 5, 6, A_GhostOff, S_WIZARD_DIE2, 0, 0},  // S_WIZARD_DIE1
    {SPR_WZRD, 6, 6, A_Scream, S_WIZARD_DIE3, 0, 0},    // S_WIZARD_DIE2
    {SPR_WZRD, 7, 6, null_hook(), S_WIZARD_DIE4, 0, 0},        // S_WIZARD_DIE3
    {SPR_WZRD, 8, 6, null_hook(), S_WIZARD_DIE5, 0, 0},        // S_WIZARD_DIE4
    {SPR_WZRD, 9, 6, A_NoBlocking, S_WIZARD_DIE6, 0, 0},        // S_WIZARD_DIE5
    {SPR_WZRD, 10, 6, null_hook(), S_WIZARD_DIE7, 0, 0},       // S_WIZARD_DIE6
    {SPR_WZRD, 11, 6, null_hook(), S_WIZARD_DIE8, 0, 0},       // S_WIZARD_DIE7
    {SPR_WZRD, 12, -1, null_hook(), S_NULL, 0, 0},     // S_WIZARD_DIE8
    {SPR_FX11, 32768, 6, null_hook(), S_WIZFX1_2, 0, 0},       // S_WIZFX1_1
    {SPR_FX11, 32769, 6, null_hook(), S_WIZFX1_1, 0, 0},       // S_WIZFX1_2
    {SPR_FX11, 32770, 5, null_hook(), S_WIZFXI1_2, 0, 0},      // S_WIZFXI1_1
    {SPR_FX11, 32771, 5, null_hook(), S_WIZFXI1_3, 0, 0},      // S_WIZFXI1_2
    {SPR_FX11, 32772, 5, null_hook(), S_WIZFXI1_4, 0, 0},      // S_WIZFXI1_3
    {SPR_FX11, 32773, 5, null_hook(), S_WIZFXI1_5, 0, 0},      // S_WIZFXI1_4
    {SPR_FX11, 32774, 5, null_hook(), S_NULL, 0, 0},   // S_WIZFXI1_5
    {SPR_IMPX, 0, 10, A_Look, S_IMP_LOOK2, 0, 0},       // S_IMP_LOOK1
    {SPR_IMPX, 1, 10, A_Look, S_IMP_LOOK3, 0, 0},       // S_IMP_LOOK2
    {SPR_IMPX, 2, 10, A_Look, S_IMP_LOOK4, 0, 0},       // S_IMP_LOOK3
    {SPR_IMPX, 1, 10, A_Look, S_IMP_LOOK1, 0, 0},       // S_IMP_LOOK4
    {SPR_IMPX, 0, 3, A_Chase, S_IMP_FLY2, 0, 0},        // S_IMP_FLY1
    {SPR_IMPX, 0, 3, A_Chase, S_IMP_FLY3, 0, 0},        // S_IMP_FLY2
    {SPR_IMPX, 1, 3, A_Chase, S_IMP_FLY4, 0, 0},        // S_IMP_FLY3
    {SPR_IMPX, 1, 3, A_Chase, S_IMP_FLY5, 0, 0},        // S_IMP_FLY4
    {SPR_IMPX, 2, 3, A_Chase, S_IMP_FLY6, 0, 0},        // S_IMP_FLY5
    {SPR_IMPX, 2, 3, A_Chase, S_IMP_FLY7, 0, 0},        // S_IMP_FLY6
    {SPR_IMPX, 1, 3, A_Chase, S_IMP_FLY8, 0, 0},        // S_IMP_FLY7
    {SPR_IMPX, 1, 3, A_Chase, S_IMP_FLY1, 0, 0},        // S_IMP_FLY8
    {SPR_IMPX, 3, 6, A_FaceTarget, S_IMP_MEATK2, 0, 0}, // S_IMP_MEATK1
    {SPR_IMPX, 4, 6, A_FaceTarget, S_IMP_MEATK3, 0, 0}, // S_IMP_MEATK2
    {SPR_IMPX, 5, 6, A_ImpMeAttack, S_IMP_FLY1, 0, 0},  // S_IMP_MEATK3
    {SPR_IMPX, 0, 10, A_FaceTarget, S_IMP_MSATK1_2, 0, 0},      // S_IMP_MSATK1_1
    {SPR_IMPX, 1, 6, A_ImpMsAttack, S_IMP_MSATK1_3, 0, 0},      // S_IMP_MSATK1_2
    {SPR_IMPX, 2, 6, null_hook(), S_IMP_MSATK1_4, 0, 0},       // S_IMP_MSATK1_3
    {SPR_IMPX, 1, 6, null_hook(), S_IMP_MSATK1_5, 0, 0},       // S_IMP_MSATK1_4
    {SPR_IMPX, 0, 6, null_hook(), S_IMP_MSATK1_6, 0, 0},       // S_IMP_MSATK1_5
    {SPR_IMPX, 1, 6, null_hook(), S_IMP_MSATK1_3, 0, 0},       // S_IMP_MSATK1_6
    {SPR_IMPX, 3, 6, A_FaceTarget, S_IMP_MSATK2_2, 0, 0},       // S_IMP_MSATK2_1
    {SPR_IMPX, 4, 6, A_FaceTarget, S_IMP_MSATK2_3, 0, 0},       // S_IMP_MSATK2_2
    {SPR_IMPX, 5, 6, A_ImpMsAttack2, S_IMP_FLY1, 0, 0}, // S_IMP_MSATK2_3
    {SPR_IMPX, 6, 3, null_hook(), S_IMP_PAIN2, 0, 0},  // S_IMP_PAIN1
    {SPR_IMPX, 6, 3, A_Pain, S_IMP_FLY1, 0, 0}, // S_IMP_PAIN2
    {SPR_IMPX, 6, 4, A_ImpDeath, S_IMP_DIE2, 0, 0},     // S_IMP_DIE1
    {SPR_IMPX, 7, 5, null_hook(), S_IMP_DIE2, 0, 0},   // S_IMP_DIE2
    {SPR_IMPX, 18, 5, A_ImpXDeath1, S_IMP_XDIE2, 0, 0}, // S_IMP_XDIE1
    {SPR_IMPX, 19, 5, null_hook(), S_IMP_XDIE3, 0, 0}, // S_IMP_XDIE2
    {SPR_IMPX, 20, 5, null_hook(), S_IMP_XDIE4, 0, 0}, // S_IMP_XDIE3
    {SPR_IMPX, 21, 5, A_ImpXDeath2, S_IMP_XDIE5, 0, 0}, // S_IMP_XDIE4
    {SPR_IMPX, 22, 5, null_hook(), S_IMP_XDIE5, 0, 0}, // S_IMP_XDIE5
    {SPR_IMPX, 8, 7, A_ImpExplode, S_IMP_CRASH2, 0, 0}, // S_IMP_CRASH1
    {SPR_IMPX, 9, 7, A_Scream, S_IMP_CRASH3, 0, 0},     // S_IMP_CRASH2
    {SPR_IMPX, 10, 7, null_hook(), S_IMP_CRASH4, 0, 0},        // S_IMP_CRASH3
    {SPR_IMPX, 11, -1, null_hook(), S_NULL, 0, 0},     // S_IMP_CRASH4
    {SPR_IMPX, 23, 7, null_hook(), S_IMP_XCRASH2, 0, 0},       // S_IMP_XCRASH1
    {SPR_IMPX, 24, 7, null_hook(), S_IMP_XCRASH3, 0, 0},       // S_IMP_XCRASH2
    {SPR_IMPX, 25, -1, null_hook(), S_NULL, 0, 0},     // S_IMP_XCRASH3
    {SPR_IMPX, 12, 5, null_hook(), S_IMP_CHUNKA2, 0, 0},       // S_IMP_CHUNKA1
    {SPR_IMPX, 13, 700, null_hook(), S_IMP_CHUNKA3, 0, 0},     // S_IMP_CHUNKA2
    {SPR_IMPX, 14, 700, null_hook(), S_NULL, 0, 0},    // S_IMP_CHUNKA3
    {SPR_IMPX, 15, 5, null_hook(), S_IMP_CHUNKB2, 0, 0},       // S_IMP_CHUNKB1
    {SPR_IMPX, 16, 700, null_hook(), S_IMP_CHUNKB3, 0, 0},     // S_IMP_CHUNKB2
    {SPR_IMPX, 17, 700, null_hook(), S_NULL, 0, 0},    // S_IMP_CHUNKB3
    {SPR_FX10, 32768, 6, null_hook(), S_IMPFX2, 0, 0}, // S_IMPFX1
    {SPR_FX10, 32769, 6, null_hook(), S_IMPFX3, 0, 0}, // S_IMPFX2
    {SPR_FX10, 32770, 6, null_hook(), S_IMPFX1, 0, 0}, // S_IMPFX3
    {SPR_FX10, 32771, 5, null_hook(), S_IMPFXI2, 0, 0},        // S_IMPFXI1
    {SPR_FX10, 32772, 5, null_hook(), S_IMPFXI3, 0, 0},        // S_IMPFXI2
    {SPR_FX10, 32773, 5, null_hook(), S_IMPFXI4, 0, 0},        // S_IMPFXI3
    {SPR_FX10, 32774, 5, null_hook(), S_NULL, 0, 0},   // S_IMPFXI4
    {SPR_KNIG, 0, 10, A_Look, S_KNIGHT_STND2, 0, 0},    // S_KNIGHT_STND1
    {SPR_KNIG, 1, 10, A_Look, S_KNIGHT_STND1, 0, 0},    // S_KNIGHT_STND2
    {SPR_KNIG, 0, 4, A_Chase, S_KNIGHT_WALK2, 0, 0},    // S_KNIGHT_WALK1
    {SPR_KNIG, 1, 4, A_Chase, S_KNIGHT_WALK3, 0, 0},    // S_KNIGHT_WALK2
    {SPR_KNIG, 2, 4, A_Chase, S_KNIGHT_WALK4, 0, 0},    // S_KNIGHT_WALK3
    {SPR_KNIG, 3, 4, A_Chase, S_KNIGHT_WALK1, 0, 0},    // S_KNIGHT_WALK4
    {SPR_KNIG, 4, 10, A_FaceTarget, S_KNIGHT_ATK2, 0, 0},       // S_KNIGHT_ATK1
    {SPR_KNIG, 5, 8, A_FaceTarget, S_KNIGHT_ATK3, 0, 0},        // S_KNIGHT_ATK2
    {SPR_KNIG, 6, 8, A_KnightAttack, S_KNIGHT_ATK4, 0, 0},      // S_KNIGHT_ATK3
    {SPR_KNIG, 4, 10, A_FaceTarget, S_KNIGHT_ATK5, 0, 0},       // S_KNIGHT_ATK4
    {SPR_KNIG, 5, 8, A_FaceTarget, S_KNIGHT_ATK6, 0, 0},        // S_KNIGHT_ATK5
    {SPR_KNIG, 6, 8, A_KnightAttack, S_KNIGHT_WALK1, 0, 0},     // S_KNIGHT_ATK6
    {SPR_KNIG, 7, 3, null_hook(), S_KNIGHT_PAIN2, 0, 0},       // S_KNIGHT_PAIN1
    {SPR_KNIG, 7, 3, A_Pain, S_KNIGHT_WALK1, 0, 0},     // S_KNIGHT_PAIN2
    {SPR_KNIG, 8, 6, null_hook(), S_KNIGHT_DIE2, 0, 0},        // S_KNIGHT_DIE1
    {SPR_KNIG, 9, 6, A_Scream, S_KNIGHT_DIE3, 0, 0},    // S_KNIGHT_DIE2
    {SPR_KNIG, 10, 6, null_hook(), S_KNIGHT_DIE4, 0, 0},       // S_KNIGHT_DIE3
    {SPR_KNIG, 11, 6, A_NoBlocking, S_KNIGHT_DIE5, 0, 0},       // S_KNIGHT_DIE4
    {SPR_KNIG, 12, 6, null_hook(), S_KNIGHT_DIE6, 0, 0},       // S_KNIGHT_DIE5
    {SPR_KNIG, 13, 6, null_hook(), S_KNIGHT_DIE7, 0, 0},       // S_KNIGHT_DIE6
    {SPR_KNIG, 14, -1, null_hook(), S_NULL, 0, 0},     // S_KNIGHT_DIE7
    {SPR_SPAX, 32768, 3, A_ContMobjSound, S_SPINAXE2, 0, 0},    // S_SPINAXE1
    {SPR_SPAX, 32769, 3, null_hook(), S_SPINAXE3, 0, 0},       // S_SPINAXE2
    {SPR_SPAX, 32770, 3, null_hook(), S_SPINAXE1, 0, 0},       // S_SPINAXE3
    {SPR_SPAX, 32771, 6, null_hook(), S_SPINAXEX2, 0, 0},      // S_SPINAXEX1
    {SPR_SPAX, 32772, 6, null_hook(), S_SPINAXEX3, 0, 0},      // S_SPINAXEX2
    {SPR_SPAX, 32773, 6, null_hook(), S_NULL, 0, 0},   // S_SPINAXEX3
    {SPR_RAXE, 32768, 5, A_DripBlood, S_REDAXE2, 0, 0}, // S_REDAXE1
    {SPR_RAXE, 32769, 5, A_DripBlood, S_REDAXE1, 0, 0}, // S_REDAXE2
    {SPR_RAXE, 32770, 6, null_hook(), S_REDAXEX2, 0, 0},       // S_REDAXEX1
    {SPR_RAXE, 32771, 6, null_hook(), S_REDAXEX3, 0, 0},       // S_REDAXEX2
    {SPR_RAXE, 32772, 6, null_hook(), S_NULL, 0, 0},   // S_REDAXEX3
    {SPR_SRCR, 0, 10, A_Look, S_SRCR1_LOOK2, 0, 0},     // S_SRCR1_LOOK1
    {SPR_SRCR, 1, 10, A_Look, S_SRCR1_LOOK1, 0, 0},     // S_SRCR1_LOOK2
    {SPR_SRCR, 0, 5, A_Sor1Chase, S_SRCR1_WALK2, 0, 0}, // S_SRCR1_WALK1
    {SPR_SRCR, 1, 5, A_Sor1Chase, S_SRCR1_WALK3, 0, 0}, // S_SRCR1_WALK2
    {SPR_SRCR, 2, 5, A_Sor1Chase, S_SRCR1_WALK4, 0, 0}, // S_SRCR1_WALK3
    {SPR_SRCR, 3, 5, A_Sor1Chase, S_SRCR1_WALK1, 0, 0}, // S_SRCR1_WALK4
    {SPR_SRCR, 16, 6, A_Sor1Pain, S_SRCR1_WALK1, 0, 0}, // S_SRCR1_PAIN1
    {SPR_SRCR, 16, 7, A_FaceTarget, S_SRCR1_ATK2, 0, 0},        // S_SRCR1_ATK1
    {SPR_SRCR, 17, 6, A_FaceTarget, S_SRCR1_ATK3, 0, 0},        // S_SRCR1_ATK2
    {SPR_SRCR, 18, 10, A_Srcr1Attack, S_SRCR1_WALK1, 0, 0},     // S_SRCR1_ATK3
    {SPR_SRCR, 18, 10, A_FaceTarget, S_SRCR1_ATK5, 0, 0},       // S_SRCR1_ATK4
    {SPR_SRCR, 16, 7, A_FaceTarget, S_SRCR1_ATK6, 0, 0},        // S_SRCR1_ATK5
    {SPR_SRCR, 17, 6, A_FaceTarget, S_SRCR1_ATK7, 0, 0},        // S_SRCR1_ATK6
    {SPR_SRCR, 18, 10, A_Srcr1Attack, S_SRCR1_WALK1, 0, 0},     // S_SRCR1_ATK7
    {SPR_SRCR, 4, 7, null_hook(), S_SRCR1_DIE2, 0, 0}, // S_SRCR1_DIE1
    {SPR_SRCR, 5, 7, A_Scream, S_SRCR1_DIE3, 0, 0},     // S_SRCR1_DIE2
    {SPR_SRCR, 6, 7, null_hook(), S_SRCR1_DIE4, 0, 0}, // S_SRCR1_DIE3
    {SPR_SRCR, 7, 6, null_hook(), S_SRCR1_DIE5, 0, 0}, // S_SRCR1_DIE4
    {SPR_SRCR, 8, 6, null_hook(), S_SRCR1_DIE6, 0, 0}, // S_SRCR1_DIE5
    {SPR_SRCR, 9, 6, null_hook(), S_SRCR1_DIE7, 0, 0}, // S_SRCR1_DIE6
    {SPR_SRCR, 10, 6, null_hook(), S_SRCR1_DIE8, 0, 0},        // S_SRCR1_DIE7
    {SPR_SRCR, 11, 25, A_SorZap, S_SRCR1_DIE9, 0, 0},   // S_SRCR1_DIE8
    {SPR_SRCR, 12, 5, null_hook(), S_SRCR1_DIE10, 0, 0},       // S_SRCR1_DIE9
    {SPR_SRCR, 13, 5, null_hook(), S_SRCR1_DIE11, 0, 0},       // S_SRCR1_DIE10
    {SPR_SRCR, 14, 4, null_hook(), S_SRCR1_DIE12, 0, 0},       // S_SRCR1_DIE11
    {SPR_SRCR, 11, 20, A_SorZap, S_SRCR1_DIE13, 0, 0},  // S_SRCR1_DIE12
    {SPR_SRCR, 12, 5, null_hook(), S_SRCR1_DIE14, 0, 0},       // S_SRCR1_DIE13
    {SPR_SRCR, 13, 5, null_hook(), S_SRCR1_DIE15, 0, 0},       // S_SRCR1_DIE14
    {SPR_SRCR, 14, 4, null_hook(), S_SRCR1_DIE16, 0, 0},       // S_SRCR1_DIE15
    {SPR_SRCR, 11, 12, null_hook(), S_SRCR1_DIE17, 0, 0},      // S_SRCR1_DIE16
    {SPR_SRCR, 15, -1, A_SorcererRise, S_NULL, 0, 0},   // S_SRCR1_DIE17
    {SPR_FX14, 32768, 6, null_hook(), S_SRCRFX1_2, 0, 0},      // S_SRCRFX1_1
    {SPR_FX14, 32769, 6, null_hook(), S_SRCRFX1_3, 0, 0},      // S_SRCRFX1_2
    {SPR_FX14, 32770, 6, null_hook(), S_SRCRFX1_1, 0, 0},      // S_SRCRFX1_3
    {SPR_FX14, 32771, 5, null_hook(), S_SRCRFXI1_2, 0, 0},     // S_SRCRFXI1_1
    {SPR_FX14, 32772, 5, null_hook(), S_SRCRFXI1_3, 0, 0},     // S_SRCRFXI1_2
    {SPR_FX14, 32773, 5, null_hook(), S_SRCRFXI1_4, 0, 0},     // S_SRCRFXI1_3
    {SPR_FX14, 32774, 5, null_hook(), S_SRCRFXI1_5, 0, 0},     // S_SRCRFXI1_4
    {SPR_FX14, 32775, 5, null_hook(), S_NULL, 0, 0},   // S_SRCRFXI1_5
    {SPR_SOR2, 0, 4, null_hook(), S_SOR2_RISE2, 0, 0}, // S_SOR2_RISE1
    {SPR_SOR2, 1, 4, null_hook(), S_SOR2_RISE3, 0, 0}, // S_SOR2_RISE2
    {SPR_SOR2, 2, 4, A_SorRise, S_SOR2_RISE4, 0, 0},    // S_SOR2_RISE3
    {SPR_SOR2, 3, 4, null_hook(), S_SOR2_RISE5, 0, 0}, // S_SOR2_RISE4
    {SPR_SOR2, 4, 4, null_hook(), S_SOR2_RISE6, 0, 0}, // S_SOR2_RISE5
    {SPR_SOR2, 5, 4, null_hook(), S_SOR2_RISE7, 0, 0}, // S_SOR2_RISE6
    {SPR_SOR2, 6, 12, A_SorSightSnd, S_SOR2_WALK1, 0, 0},       // S_SOR2_RISE7
    {SPR_SOR2, 12, 10, A_Look, S_SOR2_LOOK2, 0, 0},     // S_SOR2_LOOK1
    {SPR_SOR2, 13, 10, A_Look, S_SOR2_LOOK1, 0, 0},     // S_SOR2_LOOK2
    {SPR_SOR2, 12, 4, A_Chase, S_SOR2_WALK2, 0, 0},     // S_SOR2_WALK1
    {SPR_SOR2, 13, 4, A_Chase, S_SOR2_WALK3, 0, 0},     // S_SOR2_WALK2
    {SPR_SOR2, 14, 4, A_Chase, S_SOR2_WALK4, 0, 0},     // S_SOR2_WALK3
    {SPR_SOR2, 15, 4, A_Chase, S_SOR2_WALK1, 0, 0},     // S_SOR2_WALK4
    {SPR_SOR2, 16, 3, null_hook(), S_SOR2_PAIN2, 0, 0},        // S_SOR2_PAIN1
    {SPR_SOR2, 16, 6, A_Pain, S_SOR2_WALK1, 0, 0},      // S_SOR2_PAIN2
    {SPR_SOR2, 17, 9, A_Srcr2Decide, S_SOR2_ATK2, 0, 0},        // S_SOR2_ATK1
    {SPR_SOR2, 18, 9, A_FaceTarget, S_SOR2_ATK3, 0, 0}, // S_SOR2_ATK2
    {SPR_SOR2, 19, 20, A_Srcr2Attack, S_SOR2_WALK1, 0, 0},      // S_SOR2_ATK3
    {SPR_SOR2, 11, 6, null_hook(), S_SOR2_TELE2, 0, 0},        // S_SOR2_TELE1
    {SPR_SOR2, 10, 6, null_hook(), S_SOR2_TELE3, 0, 0},        // S_SOR2_TELE2
    {SPR_SOR2, 9, 6, null_hook(), S_SOR2_TELE4, 0, 0}, // S_SOR2_TELE3
    {SPR_SOR2, 8, 6, null_hook(), S_SOR2_TELE5, 0, 0}, // S_SOR2_TELE4
    {SPR_SOR2, 7, 6, null_hook(), S_SOR2_TELE6, 0, 0}, // S_SOR2_TELE5
    {SPR_SOR2, 6, 6, null_hook(), S_SOR2_WALK1, 0, 0}, // S_SOR2_TELE6
    {SPR_SDTH, 0, 8, A_Sor2DthInit, S_SOR2_DIE2, 0, 0}, // S_SOR2_DIE1
    {SPR_SDTH, 1, 8, null_hook(), S_SOR2_DIE3, 0, 0},  // S_SOR2_DIE2
    {SPR_SDTH, 2, 8, A_SorDSph, S_SOR2_DIE4, 0, 0},     // S_SOR2_DIE3
    {SPR_SDTH, 3, 7, null_hook(), S_SOR2_DIE5, 0, 0},  // S_SOR2_DIE4
    {SPR_SDTH, 4, 7, null_hook(), S_SOR2_DIE6, 0, 0},  // S_SOR2_DIE5
    {SPR_SDTH, 5, 7, A_Sor2DthLoop, S_SOR2_DIE7, 0, 0}, // S_SOR2_DIE6
    {SPR_SDTH, 6, 6, A_SorDExp, S_SOR2_DIE8, 0, 0},     // S_SOR2_DIE7
    {SPR_SDTH, 7, 6, null_hook(), S_SOR2_DIE9, 0, 0},  // S_SOR2_DIE8
    {SPR_SDTH, 8, 18, null_hook(), S_SOR2_DIE10, 0, 0},        // S_SOR2_DIE9
    {SPR_SDTH, 9, 6, A_NoBlocking, S_SOR2_DIE11, 0, 0}, // S_SOR2_DIE10
    {SPR_SDTH, 10, 6, A_SorDBon, S_SOR2_DIE12, 0, 0},   // S_SOR2_DIE11
    {SPR_SDTH, 11, 6, null_hook(), S_SOR2_DIE13, 0, 0},        // S_SOR2_DIE12
    {SPR_SDTH, 12, 6, null_hook(), S_SOR2_DIE14, 0, 0},        // S_SOR2_DIE13
    {SPR_SDTH, 13, 6, null_hook(), S_SOR2_DIE15, 0, 0},        // S_SOR2_DIE14
    {SPR_SDTH, 14, -1, A_BossDeath, S_NULL, 0, 0},      // S_SOR2_DIE15
    {SPR_FX16, 32768, 3, A_BlueSpark, S_SOR2FX1_2, 0, 0},       // S_SOR2FX1_1
    {SPR_FX16, 32769, 3, A_BlueSpark, S_SOR2FX1_3, 0, 0},       // S_SOR2FX1_2
    {SPR_FX16, 32770, 3, A_BlueSpark, S_SOR2FX1_1, 0, 0},       // S_SOR2FX1_3
    {SPR_FX16, 32774, 5, A_Explode, S_SOR2FXI1_2, 0, 0},        // S_SOR2FXI1_1
    {SPR_FX16, 32775, 5, null_hook(), S_SOR2FXI1_3, 0, 0},     // S_SOR2FXI1_2
    {SPR_FX16, 32776, 5, null_hook(), S_SOR2FXI1_4, 0, 0},     // S_SOR2FXI1_3
    {SPR_FX16, 32777, 5, null_hook(), S_SOR2FXI1_5, 0, 0},     // S_SOR2FXI1_4
    {SPR_FX16, 32778, 5, null_hook(), S_SOR2FXI1_6, 0, 0},     // S_SOR2FXI1_5
    {SPR_FX16, 32779, 5, null_hook(), S_NULL, 0, 0},   // S_SOR2FXI1_6
    {SPR_FX16, 32771, 12, null_hook(), S_SOR2FXSPARK2, 0, 0},  // S_SOR2FXSPARK1
    {SPR_FX16, 32772, 12, null_hook(), S_SOR2FXSPARK3, 0, 0},  // S_SOR2FXSPARK2
    {SPR_FX16, 32773, 12, null_hook(), S_NULL, 0, 0},  // S_SOR2FXSPARK3
    {SPR_FX11, 32768, 35, null_hook(), S_SOR2FX2_2, 0, 0},     // S_SOR2FX2_1
    {SPR_FX11, 32768, 5, A_GenWizard, S_SOR2FX2_3, 0, 0},       // S_SOR2FX2_2
    {SPR_FX11, 32769, 5, null_hook(), S_SOR2FX2_2, 0, 0},      // S_SOR2FX2_3
    {SPR_FX11, 32770, 5, null_hook(), S_SOR2FXI2_2, 0, 0},     // S_SOR2FXI2_1
    {SPR_FX11, 32771, 5, null_hook(), S_SOR2FXI2_3, 0, 0},     // S_SOR2FXI2_2
    {SPR_FX11, 32772, 5, null_hook(), S_SOR2FXI2_4, 0, 0},     // S_SOR2FXI2_3
    {SPR_FX11, 32773, 5, null_hook(), S_SOR2FXI2_5, 0, 0},     // S_SOR2FXI2_4
    {SPR_FX11, 32774, 5, null_hook(), S_NULL, 0, 0},   // S_SOR2FXI2_5
    {SPR_SOR2, 6, 8, null_hook(), S_SOR2TELEFADE2, 0, 0},      // S_SOR2TELEFADE1
    {SPR_SOR2, 7, 6, null_hook(), S_SOR2TELEFADE3, 0, 0},      // S_SOR2TELEFADE2
    {SPR_SOR2, 8, 6, null_hook(), S_SOR2TELEFADE4, 0, 0},      // S_SOR2TELEFADE3
    {SPR_SOR2, 9, 6, null_hook(), S_SOR2TELEFADE5, 0, 0},      // S_SOR2TELEFADE4
    {SPR_SOR2, 10, 6, null_hook(), S_SOR2TELEFADE6, 0, 0},     // S_SOR2TELEFADE5
    {SPR_SOR2, 11, 6, null_hook(), S_NULL, 0, 0},      // S_SOR2TELEFADE6
    {SPR_MNTR, 0, 10, A_Look, S_MNTR_LOOK2, 0, 0},      // S_MNTR_LOOK1
    {SPR_MNTR, 1, 10, A_Look, S_MNTR_LOOK1, 0, 0},      // S_MNTR_LOOK2
    {SPR_MNTR, 0, 5, A_Chase, S_MNTR_WALK2, 0, 0},      // S_MNTR_WALK1
    {SPR_MNTR, 1, 5, A_Chase, S_MNTR_WALK3, 0, 0},      // S_MNTR_WALK2
    {SPR_MNTR, 2, 5, A_Chase, S_MNTR_WALK4, 0, 0},      // S_MNTR_WALK3
    {SPR_MNTR, 3, 5, A_Chase, S_MNTR_WALK1, 0, 0},      // S_MNTR_WALK4
    {SPR_MNTR, 21, 10, A_FaceTarget, S_MNTR_ATK1_2, 0, 0},      // S_MNTR_ATK1_1
    {SPR_MNTR, 22, 7, A_FaceTarget, S_MNTR_ATK1_3, 0, 0},       // S_MNTR_ATK1_2
    {SPR_MNTR, 23, 12, A_MinotaurAtk1, S_MNTR_WALK1, 0, 0},     // S_MNTR_ATK1_3
    {SPR_MNTR, 21, 10, A_MinotaurDecide, S_MNTR_ATK2_2, 0, 0},  // S_MNTR_ATK2_1
    {SPR_MNTR, 24, 4, A_FaceTarget, S_MNTR_ATK2_3, 0, 0},       // S_MNTR_ATK2_2
    {SPR_MNTR, 25, 9, A_MinotaurAtk2, S_MNTR_WALK1, 0, 0},      // S_MNTR_ATK2_3
    {SPR_MNTR, 21, 10, A_FaceTarget, S_MNTR_ATK3_2, 0, 0},      // S_MNTR_ATK3_1
    {SPR_MNTR, 22, 7, A_FaceTarget, S_MNTR_ATK3_3, 0, 0},       // S_MNTR_ATK3_2
    {SPR_MNTR, 23, 12, A_MinotaurAtk3, S_MNTR_WALK1, 0, 0},     // S_MNTR_ATK3_3
    {SPR_MNTR, 23, 12, null_hook(), S_MNTR_ATK3_1, 0, 0},      // S_MNTR_ATK3_4
    {SPR_MNTR, 20, 2, A_MinotaurCharge, S_MNTR_ATK4_1, 0, 0},   // S_MNTR_ATK4_1
    {SPR_MNTR, 4, 3, null_hook(), S_MNTR_PAIN2, 0, 0}, // S_MNTR_PAIN1
    {SPR_MNTR, 4, 6, A_Pain, S_MNTR_WALK1, 0, 0},       // S_MNTR_PAIN2
    {SPR_MNTR, 5, 6, null_hook(), S_MNTR_DIE2, 0, 0},  // S_MNTR_DIE1
    {SPR_MNTR, 6, 5, null_hook(), S_MNTR_DIE3, 0, 0},  // S_MNTR_DIE2
    {SPR_MNTR, 7, 6, A_Scream, S_MNTR_DIE4, 0, 0},      // S_MNTR_DIE3
    {SPR_MNTR, 8, 5, null_hook(), S_MNTR_DIE5, 0, 0},  // S_MNTR_DIE4
    {SPR_MNTR, 9, 6, null_hook(), S_MNTR_DIE6, 0, 0},  // S_MNTR_DIE5
    {SPR_MNTR, 10, 5, null_hook(), S_MNTR_DIE7, 0, 0}, // S_MNTR_DIE6
    {SPR_MNTR, 11, 6, null_hook(), S_MNTR_DIE8, 0, 0}, // S_MNTR_DIE7
    {SPR_MNTR, 12, 5, A_NoBlocking, S_MNTR_DIE9, 0, 0}, // S_MNTR_DIE8
    {SPR_MNTR, 13, 6, null_hook(), S_MNTR_DIE10, 0, 0},        // S_MNTR_DIE9
    {SPR_MNTR, 14, 5, null_hook(), S_MNTR_DIE11, 0, 0},        // S_MNTR_DIE10
    {SPR_MNTR, 15, 6, null_hook(), S_MNTR_DIE12, 0, 0},        // S_MNTR_DIE11
    {SPR_MNTR, 16, 5, null_hook(), S_MNTR_DIE13, 0, 0},        // S_MNTR_DIE12
    {SPR_MNTR, 17, 6, null_hook(), S_MNTR_DIE14, 0, 0},        // S_MNTR_DIE13
    {SPR_MNTR, 18, 5, null_hook(), S_MNTR_DIE15, 0, 0},        // S_MNTR_DIE14
    {SPR_MNTR, 19, -1, A_BossDeath, S_NULL, 0, 0},      // S_MNTR_DIE15
    {SPR_FX12, 32768, 6, null_hook(), S_MNTRFX1_2, 0, 0},      // S_MNTRFX1_1
    {SPR_FX12, 32769, 6, null_hook(), S_MNTRFX1_1, 0, 0},      // S_MNTRFX1_2
    {SPR_FX12, 32770, 5, null_hook(), S_MNTRFXI1_2, 0, 0},     // S_MNTRFXI1_1
    {SPR_FX12, 32771, 5, null_hook(), S_MNTRFXI1_3, 0, 0},     // S_MNTRFXI1_2
    {SPR_FX12, 32772, 5, null_hook(), S_MNTRFXI1_4, 0, 0},     // S_MNTRFXI1_3
    {SPR_FX12, 32773, 5, null_hook(), S_MNTRFXI1_5, 0, 0},     // S_MNTRFXI1_4
    {SPR_FX12, 32774, 5, null_hook(), S_MNTRFXI1_6, 0, 0},     // S_MNTRFXI1_5
    {SPR_FX12, 32775, 5, null_hook(), S_NULL, 0, 0},   // S_MNTRFXI1_6
    {SPR_FX13, 0, 2, A_MntrFloorFire, S_MNTRFX2_1, 0, 0},       // S_MNTRFX2_1
    {SPR_FX13, 32776, 4, A_Explode, S_MNTRFXI2_2, 0, 0},        // S_MNTRFXI2_1
    {SPR_FX13, 32777, 4, null_hook(), S_MNTRFXI2_3, 0, 0},     // S_MNTRFXI2_2
    {SPR_FX13, 32778, 4, null_hook(), S_MNTRFXI2_4, 0, 0},     // S_MNTRFXI2_3
    {SPR_FX13, 32779, 4, null_hook(), S_MNTRFXI2_5, 0, 0},     // S_MNTRFXI2_4
    {SPR_FX13, 32780, 4, null_hook(), S_NULL, 0, 0},   // S_MNTRFXI2_5
    {SPR_FX13, 32771, 4, null_hook(), S_MNTRFX3_2, 0, 0},      // S_MNTRFX3_1
    {SPR_FX13, 32770, 4, null_hook(), S_MNTRFX3_3, 0, 0},      // S_MNTRFX3_2
    {SPR_FX13, 32769, 5, null_hook(), S_MNTRFX3_4, 0, 0},      // S_MNTRFX3_3
    {SPR_FX13, 32770, 5, null_hook(), S_MNTRFX3_5, 0, 0},      // S_MNTRFX3_4
    {SPR_FX13, 32771, 5, null_hook(), S_MNTRFX3_6, 0, 0},      // S_MNTRFX3_5
    {SPR_FX13, 32772, 5, null_hook(), S_MNTRFX3_7, 0, 0},      // S_MNTRFX3_6
    {SPR_FX13, 32773, 4, null_hook(), S_MNTRFX3_8, 0, 0},      // S_MNTRFX3_7
    {SPR_FX13, 32774, 4, null_hook(), S_MNTRFX3_9, 0, 0},      // S_MNTRFX3_8
    {SPR_FX13, 32775, 4, null_hook(), S_NULL, 0, 0},   // S_MNTRFX3_9
    {SPR_AKYY, 32768, 3, null_hook(), S_AKYY2, 0, 0},  // S_AKYY1
    {SPR_AKYY, 32769, 3, null_hook(), S_AKYY3, 0, 0},  // S_AKYY2
    {SPR_AKYY, 32770, 3, null_hook(), S_AKYY4, 0, 0},  // S_AKYY3
    {SPR_AKYY, 32771, 3, null_hook(), S_AKYY5, 0, 0},  // S_AKYY4
    {SPR_AKYY, 32772, 3, null_hook(), S_AKYY6, 0, 0},  // S_AKYY5
    {SPR_AKYY, 32773, 3, null_hook(), S_AKYY7, 0, 0},  // S_AKYY6
    {SPR_AKYY, 32774, 3, null_hook(), S_AKYY8, 0, 0},  // S_AKYY7
    {SPR_AKYY, 32775, 3, null_hook(), S_AKYY9, 0, 0},  // S_AKYY8
    {SPR_AKYY, 32776, 3, null_hook(), S_AKYY10, 0, 0}, // S_AKYY9
    {SPR_AKYY, 32777, 3, null_hook(), S_AKYY1, 0, 0},  // S_AKYY10
    {SPR_BKYY, 32768, 3, null_hook(), S_BKYY2, 0, 0},  // S_BKYY1
    {SPR_BKYY, 32769, 3, null_hook(), S_BKYY3, 0, 0},  // S_BKYY2
    {SPR_BKYY, 32770, 3, null_hook(), S_BKYY4, 0, 0},  // S_BKYY3
    {SPR_BKYY, 32771, 3, null_hook(), S_BKYY5, 0, 0},  // S_BKYY4
    {SPR_BKYY, 32772, 3, null_hook(), S_BKYY6, 0, 0},  // S_BKYY5
    {SPR_BKYY, 32773, 3, null_hook(), S_BKYY7, 0, 0},  // S_BKYY6
    {SPR_BKYY, 32774, 3, null_hook(), S_BKYY8, 0, 0},  // S_BKYY7
    {SPR_BKYY, 32775, 3, null_hook(), S_BKYY9, 0, 0},  // S_BKYY8
    {SPR_BKYY, 32776, 3, null_hook(), S_BKYY10, 0, 0}, // S_BKYY9
    {SPR_BKYY, 32777, 3, null_hook(), S_BKYY1, 0, 0},  // S_BKYY10
    {SPR_CKYY, 32768, 3, null_hook(), S_CKYY2, 0, 0},  // S_CKYY1
    {SPR_CKYY, 32769, 3, null_hook(), S_CKYY3, 0, 0},  // S_CKYY2
    {SPR_CKYY, 32770, 3, null_hook(), S_CKYY4, 0, 0},  // S_CKYY3
    {SPR_CKYY, 32771, 3, null_hook(), S_CKYY5, 0, 0},  // S_CKYY4
    {SPR_CKYY, 32772, 3, null_hook(), S_CKYY6, 0, 0},  // S_CKYY5
    {SPR_CKYY, 32773, 3, null_hook(), S_CKYY7, 0, 0},  // S_CKYY6
    {SPR_CKYY, 32774, 3, null_hook(), S_CKYY8, 0, 0},  // S_CKYY7
    {SPR_CKYY, 32775, 3, null_hook(), S_CKYY9, 0, 0},  // S_CKYY8
    {SPR_CKYY, 32776, 3, null_hook(), S_CKYY1, 0, 0},  // S_CKYY9
    {SPR_AMG1, 0, -1, null_hook(), S_NULL, 0, 0},      // S_AMG1
    {SPR_AMG2, 0, 4, null_hook(), S_AMG2_2, 0, 0},     // S_AMG2_1
    {SPR_AMG2, 1, 4, null_hook(), S_AMG2_3, 0, 0},     // S_AMG2_2
    {SPR_AMG2, 2, 4, null_hook(), S_AMG2_1, 0, 0},     // S_AMG2_3
    {SPR_AMM1, 0, -1, null_hook(), S_NULL, 0, 0},      // S_AMM1
    {SPR_AMM2, 0, -1, null_hook(), S_NULL, 0, 0},      // S_AMM2
    {SPR_AMC1, 0, -1, null_hook(), S_NULL, 0, 0},      // S_AMC1
    {SPR_AMC2, 0, 5, null_hook(), S_AMC2_2, 0, 0},     // S_AMC2_1
    {SPR_AMC2, 1, 5, null_hook(), S_AMC2_3, 0, 0},     // S_AMC2_2
    {SPR_AMC2, 2, 5, null_hook(), S_AMC2_1, 0, 0},     // S_AMC2_3
    {SPR_AMS1, 0, 5, null_hook(), S_AMS1_2, 0, 0},     // S_AMS1_1
    {SPR_AMS1, 1, 5, null_hook(), S_AMS1_1, 0, 0},     // S_AMS1_2
    {SPR_AMS2, 0, 5, null_hook(), S_AMS2_2, 0, 0},     // S_AMS2_1
    {SPR_AMS2, 1, 5, null_hook(), S_AMS2_1, 0, 0},     // S_AMS2_2
    {SPR_AMP1, 0, 4, null_hook(), S_AMP1_2, 0, 0},     // S_AMP1_1
    {SPR_AMP1, 1, 4, null_hook(), S_AMP1_3, 0, 0},     // S_AMP1_2
    {SPR_AMP1, 2, 4, null_hook(), S_AMP1_1, 0, 0},     // S_AMP1_3
    {SPR_AMP2, 0, 4, null_hook(), S_AMP2_2, 0, 0},     // S_AMP2_1
    {SPR_AMP2, 1, 4, null_hook(), S_AMP2_3, 0, 0},     // S_AMP2_2
    {SPR_AMP2, 2, 4, null_hook(), S_AMP2_1, 0, 0},     // S_AMP2_3
    {SPR_AMB1, 0, 4, null_hook(), S_AMB1_2, 0, 0},     // S_AMB1_1
    {SPR_AMB1, 1, 4, null_hook(), S_AMB1_3, 0, 0},     // S_AMB1_2
    {SPR_AMB1, 2, 4, null_hook(), S_AMB1_1, 0, 0},     // S_AMB1_3
    {SPR_AMB2, 0, 4, null_hook(), S_AMB2_2, 0, 0},     // S_AMB2_1
    {SPR_AMB2, 1, 4, null_hook(), S_AMB2_3, 0, 0},     // S_AMB2_2
    {SPR_AMB2, 2, 4, null_hook(), S_AMB2_1, 0, 0},     // S_AMB2_3
    {SPR_AMG1, 0, 100, A_ESound, S_SND_WIND, 0, 0},     // S_SND_WIND
    {SPR_AMG1, 0, 85, A_ESound, S_SND_WATERFALL, 0, 0}  // S_SND_WATERFALL
};


mobjinfo_t mobjinfo[NUMMOBJTYPES] = {

    {                           // MT_MISC0
     81,                        // doomednum
     S_ITEM_PTN1_1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_ITEMSHIELD1
     85,                        // doomednum
     S_ITEM_SHLD1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_ITEMSHIELD2
     31,                        // doomednum
     S_ITEM_SHD2_1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_MISC1
     8,                         // doomednum
     S_ITEM_BAGH1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_MISC2
     35,                        // doomednum
     S_ITEM_SPMP1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_ARTIINVISIBILITY
     75,                        // doomednum
     S_ARTI_INVS1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_SHADOW | MF_COUNTITEM,     // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_MISC3
     82,                        // doomednum
     S_ARTI_PTN2_1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_ARTIFLY
     83,                        // doomednum
     S_ARTI_SOAR1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_ARTIINVULNERABILITY
     84,                        // doomednum
     S_ARTI_INVU1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_ARTITOMEOFPOWER
     86,                        // doomednum
     S_ARTI_PWBK1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_ARTIEGG
     30,                        // doomednum
     S_ARTI_EGGC1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_EGGFX
     -1,                        // doomednum
     S_EGGFX1,                  // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_EGGFXI1_1,               // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     18 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_ARTISUPERHEAL
     32,                        // doomednum
     S_ARTI_SPHL1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_MISC4
     33,                        // doomednum
     S_ARTI_TRCH1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_MISC5
     34,                        // doomednum
     S_ARTI_FBMB1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_FIREBOMB
     -1,                        // doomednum
     S_FIREBOMB1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_phohit,                // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOGRAVITY | MF_SHADOW,  // flags
     0                          // flags2
     },

    {                           // MT_ARTITELEPORT
     36,                        // doomednum
     S_ARTI_ATLP1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB               // flags2
     },

    {                           // MT_POD
     2035,                      // doomednum
     S_POD_WAIT1,               // spawnstate
     45,                        // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_POD_PAIN1,               // painstate
     255,                       // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_POD_DIE1,                // deathstate
     S_NULL,                    // xdeathstate
     sfx_podexp,                // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     54 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID | MF_NOBLOOD | MF_SHOOTABLE | MF_DROPOFF, // flags
     MF2_WINDTHRUST | MF2_PUSHABLE | MF2_SLIDE | MF2_PASSMOBJ | MF2_TELESTOMP   // flags2
     },

    {                           // MT_PODGOO
     -1,                        // doomednum
     S_PODGOO1,                 // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_PODGOOX,                 // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_NOTELEPORT | MF2_LOGRAV | MF2_CANNOTPUSH       // flags2
     },

    {                           // MT_PODGENERATOR
     43,                        // doomednum
     S_PODGENERATOR,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0                          // flags2
     },

    {                           // MT_SPLASH
     -1,                        // doomednum
     S_SPLASH1,                 // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SPLASHX,                 // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_NOTELEPORT | MF2_LOGRAV | MF2_CANNOTPUSH       // flags2
     },

    {                           // MT_SPLASHBASE
     -1,                        // doomednum
     S_SPLASHBASE1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0                          // flags2
     },

    {                           // MT_LAVASPLASH
     -1,                        // doomednum
     S_LAVASPLASH1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0                          // flags2
     },

    {                           // MT_LAVASMOKE
     -1,                        // doomednum
     S_LAVASMOKE1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW,  // flags
     0                          // flags2
     },

    {                           // MT_SLUDGECHUNK
     -1,                        // doomednum
     S_SLUDGECHUNK1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SLUDGECHUNKX,            // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_NOTELEPORT | MF2_LOGRAV | MF2_CANNOTPUSH       // flags2
     },

    {                           // MT_SLUDGESPLASH
     -1,                        // doomednum
     S_SLUDGESPLASH1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0                          // flags2
     },

    {                           // MT_SKULLHANG70
     17,                        // doomednum
     S_SKULLHANG70_1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     70 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0                          // flags2
     },

    {                           // MT_SKULLHANG60
     24,                        // doomednum
     S_SKULLHANG60_1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     60 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0                          // flags2
     },

    {                           // MT_SKULLHANG45
     25,                        // doomednum
     S_SKULLHANG45_1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     45 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0                          // flags2
     },

    {                           // MT_SKULLHANG35
     26,                        // doomednum
     S_SKULLHANG35_1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     35 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0                          // flags2
     },

    {                           // MT_CHANDELIER
     28,                        // doomednum
     S_CHANDELIER1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     60 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0                          // flags2
     },

    {                           // MT_SERPTORCH
     27,                        // doomednum
     S_SERPTORCH1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     54 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_SMALLPILLAR
     29,                        // doomednum
     S_SMALLPILLAR,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     34 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_STALAGMITESMALL
     37,                        // doomednum
     S_STALAGMITESMALL,         // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     32 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_STALAGMITELARGE
     38,                        // doomednum
     S_STALAGMITELARGE,         // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     64 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_STALACTITESMALL
     39,                        // doomednum
     S_STALACTITESMALL,         // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     36 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
     0                          // flags2
     },

    {                           // MT_STALACTITELARGE
     40,                        // doomednum
     S_STALACTITELARGE,         // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     68 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
     0                          // flags2
     },

    {                           // MT_MISC6
     76,                        // doomednum
     S_FIREBRAZIER1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     44 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_BARREL
     44,                        // doomednum
     S_BARREL,                  // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     32 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_MISC7
     47,                        // doomednum
     S_BRPILLAR,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     14 * FRACUNIT,             // radius
     128 * FRACUNIT,            // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_MISC8
     48,                        // doomednum
     S_MOSS1,                   // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     23 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0                          // flags2
     },

    {                           // MT_MISC9
     49,                        // doomednum
     S_MOSS2,                   // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     27 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0                          // flags2
     },

    {                           // MT_MISC10
     50,                        // doomednum
     S_WALLTORCH1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOGRAVITY,              // flags
     0                          // flags2
     },

    {                           // MT_MISC11
     51,                        // doomednum
     S_HANGINGCORPSE,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     104 * FRACUNIT,            // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
     0                          // flags2
     },

    {                           // MT_KEYGIZMOBLUE
     94,                        // doomednum
     S_KEYGIZMO1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     50 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_KEYGIZMOGREEN
     95,                        // doomednum
     S_KEYGIZMO1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     50 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_KEYGIZMOYELLOW
     96,                        // doomednum
     S_KEYGIZMO1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     50 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_KEYGIZMOFLOAT
     -1,                        // doomednum
     S_KGZ_START,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID | MF_NOGRAVITY,   // flags
     0                          // flags2
     },

    {                           // MT_MISC12
     87,                        // doomednum
     S_VOLCANO1,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     20 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID,                  // flags
     0                          // flags2
     },

    {                           // MT_VOLCANOBLAST
     -1,                        // doomednum
     S_VOLCANOBALL1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_VOLCANOBALLX1,           // deathstate
     S_NULL,                    // xdeathstate
     sfx_volhit,                // deathsound
     2 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_LOGRAV | MF2_NOTELEPORT | MF2_FIREDAMAGE       // flags2
     },

    {                           // MT_VOLCANOTBLAST
     -1,                        // doomednum
     S_VOLCANOTBALL1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_VOLCANOTBALLX1,          // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     2 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_LOGRAV | MF2_NOTELEPORT | MF2_FIREDAMAGE       // flags2
     },

    {                           // MT_TELEGLITGEN
     74,                        // doomednum
     S_TELEGLITGEN1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR,        // flags
     0                          // flags2
     },

    {                           // MT_TELEGLITGEN2
     52,                        // doomednum
     S_TELEGLITGEN2,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR,        // flags
     0                          // flags2
     },

    {                           // MT_TELEGLITTER
     -1,                        // doomednum
     S_TELEGLITTER1_1,          // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     0                          // flags2
     },

    {                           // MT_TELEGLITTER2
     -1,                        // doomednum
     S_TELEGLITTER2_1,          // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     0                          // flags2
     },

    {                           // MT_TFOG
     -1,                        // doomednum
     S_TFOG1,                   // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_TELEPORTMAN
     14,                        // doomednum
     S_NULL,                    // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0                          // flags2
     },

    {                           // MT_STAFFPUFF
     -1,                        // doomednum
     S_STAFFPUFF1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_stfhit,                // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_STAFFPUFF2
     -1,                        // doomednum
     S_STAFFPUFF2_1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_stfpow,                // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_BEAKPUFF
     -1,                        // doomednum
     S_STAFFPUFF1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_chicatk,               // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_MISC13
     2005,                      // doomednum
     S_WGNT,                    // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_GAUNTLETPUFF1
     -1,                        // doomednum
     S_GAUNTLETPUFF1_1,         // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW,  // flags
     0                          // flags2
     },

    {                           // MT_GAUNTLETPUFF2
     -1,                        // doomednum
     S_GAUNTLETPUFF2_1,         // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW,  // flags
     0                          // flags2
     },

    {                           // MT_MISC14
     53,                        // doomednum
     S_BLSR,                    // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_BLASTERFX1
     -1,                        // doomednum
     S_BLASTERFX1_1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_BLASTERFXI1_1,           // deathstate
     S_NULL,                    // xdeathstate
     sfx_blshit,                // deathsound
     184 * FRACUNIT,            // speed
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_BLASTERSMOKE
     -1,                        // doomednum
     S_BLASTERSMOKE1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW,  // flags
     MF2_NOTELEPORT | MF2_CANNOTPUSH    // flags2
     },

    {                           // MT_RIPPER
     -1,                        // doomednum
     S_RIPPER1,                 // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_RIPPERX1,                // deathstate
     S_NULL,                    // xdeathstate
     sfx_hrnhit,                // deathsound
     14 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_RIP   // flags2
     },

    {                           // MT_BLASTERPUFF1
     -1,                        // doomednum
     S_BLASTERPUFF1_1,          // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_BLASTERPUFF2
     -1,                        // doomednum
     S_BLASTERPUFF2_1,          // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_WMACE
     2002,                      // doomednum
     S_WMCE,                    // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_MACEFX1
     -1,                        // doomednum
     S_MACEFX1_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_lobsht,                // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MACEFXI1_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_FLOORBOUNCE | MF2_THRUGHOST | MF2_NOTELEPORT   // flags2
     },

    {                           // MT_MACEFX2
     -1,                        // doomednum
     S_MACEFX2_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MACEFXI2_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     6,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_LOGRAV | MF2_FLOORBOUNCE | MF2_THRUGHOST | MF2_NOTELEPORT      // flags2
     },

    {                           // MT_MACEFX3
     -1,                        // doomednum
     S_MACEFX3_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MACEFXI1_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     7 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     4,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_LOGRAV | MF2_FLOORBOUNCE | MF2_THRUGHOST | MF2_NOTELEPORT      // flags2
     },

    {                           // MT_MACEFX4
     -1,                        // doomednum
     S_MACEFX4_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MACEFXI4_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     7 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     18,                        // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_LOGRAV | MF2_FLOORBOUNCE | MF2_THRUGHOST | MF2_TELESTOMP       // flags2
     },

    {                           // MT_WSKULLROD
     2004,                      // doomednum
     S_WSKL,                    // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_HORNRODFX1
     -1,                        // doomednum
     S_HRODFX1_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_hrnsht,                // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_HRODFXI1_1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_hrnhit,                // deathsound
     22 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_WINDTHRUST | MF2_NOTELEPORT    // flags2
     },

    {                           // MT_HORNRODFX2
     -1,                        // doomednum
     S_HRODFX2_1,               // spawnstate
     4 * 35,                    // spawnhealth
     S_NULL,                    // seestate
     sfx_hrnsht,                // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_HRODFXI2_1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_ramphit,               // deathsound
     22 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     10,                        // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_RAINPLR1
     -1,                        // doomednum
     S_RAINPLR1_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_RAINPLR1X_1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_RAINPLR2
     -1,                        // doomednum
     S_RAINPLR2_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_RAINPLR2X_1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_RAINPLR3
     -1,                        // doomednum
     S_RAINPLR3_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_RAINPLR3X_1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_RAINPLR4
     -1,                        // doomednum
     S_RAINPLR4_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_RAINPLR4X_1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_GOLDWANDFX1
     -1,                        // doomednum
     S_GWANDFX1_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_GWANDFXI1_1,             // deathstate
     S_NULL,                    // xdeathstate
     sfx_gldhit,                // deathsound
     22 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_GOLDWANDFX2
     -1,                        // doomednum
     S_GWANDFX2_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_GWANDFXI1_1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     18 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_GOLDWANDPUFF1
     -1,                        // doomednum
     S_GWANDPUFF1_1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_GOLDWANDPUFF2
     -1,                        // doomednum
     S_GWANDFXI1_1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_WPHOENIXROD
     2003,                      // doomednum
     S_WPHX,                    // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_PHOENIXFX1
     -1,                        // doomednum
     S_PHOENIXFX1_1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_phosht,                // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_PHOENIXFXI1_1,           // deathstate
     S_NULL,                    // xdeathstate
     sfx_phohit,                // deathsound
     20 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     20,                        // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_THRUGHOST | MF2_NOTELEPORT     // flags2
     },

    // The following thing is present in the mobjinfo table from Heretic 1.0,
    // but not in Heretic 1.3 (ie. it was removed).  It has been re-inserted
    // here to support HHE patches.

    {                           // MT_PHOENIXFX_REMOVED
     -1,                        // doomednum
     S_PHOENIXFXIX_1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_PHOENIXFXIX_3,           // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT     // flags2
     },

    {                           // MT_PHOENIXPUFF
     -1,                        // doomednum
     S_PHOENIXPUFF1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW,  // flags
     MF2_NOTELEPORT | MF2_CANNOTPUSH    // flags2
     },

    {                           // MT_PHOENIXFX2
     -1,                        // doomednum
     S_PHOENIXFX2_1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_PHOENIXFXI2_1,           // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_FIREDAMAGE    // flags2
     },

    {                           // MT_MISC15
     2001,                      // doomednum
     S_WBOW,                    // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_CRBOWFX1
     -1,                        // doomednum
     S_CRBOWFX1,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_bowsht,                // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_CRBOWFXI1_1,             // deathstate
     S_NULL,                    // xdeathstate
     sfx_hrnhit,                // deathsound
     30 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     10,                        // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_CRBOWFX2
     -1,                        // doomednum
     S_CRBOWFX2,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_bowsht,                // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_CRBOWFXI1_1,             // deathstate
     S_NULL,                    // xdeathstate
     sfx_hrnhit,                // deathsound
     32 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     6,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_CRBOWFX3
     -1,                        // doomednum
     S_CRBOWFX3,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_CRBOWFXI3_1,             // deathstate
     S_NULL,                    // xdeathstate
     sfx_hrnhit,                // deathsound
     20 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_WINDTHRUST | MF2_THRUGHOST | MF2_NOTELEPORT    // flags2
     },

    {                           // MT_CRBOWFX4
     -1,                        // doomednum
     S_CRBOWFX4_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     MF2_LOGRAV                 // flags2
     },

    {                           // MT_BLOOD
     -1,                        // doomednum
     S_BLOOD1,                  // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0                          // flags2
     },

    {                           // MT_BLOODSPLATTER
     -1,                        // doomednum
     S_BLOODSPLATTER1,          // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_BLOODSPLATTERX,          // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_NOTELEPORT | MF2_CANNOTPUSH    // flags2
     },

    {                           // MT_PLAYER
     -1,                        // doomednum
     S_PLAY,                    // spawnstate
     100,                       // spawnhealth
     S_PLAY_RUN1,               // seestate
     sfx_None,                  // seesound
     0,                         // reactiontime
     sfx_None,                  // attacksound
     S_PLAY_PAIN,               // painstate
     255,                       // painchance
     sfx_plrpai,                // painsound
     S_NULL,                    // meleestate
     S_PLAY_ATK1,               // missilestate
     S_NULL,                    // crashstate
     S_PLAY_DIE1,               // deathstate
     S_PLAY_XDIE1,              // xdeathstate
     sfx_plrdth,                // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     56 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_PICKUP | MF_NOTDMATCH,   // flags
     MF2_WINDTHRUST | MF2_FOOTCLIP | MF2_SLIDE | MF2_PASSMOBJ | MF2_TELESTOMP   // flags2
     },

    {                           // MT_BLOODYSKULL
     -1,                        // doomednum
     S_BLOODYSKULL1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     4 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_DROPOFF,        // flags
     MF2_LOGRAV | MF2_CANNOTPUSH        // flags2
     },

    {                           // MT_CHICPLAYER
     -1,                        // doomednum
     S_CHICPLAY,                // spawnstate
     100,                       // spawnhealth
     S_CHICPLAY_RUN1,           // seestate
     sfx_None,                  // seesound
     0,                         // reactiontime
     sfx_None,                  // attacksound
     S_CHICPLAY_PAIN,           // painstate
     255,                       // painchance
     sfx_chicpai,               // painsound
     S_NULL,                    // meleestate
     S_CHICPLAY_ATK1,           // missilestate
     S_NULL,                    // crashstate
     S_CHICKEN_DIE1,            // deathstate
     S_NULL,                    // xdeathstate
     sfx_chicdth,               // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     24 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_NOTDMATCH,       // flags
     MF2_WINDTHRUST | MF2_SLIDE | MF2_PASSMOBJ | MF2_FOOTCLIP | MF2_LOGRAV | MF2_TELESTOMP      // flags2
     },

    {                           // MT_CHICKEN
     -1,                        // doomednum
     S_CHICKEN_LOOK1,           // spawnstate
     10,                        // spawnhealth
     S_CHICKEN_WALK1,           // seestate
     sfx_chicpai,               // seesound
     8,                         // reactiontime
     sfx_chicatk,               // attacksound
     S_CHICKEN_PAIN1,           // painstate
     200,                       // painchance
     sfx_chicpai,               // painsound
     S_CHICKEN_ATK1,            // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_CHICKEN_DIE1,            // deathstate
     S_NULL,                    // xdeathstate
     sfx_chicdth,               // deathsound
     4,                         // speed
     9 * FRACUNIT,              // radius
     22 * FRACUNIT,             // height
     40,                        // mass
     0,                         // damage
     sfx_chicact,               // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,       // flags
     MF2_WINDTHRUST | MF2_FOOTCLIP | MF2_PASSMOBJ       // flags2
     },

    {                           // MT_FEATHER
     -1,                        // doomednum
     S_FEATHER1,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_FEATHERX,                // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     MF2_NOTELEPORT | MF2_LOGRAV | MF2_CANNOTPUSH | MF2_WINDTHRUST      // flags2
     },

    {                           // MT_MUMMY
     68,                        // doomednum
     S_MUMMY_LOOK1,             // spawnstate
     80,                        // spawnhealth
     S_MUMMY_WALK1,             // seestate
     sfx_mumsit,                // seesound
     8,                         // reactiontime
     sfx_mumat1,                // attacksound
     S_MUMMY_PAIN1,             // painstate
     128,                       // painchance
     sfx_mumpai,                // painsound
     S_MUMMY_ATK1,              // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MUMMY_DIE1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     sfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_MUMMYLEADER
     45,                        // doomednum
     S_MUMMY_LOOK1,             // spawnstate
     100,                       // spawnhealth
     S_MUMMY_WALK1,             // seestate
     sfx_mumsit,                // seesound
     8,                         // reactiontime
     sfx_mumat1,                // attacksound
     S_MUMMY_PAIN1,             // painstate
     64,                        // painchance
     sfx_mumpai,                // painsound
     S_MUMMY_ATK1,              // meleestate
     S_MUMMYL_ATK1,             // missilestate
     S_NULL,                    // crashstate
     S_MUMMY_DIE1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     sfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_MUMMYGHOST
     69,                        // doomednum
     S_MUMMY_LOOK1,             // spawnstate
     80,                        // spawnhealth
     S_MUMMY_WALK1,             // seestate
     sfx_mumsit,                // seesound
     8,                         // reactiontime
     sfx_mumat1,                // attacksound
     S_MUMMY_PAIN1,             // painstate
     128,                       // painchance
     sfx_mumpai,                // painsound
     S_MUMMY_ATK1,              // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MUMMY_DIE1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     sfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_SHADOW,        // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_MUMMYLEADERGHOST
     46,                        // doomednum
     S_MUMMY_LOOK1,             // spawnstate
     100,                       // spawnhealth
     S_MUMMY_WALK1,             // seestate
     sfx_mumsit,                // seesound
     8,                         // reactiontime
     sfx_mumat1,                // attacksound
     S_MUMMY_PAIN1,             // painstate
     64,                        // painchance
     sfx_mumpai,                // painsound
     S_MUMMY_ATK1,              // meleestate
     S_MUMMYL_ATK1,             // missilestate
     S_NULL,                    // crashstate
     S_MUMMY_DIE1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     sfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_SHADOW,        // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_MUMMYSOUL
     -1,                        // doomednum
     S_MUMMY_SOUL1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0                          // flags2
     },

    {                           // MT_MUMMYFX1
     -1,                        // doomednum
     S_MUMMYFX1_1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MUMMYFXI1_1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     9 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     14 * FRACUNIT,             // height
     100,                       // mass
     4,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_BEAST
     70,                        // doomednum
     S_BEAST_LOOK1,             // spawnstate
     220,                       // spawnhealth
     S_BEAST_WALK1,             // seestate
     sfx_bstsit,                // seesound
     8,                         // reactiontime
     sfx_bstatk,                // attacksound
     S_BEAST_PAIN1,             // painstate
     100,                       // painchance
     sfx_bstpai,                // painsound
     S_NULL,                    // meleestate
     S_BEAST_ATK1,              // missilestate
     S_NULL,                    // crashstate
     S_BEAST_DIE1,              // deathstate
     S_BEAST_XDIE1,             // xdeathstate
     sfx_bstdth,                // deathsound
     14,                        // speed
     32 * FRACUNIT,             // radius
     74 * FRACUNIT,             // height
     200,                       // mass
     0,                         // damage
     sfx_bstact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_BEASTBALL
     -1,                        // doomednum
     S_BEASTBALL1,              // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_BEASTBALLX1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     9 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     4,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_WINDTHRUST | MF2_NOTELEPORT    // flags2
     },

    {                           // MT_BURNBALL
     -1,                        // doomednum
     S_BURNBALL1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_BEASTBALLX1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_BURNBALLFB
     -1,                        // doomednum
     S_BURNBALLFB1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_BEASTBALLX1,             // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_PUFFY
     -1,                        // doomednum
     S_PUFFY1,                  // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_PUFFY1,                  // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_SNAKE
     92,                        // doomednum
     S_SNAKE_LOOK1,             // spawnstate
     280,                       // spawnhealth
     S_SNAKE_WALK1,             // seestate
     sfx_snksit,                // seesound
     8,                         // reactiontime
     sfx_snkatk,                // attacksound
     S_SNAKE_PAIN1,             // painstate
     48,                        // painchance
     sfx_snkpai,                // painsound
     S_NULL,                    // meleestate
     S_SNAKE_ATK1,              // missilestate
     S_NULL,                    // crashstate
     S_SNAKE_DIE1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_snkdth,                // deathsound
     10,                        // speed
     22 * FRACUNIT,             // radius
     70 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_snkact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_SNAKEPRO_A
     -1,                        // doomednum
     S_SNAKEPRO_A1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SNAKEPRO_AX1,            // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     14 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_WINDTHRUST | MF2_NOTELEPORT    // flags2
     },

    {                           // MT_SNAKEPRO_B
     -1,                        // doomednum
     S_SNAKEPRO_B1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SNAKEPRO_BX1,            // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     14 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_HEAD
     6,                         // doomednum
     S_HEAD_LOOK,               // spawnstate
     700,                       // spawnhealth
     S_HEAD_FLOAT,              // seestate
     sfx_hedsit,                // seesound
     8,                         // reactiontime
     sfx_hedat1,                // attacksound
     S_HEAD_PAIN1,              // painstate
     32,                        // painchance
     sfx_hedpai,                // painsound
     S_NULL,                    // meleestate
     S_HEAD_ATK1,               // missilestate
     S_NULL,                    // crashstate
     S_HEAD_DIE1,               // deathstate
     S_NULL,                    // xdeathstate
     sfx_heddth,                // deathsound
     6,                         // speed
     40 * FRACUNIT,             // radius
     72 * FRACUNIT,             // height
     325,                       // mass
     0,                         // damage
     sfx_hedact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD,       // flags
     MF2_PASSMOBJ               // flags2
     },

    {                           // MT_HEADFX1
     -1,                        // doomednum
     S_HEADFX1_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_HEADFXI1_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     13 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_THRUGHOST     // flags2
     },

    {                           // MT_HEADFX2
     -1,                        // doomednum
     S_HEADFX2_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_HEADFXI2_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     8 * FRACUNIT,              // speed
     12 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_HEADFX3
     -1,                        // doomednum
     S_HEADFX3_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_HEADFXI3_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     14 * FRACUNIT,             // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_WINDTHRUST | MF2_NOTELEPORT    // flags2
     },

    {                           // MT_WHIRLWIND
     -1,                        // doomednum
     S_HEADFX4_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_HEADFXI4_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     16 * FRACUNIT,             // radius
     74 * FRACUNIT,             // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY | MF_SHADOW,        // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_CLINK
     90,                        // doomednum
     S_CLINK_LOOK1,             // spawnstate
     150,                       // spawnhealth
     S_CLINK_WALK1,             // seestate
     sfx_clksit,                // seesound
     8,                         // reactiontime
     sfx_clkatk,                // attacksound
     S_CLINK_PAIN1,             // painstate
     32,                        // painchance
     sfx_clkpai,                // painsound
     S_CLINK_ATK1,              // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_CLINK_DIE1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_clkdth,                // deathsound
     14,                        // speed
     20 * FRACUNIT,             // radius
     64 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     sfx_clkact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD,       // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_WIZARD
     15,                        // doomednum
     S_WIZARD_LOOK1,            // spawnstate
     180,                       // spawnhealth
     S_WIZARD_WALK1,            // seestate
     sfx_wizsit,                // seesound
     8,                         // reactiontime
     sfx_wizatk,                // attacksound
     S_WIZARD_PAIN1,            // painstate
     64,                        // painchance
     sfx_wizpai,                // painsound
     S_NULL,                    // meleestate
     S_WIZARD_ATK1,             // missilestate
     S_NULL,                    // crashstate
     S_WIZARD_DIE1,             // deathstate
     S_NULL,                    // xdeathstate
     sfx_wizdth,                // deathsound
     12,                        // speed
     16 * FRACUNIT,             // radius
     68 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_wizact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_FLOAT | MF_NOGRAVITY,  // flags
     MF2_PASSMOBJ               // flags2
     },

    {                           // MT_WIZFX1
     -1,                        // doomednum
     S_WIZFX1_1,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_WIZFXI1_1,               // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     18 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_IMP
     66,                        // doomednum
     S_IMP_LOOK1,               // spawnstate
     40,                        // spawnhealth
     S_IMP_FLY1,                // seestate
     sfx_impsit,                // seesound
     8,                         // reactiontime
     sfx_impat1,                // attacksound
     S_IMP_PAIN1,               // painstate
     200,                       // painchance
     sfx_imppai,                // painsound
     S_IMP_MEATK1,              // meleestate
     S_IMP_MSATK1_1,            // missilestate
     S_IMP_CRASH1,              // crashstate
     S_IMP_DIE1,                // deathstate
     S_IMP_XDIE1,               // xdeathstate
     sfx_impdth,                // deathsound
     10,                        // speed
     16 * FRACUNIT,             // radius
     36 * FRACUNIT,             // height
     50,                        // mass
     0,                         // damage
     sfx_impact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,  // flags
     MF2_SPAWNFLOAT | MF2_PASSMOBJ      // flags2
     },

    {                           // MT_IMPLEADER
     5,                         // doomednum
     S_IMP_LOOK1,               // spawnstate
     80,                        // spawnhealth
     S_IMP_FLY1,                // seestate
     sfx_impsit,                // seesound
     8,                         // reactiontime
     sfx_impat2,                // attacksound
     S_IMP_PAIN1,               // painstate
     200,                       // painchance
     sfx_imppai,                // painsound
     S_NULL,                    // meleestate
     S_IMP_MSATK2_1,            // missilestate
     S_IMP_CRASH1,              // crashstate
     S_IMP_DIE1,                // deathstate
     S_IMP_XDIE1,               // xdeathstate
     sfx_impdth,                // deathsound
     10,                        // speed
     16 * FRACUNIT,             // radius
     36 * FRACUNIT,             // height
     50,                        // mass
     0,                         // damage
     sfx_impact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,  // flags
     MF2_SPAWNFLOAT | MF2_PASSMOBJ      // flags2
     },

    {                           // MT_IMPCHUNK1
     -1,                        // doomednum
     S_IMP_CHUNKA1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0                          // flags2
     },

    {                           // MT_IMPCHUNK2
     -1,                        // doomednum
     S_IMP_CHUNKB1,             // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0                          // flags2
     },

    {                           // MT_IMPBALL
     -1,                        // doomednum
     S_IMPFX1,                  // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_IMPFXI1,                 // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_WINDTHRUST | MF2_NOTELEPORT    // flags2
     },

    {                           // MT_KNIGHT
     64,                        // doomednum
     S_KNIGHT_STND1,            // spawnstate
     200,                       // spawnhealth
     S_KNIGHT_WALK1,            // seestate
     sfx_kgtsit,                // seesound
     8,                         // reactiontime
     sfx_kgtatk,                // attacksound
     S_KNIGHT_PAIN1,            // painstate
     100,                       // painchance
     sfx_kgtpai,                // painsound
     S_KNIGHT_ATK1,             // meleestate
     S_KNIGHT_ATK1,             // missilestate
     S_NULL,                    // crashstate
     S_KNIGHT_DIE1,             // deathstate
     S_NULL,                    // xdeathstate
     sfx_kgtdth,                // deathsound
     12,                        // speed
     24 * FRACUNIT,             // radius
     78 * FRACUNIT,             // height
     150,                       // mass
     0,                         // damage
     sfx_kgtact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_KNIGHTGHOST
     65,                        // doomednum
     S_KNIGHT_STND1,            // spawnstate
     200,                       // spawnhealth
     S_KNIGHT_WALK1,            // seestate
     sfx_kgtsit,                // seesound
     8,                         // reactiontime
     sfx_kgtatk,                // attacksound
     S_KNIGHT_PAIN1,            // painstate
     100,                       // painchance
     sfx_kgtpai,                // painsound
     S_KNIGHT_ATK1,             // meleestate
     S_KNIGHT_ATK1,             // missilestate
     S_NULL,                    // crashstate
     S_KNIGHT_DIE1,             // deathstate
     S_NULL,                    // xdeathstate
     sfx_kgtdth,                // deathsound
     12,                        // speed
     24 * FRACUNIT,             // radius
     78 * FRACUNIT,             // height
     150,                       // mass
     0,                         // damage
     sfx_kgtact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_SHADOW,        // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ        // flags2
     },

    {                           // MT_KNIGHTAXE
     -1,                        // doomednum
     S_SPINAXE1,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SPINAXEX1,               // deathstate
     S_NULL,                    // xdeathstate
     sfx_hrnhit,                // deathsound
     9 * FRACUNIT,              // speed
     10 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_WINDTHRUST | MF2_NOTELEPORT | MF2_THRUGHOST    // flags2
     },

    {                           // MT_REDAXE
     -1,                        // doomednum
     S_REDAXE1,                 // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_REDAXEX1,                // deathstate
     S_NULL,                    // xdeathstate
     sfx_hrnhit,                // deathsound
     9 * FRACUNIT,              // speed
     10 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     7,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_THRUGHOST     // flags2
     },

    {                           // MT_SORCERER1
     7,                         // doomednum
     S_SRCR1_LOOK1,             // spawnstate
     2000,                      // spawnhealth
     S_SRCR1_WALK1,             // seestate
     sfx_sbtsit,                // seesound
     8,                         // reactiontime
     sfx_sbtatk,                // attacksound
     S_SRCR1_PAIN1,             // painstate
     56,                        // painchance
     sfx_sbtpai,                // painsound
     S_NULL,                    // meleestate
     S_SRCR1_ATK1,              // missilestate
     S_NULL,                    // crashstate
     S_SRCR1_DIE1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_sbtdth,                // deathsound
     16,                        // speed
     28 * FRACUNIT,             // radius
     100 * FRACUNIT,            // height
     800,                       // mass
     0,                         // damage
     sfx_sbtact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ | MF2_BOSS     // flags2
     },

    {                           // MT_SRCRFX1
     -1,                        // doomednum
     S_SRCRFX1_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SRCRFXI1_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // height
     100,                       // mass
     10,                        // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_FIREDAMAGE    // flags2
     },

    {                           // MT_SORCERER2
     -1,                        // doomednum
     S_SOR2_LOOK1,              // spawnstate
     3500,                      // spawnhealth
     S_SOR2_WALK1,              // seestate
     sfx_sorsit,                // seesound
     8,                         // reactiontime
     sfx_soratk,                // attacksound
     S_SOR2_PAIN1,              // painstate
     32,                        // painchance
     sfx_sorpai,                // painsound
     S_NULL,                    // meleestate
     S_SOR2_ATK1,               // missilestate
     S_NULL,                    // crashstate
     S_SOR2_DIE1,               // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     14,                        // speed
     16 * FRACUNIT,             // radius
     70 * FRACUNIT,             // height
     300,                       // mass
     0,                         // damage
     sfx_soract,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,       // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ | MF2_BOSS     // flags2
     },

    {                           // MT_SOR2FX1
     -1,                        // doomednum
     S_SOR2FX1_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SOR2FXI1_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_SOR2FXSPARK
     -1,                        // doomednum
     S_SOR2FXSPARK1,            // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     MF2_NOTELEPORT | MF2_CANNOTPUSH    // flags2
     },

    {                           // MT_SOR2FX2
     -1,                        // doomednum
     S_SOR2FX2_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_SOR2FXI2_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     6 * FRACUNIT,              // speed
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     10,                        // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT             // flags2
     },

    {                           // MT_SOR2TELEFADE
     -1,                        // doomednum
     S_SOR2TELEFADE1,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0                          // flags2
     },

    {                           // MT_MINOTAUR
     9,                         // doomednum
     S_MNTR_LOOK1,              // spawnstate
     3000,                      // spawnhealth
     S_MNTR_WALK1,              // seestate
     sfx_minsit,                // seesound
     8,                         // reactiontime
     sfx_minat1,                // attacksound
     S_MNTR_PAIN1,              // painstate
     25,                        // painchance
     sfx_minpai,                // painsound
     S_MNTR_ATK1_1,             // meleestate
     S_MNTR_ATK2_1,             // missilestate
     S_NULL,                    // crashstate
     S_MNTR_DIE1,               // deathstate
     S_NULL,                    // xdeathstate
     sfx_mindth,                // deathsound
     16,                        // speed
     28 * FRACUNIT,             // radius
     100 * FRACUNIT,            // height
     800,                       // mass
     7,                         // damage
     sfx_minact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,       // flags
     MF2_FOOTCLIP | MF2_PASSMOBJ | MF2_BOSS     // flags2
     },

    {                           // MT_MNTRFX1
     -1,                        // doomednum
     S_MNTRFX1_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MNTRFXI1_1,              // deathstate
     S_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_FIREDAMAGE    // flags2
     },

    {                           // MT_MNTRFX2
     -1,                        // doomednum
     S_MNTRFX2_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MNTRFXI2_1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_phohit,                // deathsound
     14 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     4,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_FIREDAMAGE    // flags2
     },

    {                           // MT_MNTRFX3
     -1,                        // doomednum
     S_MNTRFX3_1,               // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_MNTRFXI2_1,              // deathstate
     S_NULL,                    // xdeathstate
     sfx_phohit,                // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     4,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     MF2_NOTELEPORT | MF2_FIREDAMAGE    // flags2
     },

    {                           // MT_AKYY
     73,                        // doomednum
     S_AKYY1,                   // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_NOTDMATCH, // flags
     0                          // flags2
     },

    {                           // MT_BKYY
     79,                        // doomednum
     S_BKYY1,                   // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_NOTDMATCH, // flags
     0                          // flags2
     },

    {                           // MT_CKEY
     80,                        // doomednum
     S_CKYY1,                   // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL | MF_NOTDMATCH, // flags
     0                          // flags2
     },

    {                           // MT_AMGWNDWIMPY
     10,                        // doomednum
     S_AMG1,                    // spawnstate
     AMMO_GWND_WIMPY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMGWNDHEFTY
     12,                        // doomednum
     S_AMG2_1,                  // spawnstate
     AMMO_GWND_HEFTY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMMACEWIMPY
     13,                        // doomednum
     S_AMM1,                    // spawnstate
     AMMO_MACE_WIMPY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMMACEHEFTY
     16,                        // doomednum
     S_AMM2,                    // spawnstate
     AMMO_MACE_HEFTY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMCBOWWIMPY
     18,                        // doomednum
     S_AMC1,                    // spawnstate
     AMMO_CBOW_WIMPY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMCBOWHEFTY
     19,                        // doomednum
     S_AMC2_1,                  // spawnstate
     AMMO_CBOW_HEFTY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMSKRDWIMPY
     20,                        // doomednum
     S_AMS1_1,                  // spawnstate
     AMMO_SKRD_WIMPY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMSKRDHEFTY
     21,                        // doomednum
     S_AMS2_1,                  // spawnstate
     AMMO_SKRD_HEFTY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMPHRDWIMPY
     22,                        // doomednum
     S_AMP1_1,                  // spawnstate
     AMMO_PHRD_WIMPY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMPHRDHEFTY
     23,                        // doomednum
     S_AMP2_1,                  // spawnstate
     AMMO_PHRD_HEFTY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMBLSRWIMPY
     54,                        // doomednum
     S_AMB1_1,                  // spawnstate
     AMMO_BLSR_WIMPY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_AMBLSRHEFTY
     55,                        // doomednum
     S_AMB2_1,                  // spawnstate
     AMMO_BLSR_HEFTY,           // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0                          // flags2
     },

    {                           // MT_SOUNDWIND
     42,                        // doomednum
     S_SND_WIND,                // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0                          // flags2
     },

    {                           // MT_SOUNDWATERFALL
     41,                        // doomednum
     S_SND_WATERFALL,           // spawnstate
     1000,                      // spawnhealth
     S_NULL,                    // seestate
     sfx_None,                  // seesound
     8,                         // reactiontime
     sfx_None,                  // attacksound
     S_NULL,                    // painstate
     0,                         // painchance
     sfx_None,                  // painsound
     S_NULL,                    // meleestate
     S_NULL,                    // missilestate
     S_NULL,                    // crashstate
     S_NULL,                    // deathstate
     S_NULL,                    // xdeathstate
     sfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     sfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0                          // flags2
     }
};
