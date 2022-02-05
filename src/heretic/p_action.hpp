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
//
// External definitions for action pointer functions.
//

#ifndef HERETIC_P_ACTION_H
#define HERETIC_P_ACTION_H

void
  A_FreeTargMobj(mobj_t *actor);
void
  A_RestoreSpecialThing1(mobj_t *actor);
void
  A_RestoreSpecialThing2(mobj_t *actor);
void
  A_HideThing(mobj_t *actor);
void
  A_UnHideThing(mobj_t *actor);
void
  A_RestoreArtifact(mobj_t *actor);
void
  A_Scream(mobj_t *actor);
void
  A_Explode(mobj_t *actor);
void
  A_PodPain(mobj_t *actor);
void
  A_RemovePod(mobj_t *actor);
void
  A_MakePod(mobj_t *actor);
void
  A_InitKeyGizmo(mobj_t *actor);
void
  A_VolcanoSet(mobj_t *actor);
void
  A_VolcanoBlast(mobj_t *actor);
void
  A_BeastPuff(mobj_t *actor);
void
  A_VolcBallImpact(mobj_t *actor);
void
  A_SpawnTeleGlitter(mobj_t *actor);
void
  A_SpawnTeleGlitter2(mobj_t *actor);
void
  A_AccTeleGlitter(mobj_t *actor);
void
  A_Light0(player_t *player, pspdef_t *psp);
void
  A_WeaponReady(player_t *player, pspdef_t *psp);
void
  A_Lower(player_t *player, pspdef_t *psp);
void
  A_Raise(player_t *player, pspdef_t *psp);
void
  A_StaffAttackPL1(player_t *player, pspdef_t *psp);
void
  A_ReFire(player_t *player, pspdef_t *psp);
void
  A_StaffAttackPL2(player_t *player, pspdef_t *psp);
void
  A_BeakReady(player_t *player, pspdef_t *psp);
void
  A_BeakRaise(player_t *player, pspdef_t *psp);
void
  A_BeakAttackPL1(player_t *player, pspdef_t *psp);
void
  A_BeakAttackPL2(player_t *player, pspdef_t *psp);
void
  A_GauntletAttack(player_t *player, pspdef_t *psp);
void
  A_FireBlasterPL1(player_t *player, pspdef_t *psp);
void
  A_FireBlasterPL2(player_t *player, pspdef_t *psp);
void
  A_SpawnRippers(mobj_t *actor);
void
  A_FireMacePL1(player_t *player, pspdef_t *psp);
void
  A_FireMacePL2(player_t *player, pspdef_t *psp);
void
  A_MacePL1Check(mobj_t *actor);
void
  A_MaceBallImpact(mobj_t *actor);
void
  A_MaceBallImpact2(mobj_t *actor);
void
  A_DeathBallImpact(mobj_t *actor);
void
  A_FireSkullRodPL1(player_t *player, pspdef_t *psp);
void
  A_FireSkullRodPL2(player_t *player, pspdef_t *psp);
void
  A_SkullRodPL2Seek(mobj_t *actor);
void
  A_AddPlayerRain(mobj_t *actor);
void
  A_HideInCeiling(mobj_t *actor);
void
  A_SkullRodStorm(mobj_t *actor);
void
  A_RainImpact(mobj_t *actor);
void
  A_FireGoldWandPL1(player_t *player, pspdef_t *psp);
void
  A_FireGoldWandPL2(player_t *player, pspdef_t *psp);
void
  A_FirePhoenixPL1(player_t *player, pspdef_t *psp);
void
  A_InitPhoenixPL2(player_t *player, pspdef_t *psp);
void
  A_FirePhoenixPL2(player_t *player, pspdef_t *psp);
void
  A_ShutdownPhoenixPL2(player_t *player, pspdef_t *psp);
void
  A_PhoenixPuff(mobj_t *actor);
void
  A_RemovedPhoenixFunc(mobj_t *actor);
void
  A_FlameEnd(mobj_t *actor);
void
  A_FloatPuff(mobj_t *actor);
