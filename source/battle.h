#pragma once

enum enemy_type
{
	Enemy_Hobgoblin,
	Enemy_Orc,
	//Enemy_Spider,
	//Enemy_Troll,

	Enemy_Count
};

enum battle_move
{
	BattleMove_PlayerAttack,
	BattleMove_PlayerItem,
	BattleMove_EnemyAttack
};

enum turn_go // i.e. sub-step inside a turn
{
	Go_Player = 0,
	Go_Enemy,
	Go_Done
};

// Player always goes first

struct battle_turn
{
	battle_move PlayerMove;

	turn_go CurrentGo;

	u32 GoProgress;
};

enum battle_loser
{
	Loser_None,
	Loser_Player,
	Loser_Enemy
};

enum battle_state_state
{
	BattleState_Inactive,
	BattleState_Start,
	BattleState_MonsterIntro,
	BattleState_MenuSelection,
	BattleState_StartOfMoveExecution,
	BattleState_ExecutingMoves,
	BattleState_EndingPendingCleanup,
	BattleState_EndingFinalLerp
};

struct entity;

struct battle_state
{
	battle_state_state State;

	battle_turn CurrentTurn;

	entity* Player;
	entity* Enemy;
	v2 PlayerLocation;
	v2 EnemyLocation;

	u32 DelayFrames;

	b8 PowerPillActive;
	s8 RemainingPowerPillTurns;

	enemy_type EnemyType;
	battle_loser Loser;

	char StringBuffer[64];
};

static entity* GetEnemy();
static CSTR GetEnemyName();