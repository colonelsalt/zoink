#pragma once

#include <tonc.h>

#include "memory.h"

// If all sprites have unique palettes this is max we can do for now
constexpr u32 MAX_OAM_EMTRIES = 16;


struct oam_entry
{
	OBJ_ATTR Attributes;
	s32 Y;
	oam_entry* NextHighestY;
};

struct shadow_oam
{
	oam_entry* Head; // Linked list sorted in descending order of world pos. Y

	// TODO: Make this dynamically allocated/freed with an arena?
	oam_entry Entries[MAX_OAM_EMTRIES];
	u32 NumLiveEntries;
};

static shadow_oam* s_ShadowOam;

static void InitOam(ewram_arena* Arena)
{
	for (u32 i = 0; i < 128; i++)
	{
		oam_mem[i].attr0 = ATTR0_HIDE;
	}
	s_ShadowOam = PushStructZeroed(Arena, shadow_oam);
}

static void InsertEntry(oam_entry* Entry)
{
	oam_entry* Prev = nullptr;
	oam_entry* Current = s_ShadowOam->Head;
	while (Current && Entry->Y < Current->Y)
	{
		Prev = Current;
		Current = Current->NextHighestY;
	}

	if (Prev)
	{
		Prev->NextHighestY = Entry;
		if (Current)
		{
			Entry->NextHighestY = Current;
		}
		else
		{
			Entry->NextHighestY = nullptr;
		}
	}
	else
	{
		s_ShadowOam->Head = Entry;
		Entry->NextHighestY = Current;
	}
}

static void RemoveEntry(oam_entry* Entry)
{
	oam_entry* Prev = nullptr;
	oam_entry* Current = s_ShadowOam->Head;
	while (Current && Current != Entry)
	{
		Prev = Current;
		Current = Current->NextHighestY;
	}
	Assert(Current); // Trying to free an entry that's not in the live linked list
	if (Prev)
	{
		Prev->NextHighestY = Entry->NextHighestY;
	}
	else
	{
		// The entry we're removing is the one with the highest Y
		s_ShadowOam->Head = Entry->NextHighestY;
	}
}


static void ChangeOamEntryY(oam_entry* Entry, s32 Y)
{
	if (Entry->Y == Y)
	{
		return;
	}
	Entry->Y = Y;

	RemoveEntry(Entry);
	InsertEntry(Entry);
}

// NOTE: Does not set the attributes
static oam_entry* AllocOamEntry(s32 Y)
{
	Assert(s_ShadowOam->NumLiveEntries < MAX_OAM_EMTRIES);
	oam_entry* Result = &s_ShadowOam->Entries[s_ShadowOam->NumLiveEntries++];

	Result->Y = Y;
	Result->NextHighestY = nullptr;
	Result->Attributes = {};
	
	InsertEntry(Result);

	return Result;
}

static void FreeOamEntry(oam_entry* Entry)
{
	RemoveEntry(Entry);
	s_ShadowOam->NumLiveEntries--;
}

inline void Render_Oam()
{
	oam_entry* Current = s_ShadowOam->Head;
	u32 i = 0;
	while (Current)
	{
		oam_copy(oam_mem + i, &Current->Attributes, 1);
		i++;
		Current = Current->NextHighestY;
	}
	Assert(i == s_ShadowOam->NumLiveEntries);
}