//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard, Andrey Budko
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
//	Movement, collision handling.
//	Shooting and aiming.
//

#include <cstdio>
#include <cstdlib>

#include "deh_misc.hpp"

#include "m_bbox.hpp"
#include "m_random.hpp"
#include "i_system.hpp"

#include "doomdef.hpp"
#include "m_argv.hpp"
#include "m_misc.hpp"
#include "p_local.hpp"

#include "s_sound.hpp"

// State.
#include "doomstat.hpp"
#include "r_state.hpp"
// Data.
#include "sounds.hpp"

// Spechit overrun magic value.
//
// This is the value used by PrBoom-plus.  I think the value below is
// actually better and works with more demos.  However, I think
// it's better for the spechits emulation to be compatible with
// PrBoom-plus, at least so that the big spechits emulation list
// on Doomworld can also be used with Chocolate Doom.

#define DEFAULT_SPECHIT_MAGIC 0x01C09C98

// This is from a post by myk on the Doomworld forums,
// outputted from entryway's spechit_magic generator for
// s205n546.lmp.  The _exact_ value of this isn't too
// important; as long as it is in the right general
// range, it will usually work.  Otherwise, we can use
// the generator (hacked doom2.exe) and provide it
// with -spechit.

//#define DEFAULT_SPECHIT_MAGIC 0x84f968e8


fixed_t tmbbox[4];
mobj_t *tmthing;
int     tmflags;
fixed_t tmx;
fixed_t tmy;

fixed_t tmdropoffz;

//
// TELEPORT MOVE
//

//
// PIT_StompThing
//
bool PIT_StompThing(mobj_t *thing)
{
    fixed_t blockdist;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (std::abs(thing->x - tmx) >= blockdist
        || std::abs(thing->y - tmy) >= blockdist)
    {
        // didn't hit it
        return true;
    }

    // don't clip against self
    if (thing == tmthing)
        return true;

    // monsters don't stomp things except on boss level
    if (!tmthing->player && g_doomstat_globals->gamemap != 30)
        return false;

    P_DamageMobj(thing, tmthing, tmthing, 10000);

    return true;
}


//
// P_TeleportMove
//
bool
    P_TeleportMove(mobj_t *thing,
        fixed_t            x,
        fixed_t            y)
{
    int xl;
    int xh;
    int yl;
    int yh;
    int bx;
    int by;

    subsector_t *newsubsec;

    // kill anything occupying the position
    tmthing = thing;
    tmflags = static_cast<int>(thing->flags);

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP]    = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT]  = x + tmthing->radius;
    tmbbox[BOXLEFT]   = x - tmthing->radius;

    newsubsec   = R_PointInSubsector(x, y);
    g_p_local_globals->ceilingline = nullptr;

    // The base floor/ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    g_p_local_globals->tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    g_p_local_globals->tmceilingz            = newsubsec->sector->ceilingheight;

    validcount++;
    g_p_local_globals->numspechit = 0;

    // stomp on any things contacted
    xl = (tmbbox[BOXLEFT] - g_p_local_globals->bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - g_p_local_globals->bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - g_p_local_globals->bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - g_p_local_globals->bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_StompThing))
                return false;

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition(thing);

    thing->floorz   = g_p_local_globals->tmfloorz;
    thing->ceilingz = g_p_local_globals->tmceilingz;
    thing->x        = x;
    thing->y        = y;

    // [AM] Don't interpolate mobjs that pass
    //      through teleporters
    thing->interp = false;

    P_SetThingPosition(thing);

    return true;
}


//
// MOVEMENT ITERATOR FUNCTIONS
//

static void SpechitOverrun(line_t *ld);

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
bool PIT_CheckLine(line_t *ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return true;

    if (P_BoxOnLineSide(tmbbox, ld) != -1)
        return true;

    // A line has been hit

    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.

    if (!ld->backsector)
        return false; // one sided line

    if (!(static_cast<unsigned int>(tmthing->flags) & MF_MISSILE))
    {
        if (ld->flags & ML_BLOCKING)
            return false; // explicitly blocking everything

        if (!tmthing->player && ld->flags & ML_BLOCKMONSTERS)
            return false; // block monsters only
    }

    // set openrange, opentop, openbottom
    P_LineOpening(ld);

    // adjust floor / ceiling heights
    if (g_p_local_globals->opentop < g_p_local_globals->tmceilingz)
    {
        g_p_local_globals->tmceilingz  = g_p_local_globals->opentop;
        g_p_local_globals->ceilingline = ld;
    }

    if (g_p_local_globals->openbottom > g_p_local_globals->tmfloorz)
        g_p_local_globals->tmfloorz = g_p_local_globals->openbottom;

    if (g_p_local_globals->lowfloor < tmdropoffz)
        tmdropoffz = g_p_local_globals->lowfloor;

    // if contacted a special line, add it to the list
    if (ld->special)
    {
        g_p_local_globals->spechit[g_p_local_globals->numspechit] = ld;
        g_p_local_globals->numspechit++;

        // fraggle: spechits overrun emulation code from prboom-plus
        if (g_p_local_globals->numspechit > MAXSPECIALCROSS_ORIGINAL)
        {
            // [crispy] print a warning
            if (g_p_local_globals->numspechit == MAXSPECIALCROSS_ORIGINAL + 1)
                fprintf(stderr, "PIT_CheckLine: Triggered SPECHITS overflow!\n");
            SpechitOverrun(ld);
        }
    }

    return true;
}

