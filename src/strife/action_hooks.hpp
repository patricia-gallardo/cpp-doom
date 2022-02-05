#pragma once

#include "p_mobj.hpp"

void
  A_Look(mobj_t *actor);
void
  A_RandomWalk(mobj_t *actor);
void
  A_FriendLook(mobj_t *actor);
void
  A_Listen(mobj_t *actor);
void
  A_Chase(mobj_t *actor);
void
  A_FaceTarget(mobj_t *actor);
void
  A_PeasantPunch(mobj_t *actor);
void
  A_ReaverAttack(mobj_t *actor);
void
  A_BulletAttack(mobj_t *actor);
void
  A_CheckTargetVisible(mobj_t *actor);
void
  A_SentinelAttack(mobj_t *actor);
void
  A_StalkerThink(mobj_t *actor);
void
  A_StalkerSetLook(mobj_t *actor);
void
  A_StalkerDrop(mobj_t *actor);
void
  A_StalkerScratch(mobj_t *actor);
void
  A_FloatWeave(mobj_t *actor);
void
  A_RobotMelee(mobj_t *actor);
void
  A_TemplarMauler(mobj_t *actor);
void
  A_CrusaderAttack(mobj_t *actor);
void
  A_CrusaderLeft(mobj_t *actor);
void
  A_CrusaderRight(mobj_t *actor);
void
  A_CheckTargetVisible2(mobj_t *actor);
void
  A_InqFlyCheck(mobj_t *actor);
void
  A_InqGrenade(mobj_t *actor);
void
  A_InqTakeOff(mobj_t *actor);
void
  A_InqFly(mobj_t *actor);
void
  A_FireSigilWeapon(mobj_t *actor);
void
  A_ProgrammerAttack(mobj_t *actor);
void
  A_Sigil_A_Action(mobj_t *actor);
void
  A_SpectreEAttack(mobj_t *actor);
void
  A_SpectreCAttack(mobj_t *actor);
void
  A_AlertSpectreC(mobj_t *actor);
void
  A_Sigil_E_Action(mobj_t *actor);
void
  A_SigilTrail(mobj_t *actor);
void
  A_SpectreDAttack(mobj_t *actor);
void
  A_FireSigilEOffshoot(mobj_t *actor);
void
  A_ShadowOff(mobj_t *actor);
void
  A_ModifyVisibility(mobj_t *actor);
void
  A_ShadowOn(mobj_t *actor);
void
  A_SetTLOptions(mobj_t *actor);
void
  A_BossMeleeAtk(mobj_t *actor);
void
  A_BishopAttack(mobj_t *actor);
void
  A_FireHookShot(mobj_t *actor);
void
  A_FireChainShot(mobj_t *actor);
void
  A_MissileSmoke(mobj_t *actor);
void
  A_SpawnSparkPuff(mobj_t *actor);
void
  A_Tracer(mobj_t *actor);
void
  A_ProgrammerMelee(mobj_t *actor);
void
  A_Scream(mobj_t *actor);
void
  A_XScream(mobj_t *actor);
void
  A_Pain(mobj_t *actor);
void
  A_PeasantCrash(mobj_t *actor);
void
  A_Fall(mobj_t *actor);
void
  A_HideZombie(mobj_t *actor);
void
  A_MerchantPain(mobj_t *actor);
void
  A_ProgrammerDie(mobj_t *actor);
void
  A_InqTossArm(mobj_t *actor);
void
  A_SpawnSpectreB(mobj_t *actor);
void
  A_SpawnSpectreD(mobj_t *actor);
void
  A_SpawnSpectreE(mobj_t *actor);
void
  A_SpawnEntity(mobj_t *actor);
void
  A_EntityDeath(mobj_t *actor);
void
  A_SpawnZombie(mobj_t *actor);
void
  A_ZombieInSpecialSector(mobj_t *actor);
void
  A_CrystalExplode(mobj_t *actor);
void
  A_QuestMsg(mobj_t *actor);
void
  A_ExtraLightOff(mobj_t *actor);
void
  A_CrystalRadiusAtk(mobj_t *actor);
void
  A_DeathExplode5(mobj_t *actor);
void
  A_DeathExplode1(mobj_t *actor);
void
  A_DeathExplode2(mobj_t *actor);
void
  A_DeathExplode3(mobj_t *actor);
void
  A_RaiseAlarm(mobj_t *actor);
void
  A_MissileTick(mobj_t *actor);
void
  A_SpawnGrenadeFire(mobj_t *actor);
void
  A_NodeChunk(mobj_t *actor);
void
  A_HeadChunk(mobj_t *actor);
void
  A_BurnSpread(mobj_t *actor);
void
  A_AcolyteSpecial(mobj_t *actor);
void
  A_BossDeath(mobj_t *actor);
void
  A_InqChase(mobj_t *actor);
void
  A_StalkerChase(mobj_t *actor);
void
  A_PlayerScream(mobj_t *actor);
void
  A_TeleportBeacon(mobj_t *actor);
void
  A_BodyParts(mobj_t *actor);
void
  A_ClaxonBlare(mobj_t *actor);
void
  A_ActiveSound(mobj_t *actor);
void
  A_ClearSoundTarget(mobj_t *actor);
void
  A_DropBurnFlesh(mobj_t *actor);
void
  A_FlameDeath(mobj_t *actor);
void
  A_ClearForceField(mobj_t *actor);
void
  A_WeaponReady(player_t *player, pspdef_t *ps);
void
  A_ReFire(player_t *player, pspdef_t *ps);
void
  A_CheckReload(player_t *player, pspdef_t *ps);
void
  A_Lower(player_t *player, pspdef_t *ps);
void
  A_Raise(player_t *player, pspdef_t *ps);
void
  A_GunFlash(player_t *player, pspdef_t *ps);
void
  A_Punch(player_t *player, pspdef_t *ps);
void
  A_FireFlameThrower(player_t *player, pspdef_t *ps);
void
  A_FireMissile(player_t *player, pspdef_t *ps);
void
  A_FireMauler2(player_t *player, pspdef_t *ps);
void
  A_FireGrenade(player_t *player, pspdef_t *ps);
void
  A_FireElectricBolt(player_t *player, pspdef_t *ps);
void
  A_FirePoisonBolt(player_t *player, pspdef_t *ps);
void
  A_FireRifle(player_t *player, pspdef_t *ps);
void
  A_FireMauler1(player_t *player, pspdef_t *ps);
void
  A_SigilSound(player_t *player, pspdef_t *ps);
void
  A_FireSigil(player_t *player, pspdef_t *ps);
void
  A_GunFlashThinker(player_t *player, pspdef_t *ps);
void
  A_Light0(player_t *player, pspdef_t *ps);
void
  A_Light1(player_t *player, pspdef_t *ps);
void
  A_Light2(player_t *player, pspdef_t *ps);
void
  A_SigilShock(player_t *player, pspdef_t *ps);
void
  A_TorpedoExplode(mobj_t *actor);
void
  A_MaulerSound(player_t *player, pspdef_t *ps);
