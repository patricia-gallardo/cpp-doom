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


// HEADER FILES ------------------------------------------------------------

#include <cstring>
#include "m_random.hpp"
#include "h2def.hpp"
#include "i_system.hpp"
#include "i_sound.hpp"
#include "s_sound.hpp"
#include "memory.hpp"

// MACROS ------------------------------------------------------------------

#define SS_MAX_SCRIPTS	64
#define SS_TEMPBUFFER_SIZE	1024
#define SS_SEQUENCE_NAME_LENGTH 32

#define SS_SCRIPT_NAME "SNDSEQ"
#define SS_STRING_PLAY			"play"
#define SS_STRING_PLAYUNTILDONE "playuntildone"
#define SS_STRING_PLAYTIME		"playtime"
#define SS_STRING_PLAYREPEAT	"playrepeat"
#define SS_STRING_DELAY			"delay"
#define SS_STRING_DELAYRAND		"delayrand"
#define SS_STRING_VOLUME		"volume"
#define SS_STRING_END			"end"
#define SS_STRING_STOPSOUND		"stopsound"

// TYPES -------------------------------------------------------------------

enum sscmds_t
{
    SS_CMD_NONE,
    SS_CMD_PLAY,
    SS_CMD_WAITUNTILDONE,       // used by PLAYUNTILDONE
    SS_CMD_PLAYTIME,
    SS_CMD_PLAYREPEAT,
    SS_CMD_DELAY,
    SS_CMD_DELAYRAND,
    SS_CMD_VOLUME,
    SS_CMD_STOPSOUND,
    SS_CMD_END
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void VerifySequencePtr(int *base, int *ptr);
static int GetSoundOffset(char *name);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern sfxinfo_t S_sfx[];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static struct
{
    char name[SS_SEQUENCE_NAME_LENGTH];
    int scriptNum;
    int stopSound;
} SequenceTranslate[SEQ_NUMSEQ] =
{
    {
    "Platform", 0, 0},
    {
    "Platform", 0, 0},          // a 'heavy' platform is just a platform
    {
    "PlatformMetal", 0, 0},
    {
    "Platform", 0, 0},          // same with a 'creak' platform
    {
    "Silence", 0, 0},
    {
    "Lava", 0, 0},
    {
    "Water", 0, 0},
    {
    "Ice", 0, 0},
    {
    "Earth", 0, 0},
    {
    "PlatformMetal2", 0, 0},
    {
    "DoorNormal", 0, 0},
    {
    "DoorHeavy", 0, 0},
    {
    "DoorMetal", 0, 0},
    {
    "DoorCreak", 0, 0},
    {
    "Silence", 0, 0},
    {
    "Lava", 0, 0},
    {
    "Water", 0, 0},
    {
    "Ice", 0, 0},
    {
    "Earth", 0, 0},
    {
    "DoorMetal2", 0, 0},
    {
    "Wind", 0, 0}
};

static int *SequenceData[SS_MAX_SCRIPTS];

int ActiveSequences;
seqnode_t *SequenceListHead;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// VerifySequencePtr
//
//   Verifies the integrity of the temporary ptr, and ensures that the ptr
//              isn't exceeding the size of the temporary buffer
//==========================================================================

static void VerifySequencePtr(int *base, int *ptr)
{
    if (ptr - base > SS_TEMPBUFFER_SIZE)
    {
        I_Error("VerifySequencePtr:  tempPtr >= %d\n", SS_TEMPBUFFER_SIZE);
    }
}

//==========================================================================
//
// GetSoundOffset
//
//==========================================================================

static int GetSoundOffset(char *name)
{
    int i;

    for (i = 0; i < NUMSFX; i++)
    {
        if (!strcasecmp(name, S_sfx[i].tagname))
        {
            return i;
        }
    }
    SC_ScriptError("GetSoundOffset:  Unknown sound name\n");
    return 0;
}

//==========================================================================
//
// SN_InitSequenceScript
//
//==========================================================================

void SN_InitSequenceScript()
{
    int i, j;
    int inSequence;
    int *tempDataStart = nullptr;
    int *tempDataPtr = nullptr;

    inSequence = -1;
    ActiveSequences = 0;
    for (i = 0; i < SS_MAX_SCRIPTS; i++)
    {
        SequenceData[i] = nullptr;
    }
    SC_Open(SS_SCRIPT_NAME);
    while (SC_GetString())
    {
        if (*sc_String == ':')
        {
            if (inSequence != -1)
            {
                SC_ScriptError("SN_InitSequenceScript:  Nested Script Error");
            }
            tempDataStart = zmalloc<int *>(SS_TEMPBUFFER_SIZE,
                                             PU_STATIC, nullptr);
            std::memset(tempDataStart, 0, SS_TEMPBUFFER_SIZE);
            tempDataPtr = tempDataStart;
            for (i = 0; i < SS_MAX_SCRIPTS; i++)
            {
                if (SequenceData[i] == nullptr)
                {
                    break;
                }
            }
            if (i == SS_MAX_SCRIPTS)
            {
                I_Error("Number of SS Scripts >= SS_MAX_SCRIPTS");
            }
            for (j = 0; j < SEQ_NUMSEQ; j++)
            {
                if (!strcasecmp(SequenceTranslate[j].name, sc_String + 1))
                {
                    SequenceTranslate[j].scriptNum = i;
                    inSequence = j;
                    break;
                }
            }
            continue;           // parse the next command
        }
        if (inSequence == -1)
        {
            continue;
        }
        if (SC_Compare(SS_STRING_PLAYUNTILDONE))
        {
            VerifySequencePtr(tempDataStart, tempDataPtr);
            SC_MustGetString();
            *tempDataPtr++ = SS_CMD_PLAY;
            *tempDataPtr++ = GetSoundOffset(sc_String);
            *tempDataPtr++ = SS_CMD_WAITUNTILDONE;
        }
        else if (SC_Compare(SS_STRING_PLAY))
        {
            VerifySequencePtr(tempDataStart, tempDataPtr);
            SC_MustGetString();
            *tempDataPtr++ = SS_CMD_PLAY;
            *tempDataPtr++ = GetSoundOffset(sc_String);
        }
        else if (SC_Compare(SS_STRING_PLAYTIME))
        {
            VerifySequencePtr(tempDataStart, tempDataPtr);
            SC_MustGetString();
            *tempDataPtr++ = SS_CMD_PLAY;
            *tempDataPtr++ = GetSoundOffset(sc_String);
            SC_MustGetNumber();
            *tempDataPtr++ = SS_CMD_DELAY;
            *tempDataPtr++ = sc_Number;
        }
        else if (SC_Compare(SS_STRING_PLAYREPEAT))
        {
            VerifySequencePtr(tempDataStart, tempDataPtr);
            SC_MustGetString();
            *tempDataPtr++ = SS_CMD_PLAYREPEAT;
            *tempDataPtr++ = GetSoundOffset(sc_String);
        }
        else if (SC_Compare(SS_STRING_DELAY))
        {
            VerifySequencePtr(tempDataStart, tempDataPtr);
            *tempDataPtr++ = SS_CMD_DELAY;
            SC_MustGetNumber();
            *tempDataPtr++ = sc_Number;
        }
        else if (SC_Compare(SS_STRING_DELAYRAND))
        {
            VerifySequencePtr(tempDataStart, tempDataPtr);
            *tempDataPtr++ = SS_CMD_DELAYRAND;
            SC_MustGetNumber();
            *tempDataPtr++ = sc_Number;
            SC_MustGetNumber();
            *tempDataPtr++ = sc_Number;
        }
        else if (SC_Compare(SS_STRING_VOLUME))
        {
            VerifySequencePtr(tempDataStart, tempDataPtr);
            *tempDataPtr++ = SS_CMD_VOLUME;
            SC_MustGetNumber();
            *tempDataPtr++ = sc_Number;
        }
        else if (SC_Compare(SS_STRING_END))
        {
            *tempDataPtr++ = SS_CMD_END;
            int dataSize = static_cast<int>((tempDataPtr - tempDataStart) * sizeof(int));
            SequenceData[i] = zmalloc<int *>(dataSize, PU_STATIC, nullptr);
            std::memcpy(SequenceData[i], tempDataStart, dataSize);
            Z_Free(tempDataStart);
            inSequence = -1;
        }
        else if (SC_Compare(SS_STRING_STOPSOUND))
        {
            SC_MustGetString();
            SequenceTranslate[inSequence].stopSound =
                GetSoundOffset(sc_String);
            *tempDataPtr++ = SS_CMD_STOPSOUND;
        }
        else
        {
            SC_ScriptError("SN_InitSequenceScript:  Unknown commmand.\n");
        }
    }
}

//==========================================================================
//
//  SN_StartSequence
//
//==========================================================================

void SN_StartSequence(mobj_t * mobj, int sequence)
{
    seqnode_t *node;

    SN_StopSequence(mobj);      // Stop any previous sequence
    node = zmalloc<seqnode_t *>(sizeof(seqnode_t), PU_STATIC, nullptr);
    node->sequencePtr = SequenceData[SequenceTranslate[sequence].scriptNum];
    node->sequence = sequence;
    node->mobj = mobj;
    node->delayTics = 0;
    node->stopSound = SequenceTranslate[sequence].stopSound;
    node->volume = 127;         // Start at max volume

    if (!SequenceListHead)
    {
        SequenceListHead = node;
        node->next = node->prev = nullptr;
    }
    else
    {
        SequenceListHead->prev = node;
        node->next = SequenceListHead;
        node->prev = nullptr;
        SequenceListHead = node;
    }
    ActiveSequences++;
    return;
}

//==========================================================================
//
//  SN_StartSequenceName
//
//==========================================================================

void SN_StartSequenceName(mobj_t * mobj, char *name)
{
    int i;

    for (i = 0; i < SEQ_NUMSEQ; i++)
    {
        if (!strcmp(name, SequenceTranslate[i].name))
        {
            SN_StartSequence(mobj, i);
            return;
        }
    }
}

//==========================================================================
//
//  SN_StopSequence
//
//==========================================================================

void SN_StopSequence(mobj_t * mobj)
{
    seqnode_t *node;

    for (node = SequenceListHead; node; node = node->next)
    {
        if (node->mobj == mobj)
        {
            S_StopSound(mobj);
            if (node->stopSound)
            {
                S_StartSoundAtVolume(mobj, node->stopSound, node->volume);
            }
            if (SequenceListHead == node)
            {
                SequenceListHead = node->next;
            }
            if (node->prev)
            {
                node->prev->next = node->next;
            }
            if (node->next)
            {
                node->next->prev = node->prev;
            }
            Z_Free(node);
            ActiveSequences--;
        }
    }
}

//==========================================================================
//
//  SN_UpdateActiveSequences
//
//==========================================================================

void SN_UpdateActiveSequences()
{
    seqnode_t *node;
    bool sndPlaying;

    if (!ActiveSequences || paused)
    {                           // No sequences currently playing/game is paused
        return;
    }
    for (node = SequenceListHead; node; node = node->next)
    {
        if (node->delayTics)
        {
            node->delayTics--;
            continue;
        }
        sndPlaying = S_GetSoundPlayingInfo(node->mobj, node->currentSoundID);
        switch (*node->sequencePtr)
        {
            case SS_CMD_PLAY:
                if (!sndPlaying)
                {
                    node->currentSoundID = *(node->sequencePtr + 1);
                    S_StartSoundAtVolume(node->mobj, node->currentSoundID,
                                         node->volume);
                }
                node->sequencePtr += 2;
                break;
            case SS_CMD_WAITUNTILDONE:
                if (!sndPlaying)
                {
                    node->sequencePtr++;
                    node->currentSoundID = 0;
                }
                break;
            case SS_CMD_PLAYREPEAT:
                if (!sndPlaying)
                {
                    node->currentSoundID = *(node->sequencePtr + 1);
                    S_StartSoundAtVolume(node->mobj, node->currentSoundID,
                                         node->volume);
                }
                break;
            case SS_CMD_DELAY:
                node->delayTics = *(node->sequencePtr + 1);
                node->sequencePtr += 2;
                node->currentSoundID = 0;
                break;
            case SS_CMD_DELAYRAND:
                node->delayTics = *(node->sequencePtr + 1) +
                    M_Random() % (*(node->sequencePtr + 2) -
                                  *(node->sequencePtr + 1));
                node->sequencePtr += 2;
                node->currentSoundID = 0;
                break;
            case SS_CMD_VOLUME:
                node->volume = (127 * (*(node->sequencePtr + 1))) / 100;
                node->sequencePtr += 2;
                break;
            case SS_CMD_STOPSOUND:
                // Wait until something else stops the sequence
                break;
            case SS_CMD_END:
                SN_StopSequence(node->mobj);
                break;
            default:
                break;
        }
    }
}

//==========================================================================
//
//  SN_StopAllSequences
//
//==========================================================================

void SN_StopAllSequences()
{
    seqnode_t *node;

    for (node = SequenceListHead; node; node = node->next)
    {
        node->stopSound = 0;    // don't play any stop sounds
        SN_StopSequence(node->mobj);
    }
}

//==========================================================================
//
//  SN_GetSequenceOffset
//
//==========================================================================

int SN_GetSequenceOffset(int sequence, int *sequencePtr)
{
    return static_cast<int>(sequencePtr -
            SequenceData[SequenceTranslate[sequence].scriptNum]);
}

//==========================================================================
//
//  SN_ChangeNodeData
//
//      nodeNum zero is the first node
//==========================================================================

void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume,
                       int currentSoundID)
{
    int i;
    seqnode_t *node;

    i = 0;
    node = SequenceListHead;
    while (node && i < nodeNum)
    {
        node = node->next;
        i++;
    }
    if (!node)
    {                           // reach the end of the list before finding the nodeNum-th node
        return;
    }
    node->delayTics = delayTics;
    node->volume = volume;
    node->sequencePtr += seqOffset;
    node->currentSoundID = currentSoundID;
}