//
// PIT_CheckThing
//
bool PIT_CheckThing(mobj_t *thing)
{
    fixed_t blockdist;
    bool solid;
    bool unblocking = false;
    int     damage;

    if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (std::abs(thing->x - tmx) >= blockdist
        || std::abs(thing->y - tmy) >= blockdist)
    {
        // didn't hit it
        return true;
    }

    // don't clip against self
    if (thing == tmthing)
        return true;

    // check for skulls slamming into things
    if (static_cast<unsigned int>(tmthing->flags) & MF_SKULLFLY)
    {
        // [crispy] check if attacking skull flies over player
        if (critical->overunder && thing->player)
        {
            if (tmthing->z > thing->z + thing->height)
            {
                return true;
            }
        }

        damage = ((P_Random() % 8) + 1) * tmthing->info->damage;

        P_DamageMobj(thing, tmthing, tmthing, damage);

        tmthing->flags &= ~MF_SKULLFLY;
        tmthing->momx = tmthing->momy = tmthing->momz = 0;

        P_SetMobjState(tmthing,
            tmthing->info->spawnstate);

        return false; // stop moving
    }


    // missiles can hit other things
    if (static_cast<unsigned int>(tmthing->flags) & MF_MISSILE)
    {
        // [crispy] mobj or actual sprite height
        const fixed_t thingheight = (tmthing->target && tmthing->target->player && critical->freeaim == FREEAIM_DIRECT) ?
                                        thing->info->actualheight :
                                        thing->height;
        // see if it went over / under
        if (tmthing->z > thing->z + thingheight)
            return true; // overhead
        if (tmthing->z + tmthing->height < thing->z)
            return true; // underneath

        if (tmthing->target
            && (tmthing->target->type == thing->type || (tmthing->target->type == MT_KNIGHT && thing->type == MT_BRUISER) || (tmthing->target->type == MT_BRUISER && thing->type == MT_KNIGHT)))
        {
            // Don't hit same species as originator.
            if (thing == tmthing->target)
                return true;

            // sdh: Add deh_species_infighting here.  We can override the
            // "monsters of the same species cant hurt each other" behavior
            // through dehacked patches

            if (thing->type != MT_PLAYER && !deh_species_infighting)
            {
                // Explode, but do no damage.
                // Let players missile other players.
                return false;
            }
        }

        if (!(thing->flags & MF_SHOOTABLE))
        {
            // didn't do any damage
            return !(thing->flags & MF_SOLID);
        }

        // damage / explode
        damage = ((P_Random() % 8) + 1) * tmthing->info->damage;
        P_DamageMobj(thing, tmthing, tmthing->target, damage);

        // don't traverse any more
        return false;
    }

    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
        solid = (thing->flags & MF_SOLID) != 0;
        if (static_cast<unsigned int>(tmflags) & MF_PICKUP)
        {
            // can remove thing
            P_TouchSpecialThing(thing, tmthing);
        }
        return !solid;
    }

    if (critical->overunder)
    {
        // [crispy] a solid hanging body will allow sufficiently small things underneath it
        if (thing->flags & MF_SOLID && thing->flags & MF_SPAWNCEILING)
        {
            if (tmthing->z + tmthing->height <= thing->z)
            {
                if (thing->z < g_p_local_globals->tmceilingz)
                {
                    g_p_local_globals->tmceilingz = thing->z;
                }
                return true;
            }
        }

        // [crispy] allow players to walk over/under shootable objects
        if (tmthing->player && thing->flags & MF_SHOOTABLE)
        {
            fixed_t newfloorz, newceilingz;
            // [crispy] allow the usual 24 units step-up even across monsters' heads,
            // only if the current height has not been reached by "low" jumping
            fixed_t step_up = tmthing->player->jumpTics > 7 ? 0 : 24 * FRACUNIT;

            if (tmthing->z + step_up >= thing->z + thing->height)
            {
                // player walks over object
                if ((newfloorz = thing->z + thing->height) > g_p_local_globals->tmfloorz)
                {
                    g_p_local_globals->tmfloorz = newfloorz;
                }
                if ((newceilingz = tmthing->z) < thing->ceilingz)
                {
                    thing->ceilingz = newceilingz;
                }
                return true;
            }
            else if (tmthing->z + tmthing->height <= thing->z)
            {
                // player walks underneath object
                if ((newceilingz = thing->z) < g_p_local_globals->tmceilingz)
                {
                    g_p_local_globals->tmceilingz = newceilingz;
                }
                if ((newfloorz = tmthing->z + tmthing->height) > thing->floorz)
                {
                    thing->floorz = newfloorz;
                }
                return true;
            }

            // [crispy] check if things are stuck and allow them to move further apart
            // taken from doomretro/src/p_map.c:319-332
            if (tmx == tmthing->x && tmy == tmthing->y)
            {
                unblocking = true;
            }
            else
            {
                fixed_t newdist = P_AproxDistance(thing->x - tmx, thing->y - tmy);
                fixed_t olddist = P_AproxDistance(thing->x - tmthing->x, thing->y - tmthing->y);

                if (newdist > olddist)
                {
                    unblocking = (tmthing->z < thing->z + thing->height
                                  && tmthing->z + tmthing->height > thing->z);
                }
            }
        }
    }

    return !(thing->flags & MF_SOLID) || unblocking;
}


