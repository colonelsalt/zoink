#pragma once
#include <tonc.h>
#include "util.h"

enum block_alloc_state : u8
{
	BlockAlloc_Unused,
	BlockAlloc_Used,
	BlockAlloc_Continue
};

static u8* s_AllocMap; // allocated at runtime

static void InitObjVram(ewram_arena* Arena)
{
	s_AllocMap = PushSize(Arena, 1'024);
	memset32(s_AllocMap, 0, 1024 / 4);
}

// Returns a base tile index
static u32 AllocObjVram(u32 Size)
{
	Assert(!(Size & 31) && (Size <= 32 * 1'024)); // All valid sprite sizes are multiples of 32
	u32 NumBlocksNeeded = Size >> 5; // div 32

	u32 Result = 0;
	u32 NumBlocksFound = 0;
	for (u32 i = 0; i < 1'024; i += NumBlocksNeeded)
	{
		if (s_AllocMap[i] == BlockAlloc_Unused)
		{
			NumBlocksFound = 1;
			Result = i;
			for (u32 j = i + 1; j < i + NumBlocksNeeded; j++)
			{
				if (s_AllocMap[j] == BlockAlloc_Unused)
				{
					++NumBlocksFound;
				}
				else
				{
					break;
				}
			}
			if (NumBlocksFound == NumBlocksNeeded)
			{
				break;
			}
		}
	}
	Assert(NumBlocksFound == NumBlocksNeeded); // Oops out of VRAM

	s_AllocMap[Result] = BlockAlloc_Used;
	for (u32 i = Result + 1; i < Result + NumBlocksFound; i++)
	{
		s_AllocMap[i] = BlockAlloc_Continue;
	}

	return Result;
}

static void FreeObjVram(u32 BlockIndex)
{
	Assert(BlockIndex >= 0 && BlockIndex <= 1'024);

	Assert(s_AllocMap[BlockIndex] == BlockAlloc_Used);
	s_AllocMap[BlockIndex++] = BlockAlloc_Unused;
	while (s_AllocMap[BlockIndex] == BlockAlloc_Continue)
	{
		s_AllocMap[BlockIndex++] = BlockAlloc_Unused;
	}
}