void
  A_FireCrossbowPL1(player_t *player, pspdef_t *psp);
void
  A_FireCrossbowPL2(player_t *player, pspdef_t *psp);
void
  A_BoltSpark(mobj_t *actor);
void
  A_Pain(mobj_t *actor);
void
  A_NoBlocking(mobj_t *actor);
void
  A_AddPlayerCorpse(mobj_t *actor);
void
  A_SkullPop(mobj_t *actor);
void
  A_FlameSnd(mobj_t *actor);
void
  A_CheckBurnGone(mobj_t *actor);
void
  A_CheckSkullFloor(mobj_t *actor);
void
  A_CheckSkullDone(mobj_t *actor);
void
  A_Feathers(mobj_t *actor);
void
  A_ChicLook(mobj_t *actor);
void
  A_ChicChase(mobj_t *actor);
void
  A_ChicPain(mobj_t *actor);
void
  A_FaceTarget(mobj_t *actor);
void
  A_ChicAttack(mobj_t *actor);
void
  A_Look(mobj_t *actor);
void
  A_Chase(mobj_t *actor);
void
  A_MummyAttack(mobj_t *actor);
void
  A_MummyAttack2(mobj_t *actor);
void
  A_MummySoul(mobj_t *actor);
void
  A_ContMobjSound(mobj_t *actor);
void
  A_MummyFX1Seek(mobj_t *actor);
void
  A_BeastAttack(mobj_t *actor);
void
  A_SnakeAttack(mobj_t *actor);
void
  A_SnakeAttack2(mobj_t *actor);
void
  A_HeadAttack(mobj_t *actor);
void
  A_BossDeath(mobj_t *actor);
void
  A_HeadIceImpact(mobj_t *actor);
void
  A_HeadFireGrow(mobj_t *actor);
void
  A_WhirlwindSeek(mobj_t *actor);
void
  A_ClinkAttack(mobj_t *actor);
void
  A_WizAtk1(mobj_t *actor);
void
  A_WizAtk2(mobj_t *actor);
void
  A_WizAtk3(mobj_t *actor);
void
  A_GhostOff(mobj_t *actor);
void
  A_ImpMeAttack(mobj_t *actor);
void
  A_ImpMsAttack(mobj_t *actor);
void
  A_ImpMsAttack2(mobj_t *actor);
void
  A_ImpDeath(mobj_t *actor);
void
  A_ImpXDeath1(mobj_t *actor);
void
  A_ImpXDeath2(mobj_t *actor);
void
  A_ImpExplode(mobj_t *actor);
void
  A_KnightAttack(mobj_t *actor);
void
  A_DripBlood(mobj_t *actor);
void
  A_Sor1Chase(mobj_t *actor);
void
  A_Sor1Pain(mobj_t *actor);
void
  A_Srcr1Attack(mobj_t *actor);
void
  A_SorZap(mobj_t *actor);
void
  A_SorcererRise(mobj_t *actor);
void
  A_SorRise(mobj_t *actor);
void
  A_SorSightSnd(mobj_t *actor);
void
  A_Srcr2Decide(mobj_t *actor);
void
  A_Srcr2Attack(mobj_t *actor);
void
  A_Sor2DthInit(mobj_t *actor);
void
  A_SorDSph(mobj_t *actor);
void
  A_Sor2DthLoop(mobj_t *actor);
void
  A_SorDExp(mobj_t *actor);
void
  A_SorDBon(mobj_t *actor);
void
  A_BlueSpark(mobj_t *actor);
void
  A_GenWizard(mobj_t *actor);
void
  A_MinotaurAtk1(mobj_t *actor);
void
  A_MinotaurDecide(mobj_t *actor);
void
  A_MinotaurAtk2(mobj_t *actor);
void
  A_MinotaurAtk3(mobj_t *actor);
void
  A_MinotaurCharge(mobj_t *actor);
void
  A_MntrFloorFire(mobj_t *actor);
void
  A_ESound(mobj_t *actor);

#endif /* #ifndef HERETIC_P_ACTION_H */