//
// MOVEMENT CLIPPING
//

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
//
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//
bool
    P_CheckPosition(mobj_t *thing,
        fixed_t             x,
        fixed_t             y)
{
    int          xl;
    int          xh;
    int          yl;
    int          yh;
    int          bx;
    int          by;
    subsector_t *newsubsec;

    tmthing = thing;
    tmflags = static_cast<int>(thing->flags);

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP]    = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT]  = x + tmthing->radius;
    tmbbox[BOXLEFT]   = x - tmthing->radius;

    newsubsec   = R_PointInSubsector(x, y);
    g_p_local_globals->ceilingline = nullptr;

    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    g_p_local_globals->tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    g_p_local_globals->tmceilingz            = newsubsec->sector->ceilingheight;

    validcount++;
    g_p_local_globals->numspechit = 0;

    if (static_cast<unsigned int>(tmflags) & MF_NOCLIP)
        return true;

    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.
    xl = (tmbbox[BOXLEFT] - g_p_local_globals->bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - g_p_local_globals->bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - g_p_local_globals->bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - g_p_local_globals->bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_CheckThing))
                return false;

    // check lines
    xl = (tmbbox[BOXLEFT] - g_p_local_globals->bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - g_p_local_globals->bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - g_p_local_globals->bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - g_p_local_globals->bmaporgy) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx, by, PIT_CheckLine))
                return false;

    return true;
}


