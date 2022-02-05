//
// Created by jason on 4/3/20.
//

#ifndef CRISPY_DOOM_EVENT_FUNCTION_DECLS_HPP
#define CRISPY_DOOM_EVENT_FUNCTION_DECLS_HPP

struct mobj_t;
struct player_t;
struct pspdef_t;

extern void
  A_Light0(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_WeaponReady(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_Lower(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_Raise(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_Punch(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_ReFire(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_FirePistol(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_Light1(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_FireShotgun(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_Light2(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_FireShotgun2(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_CheckReload(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_OpenShotgun2(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_LoadShotgun2(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_CloseShotgun2(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_FireCGun(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_GunFlash(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_FireMissile(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_Saw(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_FirePlasma(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_BFGsound(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_FireBFG(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_BFGSpray(mobj_t *mo);
extern void
  A_Explode(mobj_t *mo);
extern void
  A_Pain(mobj_t *mo);
extern void
  A_PlayerScream(mobj_t *mo);
extern void
  A_Fall(mobj_t *mo);
extern void
  A_XScream(mobj_t *mo);
extern void
  A_Look(mobj_t *mo);
extern void
  A_Chase(mobj_t *mo);
extern void
  A_FaceTarget(mobj_t *mo);
extern void
  A_PosAttack(mobj_t *mo);
extern void
  A_Scream(mobj_t *mo);
extern void
  A_SPosAttack(mobj_t *mo);
extern void
  A_VileChase(mobj_t *mo);
extern void
  A_VileStart(mobj_t *mo);
extern void
  A_VileTarget(mobj_t *mo);
extern void
  A_VileAttack(mobj_t *mo);
extern void
  A_StartFire(mobj_t *mo);
extern void
  A_Fire(mobj_t *mo);
extern void
  A_FireCrackle(mobj_t *mo);
extern void
  A_Tracer(mobj_t *mo);
extern void
  A_SkelWhoosh(mobj_t *mo);
extern void
  A_SkelFist(mobj_t *mo);
extern void
  A_SkelMissile(mobj_t *mo);
extern void
  A_FatRaise(mobj_t *mo);
extern void
  A_FatAttack1(mobj_t *mo);
extern void
  A_FatAttack2(mobj_t *mo);
extern void
  A_FatAttack3(mobj_t *mo);
extern void
  A_BossDeath(mobj_t *mo);
extern void
  A_CPosAttack(mobj_t *mo);
extern void
  A_CPosRefire(mobj_t *mo);
extern void
  A_TroopAttack(mobj_t *mo);
extern void
  A_SargAttack(mobj_t *mo);
extern void
  A_HeadAttack(mobj_t *mo);
extern void
  A_BruisAttack(mobj_t *mo);
extern void
  A_SkullAttack(mobj_t *mo);
extern void
  A_Metal(mobj_t *mo);
extern void
  A_SpidRefire(mobj_t *mo);
extern void
  A_BabyMetal(mobj_t *mo);
extern void
  A_BspiAttack(mobj_t *mo);
extern void
  A_Hoof(mobj_t *mo);
extern void
  A_CyberAttack(mobj_t *mo);
extern void
  A_PainAttack(mobj_t *mo);
extern void
  A_PainDie(mobj_t *mo);
extern void
  A_KeenDie(mobj_t *mo);
extern void
  A_BrainPain(mobj_t *mo);
extern void
  A_BrainScream(mobj_t *mo);
extern void
  A_BrainDie(mobj_t *mo);
extern void
  A_BrainAwake(mobj_t *mo);
extern void
  A_BrainSpit(mobj_t *mo);
extern void
  A_SpawnSound(mobj_t *mo);
extern void
  A_SpawnFly(mobj_t *mo);
extern void
  A_BrainExplode(mobj_t *mo);
// [crispy] additional BOOM and MBF states, sprites and code pointers
extern void
  A_Stop(mobj_t *mo);
extern void
  A_Die(mobj_t *mo);
extern void
  A_FireOldBFG(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_Detonate(mobj_t *mo);
extern void
  A_Mushroom(mobj_t *mo);
extern void
  A_BetaSkullAttack(mobj_t *mo);
// [crispy] more MBF code pointers
extern void
  A_Spawn(mobj_t *mo);
extern void
  A_Turn(mobj_t *mo);
extern void
  A_Face(mobj_t *mo);
extern void
  A_Scratch(mobj_t *mo);
extern void
  A_PlaySound(mobj_t *mo);
extern void
  A_RandomJump(mobj_t *mo, player_t *player, pspdef_t *psp);
extern void
  A_LineEffect(mobj_t *mo);

#endif // CRISPY_DOOM_EVENT_FUNCTION_DECLS_HPP
