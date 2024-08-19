#include "battle.h"

#include "memory.h"

static battle_state* s_State;

// TODO: Ponder whether battles deserve their own arena, or whether keeping one state in the global perm. arena suffices..?
static void InitBattleState(ewram_arena* PermArena)
{
	s_State = PushStructZeroed(PermArena, battle_state);
}

static entity* GetEnemy()
{
	Assert(s_State && s_State->State);
	return s_State->Enemy;
}

static CSTR GetEnemyName()
{
	Assert(s_State && s_State->State);
	switch (s_State->EnemyType)
	{
		case Enemy_Hobgoblin:
			return "Hobgoblin";
		case Enemy_Orc:
			return "Orc";
		case Enemy_Count:
		default:
			Assert(false);
			return "???";
	}
}

static b32 IsPowerPillActive()
{
	if (g_GameState->StateFlags & GameState_InBattle)
	{
		Assert(s_State);
		b32 Result = s_State->PowerPillActive;
		return Result;
	}
	return false;
}

static void PowerPillBoost(b32 Active = true)
{
	Assert(s_State);
	constexpr s8 POWER_PILL_ATTACK_POWER = 32;
	if (Active)
	{
		s_State->PowerPillActive = true;
		s_State->RemainingPowerPillTurns = 2;

		s_State->Player->Health = PLAYER_MAX_HEALTH;
		s_State->Player->AttackPower = POWER_PILL_ATTACK_POWER;
	}
	else
	{
		s_State->PowerPillActive = false;
		s_State->Player->AttackPower = 1;
	}
}

static void BeginBattle(entity* Player, entity* Enemy, v2 PlayerLocation, enemy_type EnemyType)
{
	*s_State = {};
	s_State->Player = Player;
	s_State->Enemy = Enemy;
	s_State->PlayerLocation = PlayerLocation;
	s_State->EnemyLocation = Enemy->WorldPos;
	s_State->EnemyType = EnemyType;

	g_GameState->StateFlags |= GameState_InBattle;
	s_State->State = BattleState_Start;

	PlaySong(MOD_RPG_BATTLE);
}

static void EndBattle()
{
	mmEffect(SFX_DIE);
	if (s_State->Loser == Loser_Player)
	{
		BeginAlphaFade(s_State->Player->Sprite);
	}
	else
	{
		Assert(s_State->Loser == Loser_Enemy);
		BeginAlphaFade(s_State->Enemy->Sprite);
	}

	s_State->State = BattleState_EndingPendingCleanup;
	s_State->DelayFrames = 15;
}