//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
bool
    P_TryMove(mobj_t *thing,
        fixed_t       x,
        fixed_t       y)
{
    fixed_t oldx;
    fixed_t oldy;
    int     side;
    int     oldside;
    line_t *ld;

    g_p_local_globals->floatok = false;
    if (!P_CheckPosition(thing, x, y))
        return false; // solid wall or thing

    if (!(thing->flags & MF_NOCLIP))
    {
        if (g_p_local_globals->tmceilingz - g_p_local_globals->tmfloorz < thing->height)
            return false; // doesn't fit

        g_p_local_globals->floatok = true;

        if (!(thing->flags & MF_TELEPORT)
            && g_p_local_globals->tmceilingz - thing->z < thing->height)
            return false; // mobj must lower itself to fit

        if (!(thing->flags & MF_TELEPORT)
            && g_p_local_globals->tmfloorz - thing->z > 24 * FRACUNIT)
            return false; // too big a step up

        if (!(thing->flags & (MF_DROPOFF | MF_FLOAT))
            && g_p_local_globals->tmfloorz - tmdropoffz > 24 * FRACUNIT)
            return false; // don't stand over a dropoff
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition(thing);

    oldx            = thing->x;
    oldy            = thing->y;
    thing->floorz   = g_p_local_globals->tmfloorz;
    thing->ceilingz = g_p_local_globals->tmceilingz;
    thing->x        = x;
    thing->y        = y;

    P_SetThingPosition(thing);

    // if any special lines were hit, do the effect
    if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
    {
        while (g_p_local_globals->numspechit--)
        {
            // see if the line was crossed
            ld      = g_p_local_globals->spechit[g_p_local_globals->numspechit];
            side    = P_PointOnLineSide(thing->x, thing->y, ld);
            oldside = P_PointOnLineSide(oldx, oldy, ld);
            if (side != oldside)
            {
                if (ld->special)
                    P_CrossSpecialLine(static_cast<int>(ld - g_r_state_globals->lines), oldside, thing);
            }
        }
    }

    return true;
}


//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//
bool P_ThingHeightClip(mobj_t *thing)
{
    bool onfloor;

    onfloor = (thing->z == thing->floorz);

    P_CheckPosition(thing, thing->x, thing->y);
    // what about stranding a monster partially off an edge?

    thing->floorz   = g_p_local_globals->tmfloorz;
    thing->ceilingz = g_p_local_globals->tmceilingz;

    if (onfloor)
    {
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;
    }
    else
    {
        // don't adjust a floating monster unless forced to
        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;
    }

    if (thing->ceilingz - thing->floorz < thing->height)
        return false;

    return true;
}


//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t bestslidefrac;
fixed_t secondslidefrac;

line_t *bestslideline;
line_t *secondslideline;

mobj_t *slidemo;

fixed_t tmxmove;
fixed_t tmymove;


//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
void P_HitSlideLine(line_t *ld)
{
    int side;

    angle_t lineangle;
    angle_t moveangle;
    angle_t deltaangle;

    fixed_t movelen;
    fixed_t newlen;


    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
        return;
    }

    side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);

    if (side == 1)
        lineangle += ANG180;

    moveangle  = R_PointToAngle2(0, 0, tmxmove, tmymove);
    deltaangle = moveangle - lineangle;

    if (deltaangle > ANG180)
        deltaangle += ANG180;
    //	I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance(tmxmove, tmymove);
    newlen  = FixedMul(movelen, finecosine[deltaangle]);

    tmxmove = FixedMul(newlen, finecosine[lineangle]);
    tmymove = FixedMul(newlen, finesine[lineangle]);
}


