//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//  MapObj data. Map Objects or mobjs are actors, entities,
//  thinker, take-your-pick... anything that moves, acts, or
//  suffers state changes of more or less violent nature.
//


#ifndef __D_THINK__
#define __D_THINK__


//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//

struct mobj_t;
struct player_t;
struct pspdef_t;

using actionf_v  = void (*)();
using actionf_p1 = void (*)(mobj_t *);
using actionf_p2 = void (*)(player_t *, pspdef_t *);
using actionf_p3 = void (*)(mobj_t *, player_t *, pspdef_t *); // [crispy] let pspr action pointers get called from mobj states

union actionf_t {
    actionf_v  acv;
    actionf_p1 acp1;
    actionf_p2 acp2;
    actionf_p3 acp3; // [crispy] let pspr action pointers get called from mobj states

    actionf_t()
        : acv { nullptr }
    {
    }
    actionf_t(actionf_v f)
        : acv { f }
    {
    }
    actionf_t(actionf_p1 f)
        : acp1 { f }
    {
    }
    actionf_t(actionf_p2 f)
        : acp2 { f }
    {
    }
    actionf_t(actionf_p3 f)
        : acp3 { f }
    {
    }
};


// Historically, "think_t" is yet another
//  function pointer to a routine to handle
//  an actor.
using think_t = actionf_t;


// Doubly linked list of actors.
using thinker_t = struct thinker_s {
    struct thinker_s *prev;
    struct thinker_s *next;
    think_t           function;
};


#endif