static void UpdateTurn()
{
	switch (s_State->CurrentTurn.CurrentGo)
	{
		case Go_Player:
		{
			switch (s_State->CurrentTurn.GoProgress)
			{
				case 0:
				{
					BeginTextBox("Jack attack!!");
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 1:
				{
					v2 TargetPos = s_State->PlayerLocation;
					TargetPos.Y -= 3 * 8;
					LerpMoveEntity(s_State->Player, TargetPos, 32);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 2:
				{
					s_State->DelayFrames = 15;
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 3:
				{
					mmEffect(SFX_PLAYER_HIT);
					SetAnimation(s_State->Player, &anim::npc::AttackUp);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 4:
				{
					BeginFadeOut(1 << s_State->Enemy->Sprite->PaletteBank, CLR_WHITE, false, 8);
					s_State->DelayFrames = 10;
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 5:
				{
					BeginFadeIn(1 << s_State->Enemy->Sprite->PaletteBank, CLR_WHITE, false, 8);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 6:
				{
					ResetFade();
					siprintf(s_State->StringBuffer, "%s take damage of %d !", GetEnemyName(), s_State->Player->AttackPower);
					BeginTextBox(s_State->StringBuffer);
					TakeDamage(s_State->Enemy, s_State->Player->AttackPower);
					s_State->CurrentTurn.GoProgress++;
					
				} break;

				case 7:
				{
					if (s_State->Enemy->Health <= 0)
					{
						siprintf(s_State->StringBuffer, "%s defeated!!", GetEnemyName());
						BeginTextBox(s_State->StringBuffer);
						s_State->Loser = Loser_Enemy;
						s_State->DelayFrames = 20;
					}
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 8:
				{
					if (s_State->Loser != Loser_None)
					{
						EndBattle();
					}
					else
					{
						LerpMoveEntity(s_State->Player, s_State->PlayerLocation, 32);
					}
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 9:
				{
					s_State->Player->FacingCardinal = Cardinal_Up;
					s_State->CurrentTurn.CurrentGo = Go_Enemy;
					s_State->CurrentTurn.GoProgress = 0;
				} break;
			}
		} break;

		case Go_Enemy:
		{
			switch (s_State->CurrentTurn.GoProgress)
			{
				case 0:
				{
					siprintf(s_State->StringBuffer, "%s attack!!", GetEnemyName());
					BeginTextBox(s_State->StringBuffer);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 1:
				{
					v2 TargetPos = s_State->EnemyLocation;
					TargetPos.Y += 2 * 8;
					LerpMoveEntity(s_State->Enemy, TargetPos, 32);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 2:
				{
					s_State->DelayFrames = 15;
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 3:
				{
					mmEffect(SFX_ENEMY_HIT);
					SetAnimation(s_State->Enemy, &anim::enemy::AttackDown);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 4:
				{
					BeginFadeOut(1 << s_State->Player->Sprite->PaletteBank, CLR_WHITE, false, 8);
					s_State->DelayFrames = 10;
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 5:
				{
					BeginFadeIn(1 << s_State->Player->Sprite->PaletteBank, CLR_WHITE, false, 8);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 6:
				{
					ResetFade();
					s8 AttackPower = s_State->Enemy->AttackPower;
					if (s_State->PowerPillActive)
					{
						AttackPower /= 4;
					}
					siprintf(s_State->StringBuffer, "Jack take damage of %d !", AttackPower);
					BeginTextBox(s_State->StringBuffer);

					TakeDamage(s_State->Player, AttackPower);
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 7:
				{
					if (s_State->Player->Health <= 0)
					{
						BeginTextBox("Jack defeated!!");
						s_State->Loser = Loser_Player;
						s_State->DelayFrames = 20;
					}
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 8:
				{
					if (s_State->Loser != Loser_None)
					{
						EndBattle();
					}
					else
					{
						LerpMoveEntity(s_State->Enemy, s_State->EnemyLocation, 32);
					}
					s_State->CurrentTurn.GoProgress++;
				} break;

				case 9:
				{
					if (s_State->PowerPillActive)
					{
						s_State->RemainingPowerPillTurns--;
						if (s_State->RemainingPowerPillTurns <= 0)
						{
							mmEffect(SFX_POWERDOWN);
							BeginTextBox("PwrPill effects over !");
							PowerPillBoost(false);
						}
					}

					s_State->CurrentTurn.CurrentGo = Go_Done;
					s_State->CurrentTurn.GoProgress = 0;
				} break;
			} 
		} break;

		case Go_Done:
		{
			s_State->State = BattleState_MenuSelection;
			g_GameState->OpenMenus = Menu_BattleMenu;
		} break;
	}
}

static void UpdateBattle()
{
	if (s_State->State == BattleState_Inactive)
	{
		return;
	}
	else if (!(g_GameState->StateFlags & GameState_InBattle))
	{
		scripts::EndScript();
		s_State->State = BattleState_Inactive;
		return;
	}

	if (GetFadeState() && !(g_GameState->StateFlags & GameState_PendingWarp))
	{
		UpdateFade();
	}
	if (IsDialogueBoxActive() || 
		g_Camera.State == CamState_Lerping || 
		(s_State->Player->StateFlags & EFlag_ScriptLerpMovement) ||
		(s_State->Enemy->StateFlags & EFlag_ScriptLerpMovement) ||
		IsEntityInBlockingAnimation(s_State->Player) ||
		IsEntityInBlockingAnimation(s_State->Enemy))
	{
		return; // Wait for item description box to finish
	}
	if (g_GameState->StateFlags & GameState_AlphaBlending)
	{
		constexpr fixed BLEND_SPEED = fixed(0.5);

		g_GameState->AlphaBlendAmount -= BLEND_SPEED;
		if (g_GameState->AlphaBlendAmount < 0)
		{
			g_GameState->AlphaBlendAmount = 0;
			g_GameState->StateFlags &= ~GameState_AlphaBlending;
		}
		return;
	}
	else if (s_State->DelayFrames > 0)
	{
		s_State->DelayFrames--;
		return;
	}

	switch (s_State->State)
	{
		case BattleState_Inactive:
			return;

		case BattleState_Start:
		{
			g_GameState->OpenMenus = 0; // deliberately close all other menus
			v2 CamTarget = GetCameraCentredPos(s_State->Enemy);
			CamTarget.Y -= 2 * 8;
			CamTarget.X -= 2 * 8;
			LerpCamera(CamTarget, 64);
			LerpMoveEntity(s_State->Player, s_State->PlayerLocation, 64);
			s_State->Player->StateFlags |= EFlag_Locked;
			s_State->State = BattleState_MonsterIntro;
		} break;

		case BattleState_MonsterIntro:
		{
			siprintf(s_State->StringBuffer, "Wild %s like to battle !", GetEnemyName());
			BeginTextBox(s_State->StringBuffer);
			g_GameState->OpenMenus = Menu_EnemyStatus;
			s_State->State = BattleState_MenuSelection;
		} break;

		case BattleState_MenuSelection:
		{
			g_GameState->OpenMenus |= Menu_BattleMenu;
			s_State->Player->StateFlags &= ~EFlag_Locked;
			SetAnimation(s_State->Enemy, &anim::enemy::Loiter);
			if (g_Input->KeyPressed(KEY_A))
			{
				mmEffect(SFX_INTERACT);
				if (g_GameState->OpenMenus & Menu_Inventory)
				{
					if (g_GameState->Inventory->NumOccupiedSlots > 0)
					{
						if (UseSelectedItem(s_State->Player))
						{
							g_GameState->OpenMenus &= ~Menu_CommandWindow;

							battle_turn Turn = {};
							Turn.PlayerMove = BattleMove_PlayerItem;
							Turn.CurrentGo = Go_Enemy;

							s_State->CurrentTurn = Turn;

							s_State->State = BattleState_StartOfMoveExecution;
							g_GameState->OpenMenus &= ~(Menu_Inventory | Menu_CommandWindow);
						}
					}
				}
				else
				{
					if (g_GameState->Inventory->YesNoIndex == BattleMenu_Fight)
					{
						g_GameState->OpenMenus &= ~Menu_CommandWindow;

						battle_turn Turn = {};
						Turn.PlayerMove = BattleMove_PlayerAttack;

						s_State->CurrentTurn = Turn;

						s_State->State = BattleState_StartOfMoveExecution;
					}
					else
					{
						Assert(g_GameState->Inventory->YesNoIndex == BattleMenu_Pak);

						if (g_GameState->Inventory->NumOccupiedSlots == 0)
						{
							BeginTextBox("No item in PAK");
						}
						else
						{
							g_GameState->OpenMenus |= Menu_Inventory;
						}
					}
				}
			}
			else if (g_Input->KeyPressed(KEY_B))
			{
				g_GameState->OpenMenus &= ~Menu_Inventory;
			}
		} break;

		case BattleState_StartOfMoveExecution:
		{
			s_State->Player->StateFlags |= EFlag_Locked;
			s_State->Player->WorldPos = s_State->PlayerLocation;
			s_State->Player->FacingCardinal = Cardinal_Up;

			s_State->State = BattleState_ExecutingMoves;
		} break;

		case BattleState_ExecutingMoves:
		{
			UpdateTurn();
		} break;

		case BattleState_EndingPendingCleanup:
		{
			if (s_State->Loser == Loser_Player)
			{
				PlayerWhiteOut(s_State->Player);
				if (g_GameState->StateFlags & GameState_ScriptRunning)
				{
					// Don't finish the script if player lost, so he can return to it later
					scripts::EndScript();
				}
				s_State->State = BattleState_Inactive;
				g_GameState->StateFlags &= ~GameState_InBattle;
			}
			else
			{
				Assert(s_State->Loser == Loser_Enemy);
				mmEffect(SFX_VICTORY);
				SetFlag(s_State->Enemy->VisibilityFlag);

				v2 CamTarget = GetCameraCentredPos(s_State->Player);
				LerpCamera(CamTarget, 32);
				s_State->State = BattleState_EndingFinalLerp;
			}

			g_GameState->OpenMenus = 0;
			mmPause();
		} break;
		
		case BattleState_EndingFinalLerp:
		{
			g_Camera.State = CamState_FollowPlayer;
			s_State->State = BattleState_Inactive;
			s_State->Player->StateFlags &= ~EFlag_Locked;
			g_GameState->StateFlags &= ~GameState_InBattle;
		} break;
	}
}