//
// PTR_SlideTraverse
//
bool PTR_SlideTraverse(intercept_t *in)
{
    line_t *li;

    if (!in->isaline)
        I_Error("PTR_SlideTraverse: not a line?");

    li = in->d.line;

    if (!(li->flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
        {
            // don't hit the back side
            return true;
        }
        goto isblocking;
    }

    // set openrange, opentop, openbottom
    P_LineOpening(li);

    if (g_p_local_globals->openrange < slidemo->height)
        goto isblocking; // doesn't fit

    if (g_p_local_globals->opentop - slidemo->z < slidemo->height)
        goto isblocking; // mobj is too high

    if (g_p_local_globals->openbottom - slidemo->z > 24 * FRACUNIT)
        goto isblocking; // too big a step up

    // this line doesn't block movement
    return true;

    // the line does block movement,
    // see if it is closer than best so far
isblocking:
    if (in->frac < bestslidefrac)
    {
        secondslidefrac = bestslidefrac;
        secondslideline = bestslideline;
        bestslidefrac   = in->frac;
        bestslideline   = li;
    }

    return false; // stop
}


//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
void P_SlideMove(mobj_t *mo)
{
    fixed_t leadx;
    fixed_t leady;
    fixed_t trailx;
    fixed_t traily;
    fixed_t newx;
    fixed_t newy;
    int     hitcount;

    slidemo  = mo;
    hitcount = 0;

retry:
    if (++hitcount == 3)
        goto stairstep; // don't loop forever


    // trace along the three leading corners
    if (mo->momx > 0)
    {
        leadx  = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx  = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if (mo->momy > 0)
    {
        leady  = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady  = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT + 1;

    P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);

    // move up to the wall
    if (bestslidefrac == FRACUNIT + 1)
    {
        // the move most have hit the middle, so stairstep
    stairstep:
        if (!P_TryMove(mo, mo->x, mo->y + mo->momy))
            P_TryMove(mo, mo->x + mo->momx, mo->y);
        return;
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;
    if (bestslidefrac > 0)
    {
        newx = FixedMul(mo->momx, bestslidefrac);
        newy = FixedMul(mo->momy, bestslidefrac);

        if (!P_TryMove(mo, mo->x + newx, mo->y + newy))
            goto stairstep;
    }

    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = FRACUNIT - (bestslidefrac + 0x800);

    if (bestslidefrac > FRACUNIT)
        bestslidefrac = FRACUNIT;

    if (bestslidefrac <= 0)
        return;

    tmxmove = FixedMul(mo->momx, bestslidefrac);
    tmymove = FixedMul(mo->momy, bestslidefrac);

    P_HitSlideLine(bestslideline); // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove))
    {
        goto retry;
    }
}


//
// P_LineAttack
//
mobj_t *shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
fixed_t shootz;

int     la_damage;
fixed_t attackrange;

fixed_t aimslope;

// slopes to top and bottom of target
extern fixed_t topslope;
extern fixed_t bottomslope;

extern degenmobj_t *laserspot;

//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
bool
    PTR_AimTraverse(intercept_t *in)
{
    line_t *li;
    mobj_t *th;
    fixed_t slope;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;
    fixed_t dist;

    if (in->isaline)
    {
        li = in->d.line;

        if (!(li->flags & ML_TWOSIDED))
            return false; // stop

        // Crosses a two sided line.
        // A two sided line will restrict
        // the possible target ranges.
        P_LineOpening(li);

        if (g_p_local_globals->openbottom >= g_p_local_globals->opentop)
            return false; // stop

        dist = FixedMul(attackrange, in->frac);

        if (li->backsector == nullptr
            || li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv(g_p_local_globals->openbottom - shootz, dist);
            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (li->backsector == nullptr
            || li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv(g_p_local_globals->opentop - shootz, dist);
            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false; // stop

        return true; // shot continues
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true; // can't shoot self

    if (!(th->flags & MF_SHOOTABLE))
        return true; // corpse or something

    // check angles to see if the thing can be aimed at
    dist          = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);

    if (thingtopslope < bottomslope)
        return true; // shot over the thing

    thingbottomslope = FixedDiv(th->z - shootz, dist);

    if (thingbottomslope > topslope)
        return true; // shot under the thing

    // this thing can be hit!
    if (thingtopslope > topslope)
        thingtopslope = topslope;

    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope   = (thingtopslope + thingbottomslope) / 2;
    g_p_local_globals->linetarget = th;

    return false; // don't go any farther
}


//
// PTR_ShootTraverse
//
bool PTR_ShootTraverse(intercept_t *in)
{
    fixed_t x;
    fixed_t y;
    fixed_t z;
    fixed_t frac;

    line_t *li;

    mobj_t *th;

    fixed_t slope;
    fixed_t dist;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;
    fixed_t thingheight; // [crispy] mobj or actual sprite height

    if (in->isaline)
    {
        bool safe = false;
        li           = in->d.line;

        // [crispy] laser spot does not shoot any line
        if (li->special && la_damage > INT_MIN)
            P_ShootSpecialLine(shootthing, li);

        if (!(li->flags & ML_TWOSIDED))
            goto hitline;

        // crosses a two sided line
        P_LineOpening(li);

        dist = FixedMul(attackrange, in->frac);

        // e6y: emulation of missed back side on two-sided lines.
        // backsector can be nullptr when emulating missing back side.

        if (li->backsector == nullptr)
        {
            slope = FixedDiv(g_p_local_globals->openbottom - shootz, dist);
            if (slope > aimslope)
                goto hitline;

            slope = FixedDiv(g_p_local_globals->opentop - shootz, dist);
            if (slope < aimslope)
                goto hitline;
        }
        else
        {
            if (li->frontsector->floorheight != li->backsector->floorheight)
            {
                slope = FixedDiv(g_p_local_globals->openbottom - shootz, dist);
                if (slope > aimslope)
                    goto hitline;
            }

            if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
            {
                slope = FixedDiv(g_p_local_globals->opentop - shootz, dist);
                if (slope < aimslope)
                    goto hitline;
            }
        }

        // shot continues
        return true;


        // hit line
    hitline:
        // position a bit closer
        frac = in->frac - FixedDiv(4 * FRACUNIT, attackrange);
        x    = g_p_local_globals->trace.x + FixedMul(g_p_local_globals->trace.dx, frac);
        y    = g_p_local_globals->trace.y + FixedMul(g_p_local_globals->trace.dy, frac);
        z    = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

        if (li->frontsector->ceilingpic == g_doomstat_globals->skyflatnum)
        {
            // don't shoot the sky!
            if (z > li->frontsector->ceilingheight)
                return false;

            // it's a sky hack wall
            if (li->backsector && li->backsector->ceilingpic == g_doomstat_globals->skyflatnum)
            {
                // [crispy] fix bullet puffs and laser spot not appearing in outdoor areas
                if (li->backsector->ceilingheight < z)
                    return false;
                else
                    safe = true;
            }
        }

        // [crispy] check if the pullet puff's z-coordinate is below of above
        // its spawning sector's floor or ceiling, respectively, and move its
        // coordinates to the point where the trajectory hits the plane
        if (aimslope)
        {
            const int lineside = P_PointOnLineSide(x, y, li);
            int       side;

            if ((side = li->sidenum[lineside]) != NO_INDEX)
            {
                const sector_t *const sector = g_r_state_globals->sides[side].sector;

                if (z < sector->floorheight || (z > sector->ceilingheight && sector->ceilingpic != g_doomstat_globals->skyflatnum))
                {
                    z    = BETWEEN(sector->floorheight, sector->ceilingheight, z);
                    frac = FixedDiv(z - shootz, FixedMul(aimslope, attackrange));
                    x    = g_p_local_globals->trace.x + FixedMul(g_p_local_globals->trace.dx, frac);
                    y    = g_p_local_globals->trace.y + FixedMul(g_p_local_globals->trace.dy, frac);
                }
            }
        }

        // [crispy] update laser spot position and return
        if (la_damage == INT_MIN)
        {
            laserspot->thinker.function     = valid_hook(true);
            laserspot->x                    = x;
            laserspot->y                    = y;
            laserspot->z                    = z;
            return false;
        }

        // Spawn bullet puffs.
        P_SpawnPuffSafe(x, y, z, safe);

        // don't go any farther
        return false;
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true; // can't shoot self

    if (!(th->flags & MF_SHOOTABLE))
        return true; // corpse or something

    // check angles to see if the thing can be aimed at
    dist = FixedMul(attackrange, in->frac);
    // [crispy] mobj or actual sprite height
    thingheight = (shootthing->player && critical->freeaim == FREEAIM_DIRECT) ?
                      th->info->actualheight :
                      th->height;
    thingtopslope = FixedDiv(th->z + thingheight - shootz, dist);

    if (thingtopslope < aimslope)
        return true; // shot over the thing

    thingbottomslope = FixedDiv(th->z - shootz, dist);

    if (thingbottomslope > aimslope)
        return true; // shot under the thing


    // hit thing
    // position a bit closer
    frac = in->frac - FixedDiv(10 * FRACUNIT, attackrange);

    x = g_p_local_globals->trace.x + FixedMul(g_p_local_globals->trace.dx, frac);
    y = g_p_local_globals->trace.y + FixedMul(g_p_local_globals->trace.dy, frac);
    z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

    // [crispy] update laser spot position and return
    if (la_damage == INT_MIN)
    {
        // [crispy] pass through Spectres
        if (th->flags & MF_SHADOW)
            return true;

        laserspot->thinker.function     = valid_hook(true);
        laserspot->x                    = th->x;
        laserspot->y                    = th->y;
        laserspot->z                    = z;
        return false;
    }

    // Spawn bullet puffs or blod spots,
    // depending on target type.
    if (static_cast<unsigned int>(in->d.thing->flags) & MF_NOBLOOD)
        P_SpawnPuff(x, y, z);
    else
        P_SpawnBlood(x, y, z, la_damage, th); // [crispy] pass thing type

    if (la_damage)
        P_DamageMobj(th, shootthing, shootthing, la_damage);

    // don't go any farther
    return false;
}


//
// P_AimLineAttack
//
fixed_t
    P_AimLineAttack(mobj_t *t1,
        angle_t             angle,
        fixed_t             distance)
{
    fixed_t x2;
    fixed_t y2;

    t1 = P_SubstNullMobj(t1);

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;

    x2     = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2     = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;

    // can't shoot outside view angles
    topslope    = (ORIGHEIGHT / 2) * FRACUNIT / (ORIGWIDTH / 2);
    bottomslope = -(ORIGHEIGHT / 2) * FRACUNIT / (ORIGWIDTH / 2);

    attackrange = distance;
    g_p_local_globals->linetarget  = nullptr;

    P_PathTraverse(t1->x, t1->y,
        x2, y2,
        PT_ADDLINES | PT_ADDTHINGS,
        PTR_AimTraverse);

    if (g_p_local_globals->linetarget)
        return aimslope;

    return 0;
}


//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
// [crispy] if damage == INT_MIN, it is a trace
// to update the laser spot position
//
void P_LineAttack(mobj_t *t1,
    angle_t               angle,
    fixed_t               distance,
    fixed_t               slope,
    int                   damage)
{
    // [crispy] smooth laser spot movement with uncapped framerate
    const fixed_t t1x = (damage == INT_MIN) ? g_r_state_globals->viewx : t1->x;
    const fixed_t t1y = (damage == INT_MIN) ? g_r_state_globals->viewy : t1->y;
    fixed_t       x2;
    fixed_t       y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing  = t1;
    la_damage   = damage;
    x2          = t1x + (distance >> FRACBITS) * finecosine[angle];
    y2          = t1y + (distance >> FRACBITS) * finesine[angle];
    shootz      = (damage == INT_MIN) ? g_r_state_globals->viewz : t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    attackrange = distance;
    aimslope    = slope;

    P_PathTraverse(t1x, t1y,
        x2, y2,
        PT_ADDLINES | PT_ADDTHINGS,
        PTR_ShootTraverse);
}

// [crispy] update laser spot position
// call P_AimLineAttack() to check if a target is aimed at (linetarget)
// then call P_LineAttack() with either aimslope or the passed slope
void P_LineLaser(mobj_t *t1,
    angle_t              angle,
    fixed_t              distance,
    fixed_t              slope)
{
    fixed_t lslope;

    laserspot->thinker.function = null_hook();

    // [crispy] intercepts overflow guard
    crispy->crosshair |= CROSSHAIR_INTERCEPT;

    // [crispy] set the linetarget pointer
    lslope = P_AimLineAttack(t1, angle, distance);

    if (critical->freeaim == FREEAIM_DIRECT)
    {
        lslope = slope;
    }
    else
    {
        // [crispy] increase accuracy
        if (!g_p_local_globals->linetarget)
        {
            angle_t an = angle;

            an += 1 << 26;
            lslope = P_AimLineAttack(t1, an, distance);

            if (!g_p_local_globals->linetarget)
            {
                an -= 2 << 26;
                lslope = P_AimLineAttack(t1, an, distance);

                if (!g_p_local_globals->linetarget && critical->freeaim == FREEAIM_BOTH)
                {
                    lslope = slope;
                }
            }
        }
    }

    if ((crispy->crosshair & ~CROSSHAIR_INTERCEPT) == CROSSHAIR_PROJECTED)
    {
        // [crispy] don't aim at Spectres
        if (g_p_local_globals->linetarget && !(g_p_local_globals->linetarget->flags & MF_SHADOW) && (crispy->freeaim != FREEAIM_DIRECT))
            P_LineAttack(t1, angle, distance, aimslope, INT_MIN);
        else
            // [crispy] double the auto aim distance
            P_LineAttack(t1, angle, 2 * distance, lslope, INT_MIN);
    }

    // [crispy] intercepts overflow guard
    crispy->crosshair &= ~CROSSHAIR_INTERCEPT;
}


//
// USE LINES
//
mobj_t *usething;

bool PTR_UseTraverse(intercept_t *in)
{
    int side;

    if (!in->d.line->special)
    {
        P_LineOpening(in->d.line);
        if (g_p_local_globals->openrange <= 0)
        {
            S_StartSound(usething, sfx_noway);

            // can't use through a wall
            return false;
        }
        // not a special line, but keep checking
        return true;
    }

    side = 0;
    if (P_PointOnLineSide(usething->x, usething->y, in->d.line) == 1)
        side = 1;

    //	return false;		// don't use back side

    P_UseSpecialLine(usething, in->d.line, side);

    // can't use for than one special line in a row
    return false;
}


//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines(player_t *player)
{
    int     angle;
    fixed_t x1;
    fixed_t y1;
    fixed_t x2;
    fixed_t y2;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];

    P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse);
}


//
// RADIUS ATTACK
//
mobj_t *bombsource;
mobj_t *bombspot;
int     bombdamage;


//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
bool PIT_RadiusAttack(mobj_t *thing)
{
    fixed_t dx;
    fixed_t dy;
    fixed_t dist;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    // Boss spider and cyborg
    // take no damage from concussion.
    if (thing->type == MT_CYBORG
        || thing->type == MT_SPIDER)
        return true;

    dx = std::abs(thing->x - bombspot->x);
    dy = std::abs(thing->y - bombspot->y);

    dist = dx > dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;

    if (dist < 0)
        dist = 0;

    if (dist >= bombdamage)
        return true; // out of range

    if (P_CheckSight(thing, bombspot))
    {
        // must be in direct path
        P_DamageMobj(thing, bombspot, bombsource, bombdamage - dist);
    }

    return true;
}


//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack(mobj_t *spot,
    mobj_t *                source,
    int                     damage)
{
    int x;
    int y;

    int xl;
    int xh;
    int yl;
    int yh;

    fixed_t dist;

    dist       = (damage + MAXRADIUS) << FRACBITS;
    yh         = (spot->y + dist - g_p_local_globals->bmaporgy) >> MAPBLOCKSHIFT;
    yl         = (spot->y - dist - g_p_local_globals->bmaporgy) >> MAPBLOCKSHIFT;
    xh         = (spot->x + dist - g_p_local_globals->bmaporgx) >> MAPBLOCKSHIFT;
    xl         = (spot->x - dist - g_p_local_globals->bmaporgx) >> MAPBLOCKSHIFT;
    bombspot   = spot;
    bombsource = source;
    bombdamage = damage;

    for (y = yl; y <= yh; y++)
        for (x = xl; x <= xh; x++)
            P_BlockThingsIterator(x, y, PIT_RadiusAttack);
}


//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//
bool crushchange;
bool nofit;


//
// PIT_ChangeSector
//
bool PIT_ChangeSector(mobj_t *thing)
{
    mobj_t *mo;

    if (P_ThingHeightClip(thing))
    {
        // keep checking
        return true;
    }


    // crunch bodies to giblets
    if (thing->health <= 0)
    {
        // [crispy] no blood, no giblets
        // S_GIBS should be a "safe" state, and so is S_NULL
        // TODO: Add a check for DEHACKED states
        P_SetMobjState(thing, (thing->flags & MF_NOBLOOD) ? S_NULL : S_GIBS);

        if (g_doomstat_globals->gameversion > exe_doom_1_2)
            thing->flags &= ~MF_SOLID;
        thing->height = 0;
        thing->radius = 0;

        // [crispy] connect giblet object with the crushed monster
        thing->target = thing;

        // keep checking
        return true;
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj(thing);

        // keep checking
        return true;
    }

    if (!(thing->flags & MF_SHOOTABLE))
    {
        // assume it is bloody gibs or something
        return true;
    }

    nofit = true;

    if (crushchange && !(leveltime & 3))
    {
        P_DamageMobj(thing, nullptr, nullptr, 10);

        // spray blood in a random direction
        mo = P_SpawnMobj(thing->x,
            thing->y,
            // [crispy] Lost Souls and Barrels bleed Puffs
            thing->z + thing->height / 2, (thing->flags & MF_NOBLOOD) ? MT_PUFF : MT_BLOOD);

        mo->momx = P_SubRandom() << 12;
        mo->momy = P_SubRandom() << 12;

        // [crispy] connect blood object with the monster that bleeds it
        mo->target = thing;

        // [crispy] Spectres bleed spectre blood
        if (crispy->coloredblood)
            mo->flags |= (thing->flags & MF_SHADOW);
    }

    // keep checking (crush other things)
    return true;
}


//
// P_ChangeSector
//
bool
    P_ChangeSector(sector_t *sector,
        bool              crunch)
{
    int x;
    int y;

    nofit       = false;
    crushchange = crunch;

    // re-check heights for all things near the moving sector
    for (x = sector->blockbox[BOXLEFT]; x <= sector->blockbox[BOXRIGHT]; x++)
        for (y = sector->blockbox[BOXBOTTOM]; y <= sector->blockbox[BOXTOP]; y++)
            P_BlockThingsIterator(x, y, PIT_ChangeSector);


    return nofit;
}

// Code to emulate the behavior of Vanilla Doom when encountering an overrun
// of the spechit array.  This is by Andrey Budko (e6y) and comes from his
// PrBoom plus port.  A big thanks to Andrey for this.

static void SpechitOverrun(line_t *ld)
{
    static unsigned int baseaddr = 0;

    if (baseaddr == 0)
    {
        int p;

        // This is the first time we have had an overrun.  Work out
        // what base address we are going to use.
        // Allow a spechit value to be specified on the command line.

        //!
        // @category compat
        // @arg <n>
        //
        // Use the specified magic value when emulating spechit overruns.
        //

        p = M_CheckParmWithArgs("-spechit", 1);

        if (p > 0)
        {
            M_StrToInt(myargv[p + 1], reinterpret_cast<int *>(&baseaddr));
        }
        else
        {
            baseaddr = DEFAULT_SPECHIT_MAGIC;
        }
    }

    // Calculate address used in doom2.exe

    unsigned int addr = static_cast<unsigned int>(baseaddr + (ld - g_r_state_globals->lines) * 0x3E);

    switch (g_p_local_globals->numspechit)
    {
    case 9:
    case 10:
    case 11:
    case 12:
        tmbbox[g_p_local_globals->numspechit - 9] = static_cast<fixed_t>(addr);
        break;
    case 13:
        crushchange = addr;
        break;
    case 14:
        nofit = addr;
        break;
    default:
        fprintf(stderr, "SpechitOverrun: Warning: unable to emulate"
                        "an overrun where numspechit=%i\n",
            g_p_local_globals->numspechit);
        break;
    }
}
