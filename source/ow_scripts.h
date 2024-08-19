#pragma once

enum script_state
{
	ScriptState_Running,
	ScriptState_WaitingForText,
	ScriptState_WaitingForMovement,
	ScriptState_WaitingForBattle,
	ScriptState_WaitingForVendingMachine,
};

enum wait_movement_type
{
	WaitMovement_None,
	WaitMovement_ScriptEntity,
	WaitMovement_Player,
	WaitMovement_All,
};

enum script_source_type
{
	ScriptSource_Npc,
	ScriptSource_Tile,
	// ScriptSource_Level
};

struct script_source
{
	script_source_type Type;
	union
	{
		entity* Entity; // who we're interacting with
		const script_tile* Tile;
	};
};

struct script_context
{
	script_source Source;
	entity* Player;

	u32 Tick;
	script_state State;
	wait_movement_type WaitMoveState;

	script_func ActiveScript;

	ewram_arena* Arena;
	u32 PendingDelayFrames;
};

namespace scripts
{
	static script_context* g_ScriptCtx; // should be set only when a script is running

	enum movement_command
	{
		MoveCmd_Up,
		MoveCmd_Down,
		MoveCmd_Left,
		MoveCmd_Right,
		
		MoveCmd_OnspotR,
		MoveCmd_OnspotL,
		MoveCmd_OnspotD,
		MoveCmd_OnspotU
	};

	#define script_begin switch (g_ScriptCtx->Tick) { case 0: {
	#define yield g_ScriptCtx->Tick = __LINE__; return false; } break; case (__LINE__): {
	#define script_end } break; } return true;
}