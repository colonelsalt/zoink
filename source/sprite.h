#pragma once

#include "oam.h"

enum sprite_size
{
	SpriteSize_8x8,
	SpriteSize_16x16,
	SpriteSize_16x32,
	SpriteSize_32x32,
	SpriteSize_32x64,
	SpriteSize_64x64
};

enum bit_depth
{
	BitDepth_4bpp
};

struct sprite
{
	sprite_size Size;
	bit_depth BitDepth;

	u32 IdleFrames[4]; // each entry is a frame index

	oam_entry* OamEntry;
	const u8* TilesBasePtr;
	const u16* PalettePtr;
	u32 PaletteBank;
	u32 FrameIndex;
	u32 VramTileIndex;
};

// TOOD: Maybe rename to 'ow_id'?
enum sprite_id
{
	SpriteId_None,

	SpriteId_Player,
	SpriteId_Hoke,
	SpriteId_Ronnie,
	SpriteId_Mum,
	SpriteId_OldDude,
	SpriteId_ForestGuard,
	SpriteId_SandboxGirl,
	SpriteId_Hobo,
	SpriteId_BikiniLady,
	SpriteId_Hobgoblin,
	SpriteId_MaleAid,
	SpriteId_FemaleAid,
	SpriteId_Policeman,
	SpriteId_Item,
	SpriteId_Orc,
	SpriteId_BigHoke,
	SpriteId_Groofus,
	SpriteId_Helga,
	SpriteId_KimonoLady,
	SpriteId_MustacheMan,
	SpriteId_ForestBoy,
	SpriteId_Adventurer,
	SpriteId_Traveller,
	SpriteId_FemaleTraveller,
	SpriteId_BusinessMan,

	SpriteId_Count
};