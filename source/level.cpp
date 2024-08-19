#include "level.h"

#include <tonc.h>


static bigmap_copy_spec GetTargetTiles(map_layer* Map, iv2 SrcTilePos)
{
	bigmap_copy_spec Result = {};

	u32 SrcIndex = SrcTilePos.Y * Map->TileDimensions.Width + SrcTilePos.X;
	Assert(SrcIndex < Map->MapSize);
	Result.SourceEntry = Map->Src + SrcIndex;

	Result.DestTilePos = iv2(SrcTilePos.X & 31, SrcTilePos.Y & 31); // aka mod 32
	u32 DestIndex = Result.DestTilePos.Y * 32 + Result.DestTilePos.X;
	Assert(DestIndex < 32 * 32);
	Result.DestEntry = Map->Dest + DestIndex;

	return Result;
}

static void AppendMapRow(map_layer* Map, iv2 SrcTilePos)
{
	bigmap_copy_spec Spec = GetTargetTiles(Map, SrcTilePos);

	for (s32 X = Spec.DestTilePos.X; X < 32; X++)
	{
		*Spec.DestEntry++ = *Spec.SourceEntry++;
	}

	Spec.DestEntry -= 32;

	for (s32 X = 0; X < Spec.DestTilePos.X; X++)
	{
		*Spec.DestEntry++ = *Spec.SourceEntry++;
	}
}

static void AppendMapColumn(map_layer* Map, iv2 SrcTilePos)
{
	bigmap_copy_spec Spec = GetTargetTiles(Map, SrcTilePos);
	
	for (s32 Y = Spec.DestTilePos.Y; Y < 32; Y++)
	{
		*Spec.DestEntry = *Spec.SourceEntry;

		Spec.DestEntry += 32; // DestTilePos.Y++
		Spec.SourceEntry += Map->TileDimensions.Width; // SrcTilePos.Y++
	}

	Spec.DestEntry -= 32 * 32; // DestTilePos.Y = 0

	for (s32 Y = 0; Y < Spec.DestTilePos.Y; Y++)
	{
		*Spec.DestEntry = *Spec.SourceEntry;

		Spec.DestEntry += 32;
		Spec.SourceEntry += Map->TileDimensions.Width;
	}
}

// i.e. tile coord of camera last frame
static iv2 s_PrevCameraTilePos;

static void InitBigMap()
{
	iv2 CameraTilePos = iv2(g_Camera.WorldPos) >> 3;
	for (u32 i = 0; i < g_Level->NumLayers; i++)
	{
		map_layer* Map = g_Level->Layers + i;
		iv2 CamPos = CameraTilePos;
		for (u32 Y = 0; Y < 32; Y++)
		{
			AppendMapRow(Map, CamPos);
			CamPos.Y++;
		}
	}
	s_PrevCameraTilePos = CameraTilePos;
}

static void UpdateBigMap()
{
	// TODO: Should this actually round up from every 0.5?
	iv2 CameraTilePos = iv2(g_Camera.WorldPos) >> 3;
	
	for (u32 i = 0; i < g_Level->NumLayers; i++)
	{
		map_layer* Map = g_Level->Layers + i;
		Assert(Map->Type == MapType_Bigmap);

		if (CameraTilePos.X < s_PrevCameraTilePos.X)
		{
			AppendMapColumn(Map, CameraTilePos); // append from left edge
		}
		else if (CameraTilePos.X > s_PrevCameraTilePos.X)
		{
			iv2 NewColumnTilePos = iv2(CameraTilePos.X + 31, CameraTilePos.Y);
			AppendMapColumn(Map, NewColumnTilePos); // append from right edge
		}

		if (CameraTilePos.Y < s_PrevCameraTilePos.Y)
		{
			AppendMapRow(Map, CameraTilePos); // append from top
		}
		else if (CameraTilePos.Y > s_PrevCameraTilePos.Y)
		{
			iv2 NewRowTilePos = iv2(CameraTilePos.X, CameraTilePos.Y + 31);
			AppendMapRow(Map, NewRowTilePos); // append from bottom
		}
	}
	s_PrevCameraTilePos = CameraTilePos;
}

