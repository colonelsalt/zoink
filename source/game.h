#pragma once

#include "memory.h"
#include "inventory.h"

enum game_state_flags : u32
{
	GameState_TitleScreen		  = 1,
	GameState_IntroSequence       = 1 << 1,
	GameState_PendingWarpTile     = 1 << 2,
	GameState_PendingManualWarp   = 1 << 3,
	GameState_PendingScriptTile   = 1 << 4,
	GameState_ScriptRunning       = 1 << 5,
	GameState_InBattle			  = 1 << 6,
	GameState_AlphaBlendingIn 	  = 1 << 7,
	GameState_AlphaBlendingOut    = 1 << 8,
	GameState_MusicFadingOut 	  = 1 << 9,
	GameState_EndSequence		  = 1 << 10,
	GameState_EndScreen			  = 1 << 11,

	GameState_PendingWarp   = GameState_PendingWarpTile | GameState_PendingManualWarp,
	GameState_AlphaBlending = GameState_AlphaBlendingIn | GameState_AlphaBlendingOut
};

enum open_menu_flags : u32
{
	Menu_Inventory        = 1,
	Menu_PlayerStatus	  = 1 << 1,
	Menu_ConfirmWindow    = 1 << 2,
	Menu_CommandWindow    = 1 << 3, // i.e. the 'FIGHT/PAK' window
	Menu_EnemyStatus	  = 1 << 4,
	Menu_VendingMachine   = 1 << 5,

	Menu_StartMenu		  = Menu_Inventory     | Menu_PlayerStatus,
	Menu_TwoChoiceWindow  = Menu_ConfirmWindow | Menu_CommandWindow,
	Menu_BattleMenu       = Menu_CommandWindow | Menu_PlayerStatus   | Menu_EnemyStatus,
};

enum progression_flags : u32
{
	ProgFlag_ClutTownRonnieVis        = 1,
	ProgFlag_ClutTownPotion           = 1 << 1,
	ProgFlag_ClutTownGoblinVis        = 1 << 2,
	ProgFlag_TollGatePowerPill        = 1 << 3,
	ProgFlag_CityOrcVis		          = 1 << 4,
							          
							          
	ProgFlag_GroofusVis		          = 1 << 5,
	ProgFlag_HelgaVis	              = 1 << 6,
	ProgFlag_EndHokeVis               = 1 << 7,
							          
	ProgFlag_OrcDefeated	          = 1 << 8,
							          
	ProgFlag_MumSpokenToOnce          = 1 << 9,
	ProgFlag_ForestPotion	          = 1 << 10,
								      
	ProgFlag_TollGateCheckpoint       = 1 << 11,
	ProgFlag_RonnieForestSpokenOnce   = 1 << 12,
	ProgFlag_TollGuardSpokenOnce	  = 1 << 13,
	ProgFlag_RonnieTollGateSpokenOnce = 1 << 14,

	ProgFlag_CityNpcVis				  = 1 << 15
};

enum progression_vars
{
	Var_ClutTownProgression,
	Var_TollGateProgression,
	Var_CityProgression,

	Vars_Count
};

struct warp_tile;
struct script_tile;

struct game_state
{
	u32 StateFlags;
	u32 OpenMenus;

	u32 PrevFrameMenus;
	//u32 PrevFrameState;

	union
	{
		const warp_tile* PendingWarpTile;
		struct
		{
			u8 PendingWarpLevel;
			iv2 PendingWarpLocation;
		} PendingManualWarp;
	};
	const script_tile* PendingScriptTile;

	inventory* Inventory;

	u32 ProgressionFlags;
	u8 ProgressionVars[Vars_Count];

	fixed AlphaBlendAmount; // 0-16
	u32 CurrentlyPlayingSong;
	s16 Volume; // 0-1024
};

struct entity;
struct render_data
{
	entity* Player;

	b32 ReadyToDraw;

	b8 Bg1Only; // piggy piggy
};

static game_state* g_GameState;

inline static void SetFlag(u32 Flag)
{
	Assert(Flag); // If we're setting a flag of zero, something's not right
	g_GameState->ProgressionFlags |= Flag;
}

inline static void InitGameState(ewram_arena* Arena)
{
	g_GameState = PushStructZeroed(Arena, game_state);
	g_GameState->Inventory = InitInventory(Arena);

	// Set progression flags that should be set at the start
	SetFlag(ProgFlag_ClutTownRonnieVis);
	SetFlag(ProgFlag_GroofusVis);
	SetFlag(ProgFlag_HelgaVis);
	SetFlag(ProgFlag_EndHokeVis);
	SetFlag(ProgFlag_ClutTownGoblinVis);
}

inline static void PlaySong(u32 SongId, b32 ForceRestart = false)
{
	if (g_GameState->CurrentlyPlayingSong != SongId || ForceRestart)
	{
		mmStart(SongId, MM_PLAY_LOOP);
		g_GameState->CurrentlyPlayingSong = SongId;
	}
	g_GameState->StateFlags &= ~GameState_MusicFadingOut;
	g_GameState->Volume = 1024;
}

inline static void BeginSongFadeOut()
{
	g_GameState->StateFlags |= GameState_MusicFadingOut;
}

inline static void UpdateSongFade()
{
	static constexpr s16 FADE_SPEED = 100;
	if (g_GameState->StateFlags & GameState_MusicFadingOut)
	{
		g_GameState->Volume -= FADE_SPEED;
		if (g_GameState->Volume < 0)
		{
			g_GameState->Volume = 0;
		}
	}
	mmSetModuleVolume(g_GameState->Volume);
}

inline static b32 MenuFlipped(u32 MenuFlag)
{
	b32 Result = (g_GameState->OpenMenus & MenuFlag) ^ (g_GameState->PrevFrameMenus & MenuFlag);
	return Result;
}

inline static b32 MenuOpenedThisFrame(u32 StateFlag)
{
	b32 Result = MenuFlipped(StateFlag) && (g_GameState->StateFlags & StateFlag);
	return Result;
}

inline static b32 MenuClosedThisFrame(u32 StateFlag)
{
	b32 Result = MenuFlipped(StateFlag) && !(g_GameState->StateFlags & StateFlag);
	return Result;
}

inline static u8 GetVar(u32 Var)
{
	Assert(Var < Vars_Count);
	u8 Result = g_GameState->ProgressionVars[Var];
	return Result;
}

inline static void SetVar(u32 Var, u8 Value)
{
	Assert(Var < Vars_Count);
	g_GameState->ProgressionVars[Var] = Value;
}

inline static b32 GetFlag(u32 Flag) 
{
	b32 Result = g_GameState->ProgressionFlags & Flag;
	return Result;
}

inline static void ClearFlag(u32 Flag)
{
	g_GameState->ProgressionFlags &= ~Flag;
}