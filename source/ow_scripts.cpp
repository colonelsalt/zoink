#include "ow_scripts.h"
#include "memory.h"

namespace scripts
{
	static void EndScript();


	static void InitScript(script_func ScriptFunc,
						   ewram_arena* Arena, 
						   entity* Player,
						   script_source Source)
	{
		if (g_GameState->StateFlags & GameState_ScriptRunning)
		{
			Assert(g_ScriptCtx);
			if (ScriptFunc == g_ScriptCtx->ActiveScript)
			{
				// If it's the same script that's already running, just ignore it.
				return;
			}
			else
			{
				EndScript();
			}
		}

		Assert(!(g_GameState->StateFlags & GameState_ScriptRunning));

		if (Source.Type == ScriptSource_Npc)
		{
			mmEffect(SFX_INTERACT);
		}

		g_ScriptCtx = PushStructZeroed(Arena, script_context);

		g_ScriptCtx->ActiveScript = ScriptFunc;
		g_ScriptCtx->Source = Source;
		g_ScriptCtx->Player = Player;

		g_ScriptCtx->Arena = Arena;
		g_ScriptCtx->Tick = 0;
		g_ScriptCtx->PendingDelayFrames = 0;

		g_GameState->StateFlags |= GameState_ScriptRunning;
	}

	static void InitNpcScript(script_func ScriptFunc,
							  ewram_arena* Arena, 
							  entity* ScriptEntity,
							  entity* Player)
	{
		script_source Source;
		Source.Type = ScriptSource_Npc;
		Source.Entity = ScriptEntity;
		InitScript(ScriptFunc, Arena, Player, Source);
	}

	static void InitTileScript(script_func ScriptFunc,
							   ewram_arena* Arena, 
							   const script_tile* Tile,
							   entity* Player)
	{
		script_source Source;
		Source.Type = ScriptSource_Tile;
		Source.Tile = Tile;
		InitScript(ScriptFunc, Arena, Player, Source);
	}

	static void EndScript()
	{
		if (IsDialogueBoxActive())
		{
			// TODO: This isn't happening during VBlank, obvs
			Render_HideTextBox();
		}

		ResetArena(g_ScriptCtx->Arena);
		g_ScriptCtx = nullptr;
		g_GameState->StateFlags &= ~GameState_ScriptRunning;
	}

	static void Update()
	{
		if (!g_ScriptCtx)
		{
			return;
		}

		Assert(g_ScriptCtx->ActiveScript);

		if (g_ScriptCtx->PendingDelayFrames > 0)
		{
			g_ScriptCtx->PendingDelayFrames--;
			return;
		}

		if (g_ScriptCtx->State == ScriptState_WaitingForBattle && 
			(g_GameState->StateFlags & GameState_InBattle))
		{
			return;
		}
		if (IsDialogueBoxActive())
		{
			if (g_ScriptCtx->State == ScriptState_WaitingForText)
			{
				return;
			}
		}
		if (g_ScriptCtx->State == ScriptState_WaitingForVendingMachine)
		{
			if (g_GameState->OpenMenus & Menu_VendingMachine)
			{
				return;
			}
		}
		if (g_ScriptCtx->State == ScriptState_WaitingForMovement)
		{
			switch (g_ScriptCtx->WaitMoveState)
			{
				case WaitMovement_ScriptEntity:
				{
					Assert(g_ScriptCtx->Source.Type == ScriptSource_Npc);
					if (g_ScriptCtx->Source.Entity->StateFlags & EFlag_ScriptMovement)
					{
						return;
					}
				} break;

				case WaitMovement_Player:
				{
					if (g_ScriptCtx->Player->StateFlags & EFlag_ScriptMovement)
					{
						return;
					}
				} break;

				case WaitMovement_All:
				{
					for (u32 i = 0; i < g_Level->NumEntities; i++)
					{
						entity* Entity = g_Level->Entities + i;
						if (Entity->StateFlags & EFlag_ScriptMovement)
						{
							return;
						}
					}
				} break;

				case WaitMovement_None:
					break;
			}
			g_ScriptCtx->WaitMoveState = WaitMovement_None;
		}

		g_ScriptCtx->State = ScriptState_Running;

		if (g_ScriptCtx->ActiveScript())
		{
			EndScript();
		}
	}

	namespace commands
	{
		static entity* _GetEntity()
		{
			Assert(g_ScriptCtx->Source.Type == ScriptSource_Npc);
			entity* Result = g_ScriptCtx->Source.Entity;
			Assert(Result);
			return Result;
		}

		static void SetOwningEntity(entity* Entity)
		{
			g_ScriptCtx->Source.Type = ScriptSource_Npc;
			g_ScriptCtx->Source.Entity = Entity;
		}

		static void LockPlayer()
		{
			g_ScriptCtx->Player->StateFlags |= EFlag_Locked;
		}

		static void Face(cardinal Direction, entity* Entity = nullptr)
		{
			if (!Entity)
			{
				Entity = _GetEntity();
			}
			
			Entity->PrevFacingCardinal = Entity->FacingCardinal;
			Entity->FacingCardinal = Direction;
		}

		static void FacePlayer(entity* Entity = nullptr)
		{
			if (!Entity)
			{
				Entity = _GetEntity();
			}

			v2 ToPlayer = g_ScriptCtx->Player->WorldPos - Entity->WorldPos;
			cardinal Facing = DominantDirectionFromVector(ToPlayer);

			Entity->PrevFacingCardinal = Entity->FacingCardinal;
			Entity->FacingCardinal = Facing;
		}

		static void Msg(CSTR String)
		{
			BeginTextBox(String);
		}

		static void ReleasePlayer()
		{
			g_ScriptCtx->Player->StateFlags &= ~EFlag_Locked;
		}

		static entity* GetEntity(sprite_id Id)
		{
			for (u32 i = 0; i < g_Level->NumEntities; i++)
			{
				entity* Entity = g_Level->Entities + i;
				if (Entity->SpriteId == Id)
				{
					return Entity;
				}
			}
			return nullptr;
		}

		static void MoveStraight(iv2 TileMovement, entity* Entity = nullptr)
		{
			if (!Entity)
			{
				Entity = _GetEntity();
			}

			Entity->ScriptMovementVector = TileMovement;
			Entity->StraightLineMovedSoFar = v2 { 0, 0 };
			Entity->StateFlags |= EFlag_ScriptStraightLineMovement;
		}

		static void _WaitMovement(wait_movement_type Type)
		{
			g_ScriptCtx->WaitMoveState = Type;
			g_ScriptCtx->State = ScriptState_WaitingForMovement;
		}

		// Async commands ---
		#define WaitMsg() g_ScriptCtx->State = ScriptState_WaitingForText; yield;
		#define Delay(NumFrames) g_ScriptCtx->PendingDelayFrames = NumFrames; yield;
		#define WaitMovement(Type) _WaitMovement(Type); yield;
		#define WaitBattle() g_ScriptCtx->State = ScriptState_WaitingForBattle; yield;
		#define WaitVending() g_ScriptCtx->State = ScriptState_WaitingForVendingMachine; yield;
	}
}