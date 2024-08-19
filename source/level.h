#pragma once

struct bigmap_copy_spec
{
	const SCR_ENTRY* SourceEntry; // First tile we're copying from
	SCR_ENTRY* DestEntry; // First tile we're copying to

	iv2 DestTilePos; // Tile coordinates of first destination tile
};

enum level_name : u8
{
	Level_JacksHouse1F,
	Level_JacksHouseGF,
	Level_ClutTown,
	Level_HokesLab,
	Level_ForbiddenForest,
	Level_TollGate,
	Level_City,
	Level_End
};

enum tile_collision : u8
{
	TileCol_Walkable,
	TileCol_Blocked,
	TileCol_TopOnly, // not used atm lol
	TileCol_WaterAdjacent,
	TileCol_ReflectiveWater
};

enum map_type
{
	MapType_32x32,
	MapType_Bigmap
};

struct warp_tile;
struct script_tile;
struct level_spec;

struct grit_pic
{
	const u8* Tiles;
	u32 TilesSize; // bytes

	const u16* Palette;
	u32 PaletteSize; // bytes
};

struct level_tiles_mapping
{
	const level_spec* LevelSpec;
	const grit_pic* Pic;
	b32 IsBigMap;
};

// compile-time known data for one level
struct level_spec
{
	u32 TileWidth;
	u32 TileHeight;

	u32 NumMapLayers;
	const u16** Layers;

	const tile_collision* CollisionMap;

	u32 NumCharacters;
	const level_char* Characters;

	u32 NumWarps;
	const warp_tile* Warps;

	u32 NumScripts;
	const script_tile* ScriptTiles;
};

struct map_layer
{
	map_type Type;

	const SCR_ENTRY* Src;
	u32 MapSize; // bytes TODO: shouldn't this be a property of `level` instead?

	iv2 TileDimensions; // of the source map, obvs.

	SCR_ENTRY* Dest; // 32x32t square in VRAM if this is a bigmap

	u32 ScreenBaseBlock;

	u16 BgNo;
	u16 BgControl;
};

struct script_tile
{
	iv2 WorldPos;
	script_func Script;
	u32 TriggerVar;
	u8 TriggerValue;
};

struct warp_tile
{
	iv2 WorldPos;
	level_name DestMap;
	u32 DestWarp;
	direction Direction; // i.e. the direction of travel (up/down/forward/back/right/left),
	s32 NudgeX;
	s32 NudgeY;
};

// runtime level
struct level
{
	level_name Name;

	u16 TileWidth;
	u16 TileHeight;
	b32 IsBigMap;
	b32 IsCameraLocked;

	const u8* Tiles;
	const tile_collision* CollisionMap;
	u32 TilesSize; // bytes

	map_layer* Layers;
	u32 NumLayers;

	u16 BgFlags; // Which BGs are enabled: DCNT_BG0 | DCNT_BG1, etc.

	entity* Entities;
	u32 NumEntities;

	entity* PlayerReflection;

	const warp_tile* Warps;
	u32 NumWarps;

	const script_tile* ScriptTiles;
	u32 NumScriptTiles;

	ewram_arena* Arena; // this arena's lifetime is the same as the level's (reset when unloaded)
};

inline v2 PixelToTilePos(v2 PixelPos)
{
	v2 Result;
	Result.X = PixelPos.X >> 3;
	Result.Y = PixelPos.Y >> 3;
	return Result;
}

inline iv2 TileDimensionsFromBgFlags(u16 BgCnt)
{
	u16 Size = BgCnt & BG_SIZE_MASK;
	iv2 Result;
	switch (Size)
	{
		case BG_REG_32x32:
		{
			Result = iv2(32, 32);
		} break;
		case BG_REG_64x32:
		{
			Result = iv2(64, 32);
		} break;
		case BG_REG_32x64:
		{
			Result = iv2(32, 64);
		} break;
		case BG_REG_64x64:
		{
			Result = iv2(64, 64);
		} break;
		default:
		{
			Assert(false);
		} break;
	}
	return Result;
}

inline constexpr u32 GetBgBit(u32 BgNo)
{
	switch (BgNo)
	{
		case 0:
			return DCNT_BG0;
		case 1:
			return DCNT_BG1;
		case 2:
			return DCNT_BG2;
		case 3:
			return DCNT_BG3;
	}
	Assert(false);
	return 0;
}

static level* g_Level;