static void UpdateLevel()
{
	for (u32 i = 0; i < g_Level->NumEntities; i++)
	{
		UpdateEntity(g_Level->Entities + i);
	}

}

static void UnloadLevel();
static void LoadLevel(level_name LevelName, ewram_arena* Arena);
static sprite* LoadOwSprite(sprite_id SpriteId, v2* WorldPos, sprite_size Size, ewram_arena* Arena);
static void UpdateCamera(entity* Player);


static void OnLevelLoad()
{
	switch (g_Level->Name)
	{
		case Level_JacksHouse1F:
		{
			PlaySong(MOD_COZY_FUZZYFARMS);
		} break;

		case Level_JacksHouseGF:
		{
			PlaySong(MOD_COZY_FUZZYFARMS);
		} break;

		case Level_ClutTown:
		{
			// TODO TEMP
			//SetVar(Var_ClutTownProgression, 2);
			//GiveItem(Item_Sword, 1);

			PlaySong(MOD_COZY_SUMMERTOWN);
			SetFlag(ProgFlag_ClutTownGoblinVis);
		} break;

		case Level_HokesLab:
		{
			PlaySong(MOD_COZY_FUZZYFARMS);
		} break;

		case Level_ForbiddenForest:
		{
			PlaySong(MOD_RPG_MAP);
		} break;

		case Level_TollGate:
		{
			PlaySong(MOD_COZY_FUZZYFARMS);
			ClearFlag(ProgFlag_TollGatePowerPill);
			SetFlag(ProgFlag_TollGateCheckpoint);
		} break;

		case Level_City:
		{
			PlaySong(MOD_MYSTERY_FOG);
			SetFlag(ProgFlag_CityOrcVis);
		} break;

		case Level_End:
		default:
		{
			Assert(false);
		} break;
	}
}

static void Warp(entity* Player)
{
	ewram_arena* MapArena = g_Level->Arena;
	UnloadLevel();

	InitObjVram(MapArena);
	InitOam(MapArena);
	InitSpritePalettes(MapArena);

	v2 TargetLocation;
	if (g_GameState->StateFlags & GameState_PendingWarpTile)
	{
		LoadLevel(g_GameState->PendingWarpTile->DestMap, MapArena);
		Assert(g_GameState->PendingWarpTile->DestWarp < g_Level->NumWarps);

		const warp_tile* DestWarp = g_Level->Warps + g_GameState->PendingWarpTile->DestWarp;
		Player->WorldPos = v2 { DestWarp->WorldPos.X + g_GameState->PendingWarpTile->NudgeX, DestWarp->WorldPos.Y - 24 + g_GameState->PendingWarpTile->NudgeY };
	}
	else
	{
		Assert(g_GameState->StateFlags & GameState_PendingManualWarp);
		LoadLevel((level_name)g_GameState->PendingManualWarp.PendingWarpLevel, MapArena);

		Player->WorldPos = v2 { g_GameState->PendingManualWarp.PendingWarpLocation.X, g_GameState->PendingManualWarp.PendingWarpLocation.Y };
	}

	if (g_Level->IsBigMap)
	{
		g_Camera.State = CamState_FollowPlayer;
		UpdateCamera(Player);
		InitBigMap();
	}

	Player->Sprite = LoadOwSprite(SpriteId_Player, &Player->WorldPos, SpriteSize_16x32, MapArena);

	REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | g_Level->BgFlags;
}

static void SpawnPlayerReflection(entity* Player)
{
	sprite* RefSprite = g_Level->PlayerReflection->Sprite;

	*g_Level->PlayerReflection = *Player;
	g_Level->PlayerReflection->Type = Entity_Reflection;
	g_Level->PlayerReflection->Sprite = RefSprite;
	
	g_Level->PlayerReflection->WorldPos.Y += Player->Height + 4;

	BeginAlphaFade(g_Level->PlayerReflection->Sprite);
	obj_unhide(&g_Level->PlayerReflection->Sprite->OamEntry->Attributes, 0);
	g_GameState->AlphaBlendAmount = 10;
}