#pragma once

namespace scripts::clut_town
{
	using namespace commands;

	static b32 OldDude()
	{
		script_begin;
		FacePlayer();

		const char* TestText = "Ho ho !\nLittle Jack is begins adventure !#p"
							   "Kiddo are really mothers son, eh?!\n"
							   "You #{ci:1;cs:9}look exactly like she#{ci:13;cs:14} !";

		Msg(TestText);
		WaitMsg();
		script_end;
	}

	static b32 SandboxGirl()
	{
		script_begin;
		FacePlayer();

		Msg("Groofus bad !\n"
			"Helga good !#p"
			"Prof. Hoke #{ci:1;cs:9}very smart#{ci:13;cs:14} !");
		WaitMsg();
		Face(Cardinal_Right);
		script_end;
	}

	static b32 Hobo()
	{
		script_begin;
		FacePlayer();

		Msg("I was once adventurer like you Jack,\n"
			"but then took a punch to my head.#p"
			"If only I #{ci:1;cs:9}listened to my elders#{ci:13;cs:14}, I could\n"
			"still be best hero !");
		WaitMsg();
		script_end;
	}

	static b32 BikiniLady()
	{
		script_begin;
		FacePlayer();

		Msg("Truly, the water in Zoink is #{ci:1;cs:9}beutiful blue#{ci:13;cs:14} !\n"
			"Have you looked close on it ?");
		WaitMsg();
		Face(Cardinal_Down);
		script_end;
	}

	static CSTR GuardBlockMsg = "This way enters the Forest of Forbidden !#p"
								"Monsters attack, you need weapon for\ndefending !";
	static b32 ForestGuard()
	{
		script_begin;
		FacePlayer();

		if (HasItem(Item_Sword))
		{
			Msg("With sword in hand, you can kill monsters !#p"
				"But be strong ! Once battle begins, there is\n"
				"#{ci:1;cs:9}no escape#{ci:13;cs:14}!");
		}
		else
		{
			Msg(GuardBlockMsg);
		}
		WaitMsg();
		script_end;
	}

	static b32 GuardBlockTile()
	{
		script_begin;
		LockPlayer();
		entity* Guard = GetEntity(SpriteId_ForestGuard);
		SetOwningEntity(Guard);

		Face(Cardinal_Right, Guard);
		Face(Cardinal_Left, g_ScriptCtx->Player);

		Msg(GuardBlockMsg);
		WaitMsg();

		v2 PlayerMove = v2 { 0, 16 };
		MoveStraight(PlayerMove, g_ScriptCtx->Player);
		WaitMovement(WaitMovement_Player);

		ReleasePlayer();
		script_end;
	}

	static b32 RonnieIntroTile()
	{
		entity* Ronnie = GetEntity(SpriteId_Ronnie);
		script_begin;
		g_GameState->OpenMenus = 0;
		LockPlayer();
		SetOwningEntity(Ronnie);
		ClearFlag(ProgFlag_ClutTownRonnieVis);

		Msg("Hey man, wait up!");
		WaitMsg();
		Face(Cardinal_Right, g_ScriptCtx->Player);

		SetOwningEntity(Ronnie);
		v2 TargetLoc = v2 { 48, 27 } * 8;
		LerpMoveEntity(Ronnie, TargetLoc, 128);
		WaitMovement(WaitMovement_ScriptEntity);

		Msg("You're Jack, aren't you?#p"
			"Yeah, I heard you're going out to save Princess\nHelga, eh?#p"
			"Ha!\n"
			"Not if I get to her first!#p"
			"Those magical stones are as good as mine!\n"
			"That Groofus won't know what hit him!");
		WaitMsg();
		SetOwningEntity(Ronnie);

		v2 PlayerTarget = v2 { 46, 25.5 } * 8;
		LerpMoveEntity(g_ScriptCtx->Player, PlayerTarget, 20);
		Delay(22);
		Face(Cardinal_Down, g_ScriptCtx->Player);
		v2 RonTarget = v2 { 43, 28 } * 8;
		LerpMoveEntity(Ronnie, RonTarget, 64);
		WaitMovement(WaitMovement_All);
		
		Face(Cardinal_Down, g_ScriptCtx->Player);
		SetOwningEntity(Ronnie);
		Face(Cardinal_Right);

		Msg("I'm Ronnie, by the way. At least...\n"
			"that's what they call me around here.#p"
			"...#p"
			"Well, what are you waiting for, Jackie?\n"
			"May the best adventurer win!");
		WaitMsg();

		v2 RonTarget = v2 { 23, 29 } * 8;
		LerpMoveEntity(Ronnie, RonTarget, 70);
		WaitMovement(WaitMovement_All);

		ReleasePlayer();
		SetFlag(ProgFlag_ClutTownRonnieVis);
		SetVar(Var_ClutTownProgression, 2);
		script_end;
	}

	static b32 Tonic()
	{
		script_begin;

		mmEffect(SFX_TREASURE);
		Msg("Jack obtain Poshon !");
		GiveItem(Item_Potion, 1);
		WaitMsg();
		SetFlag(ProgFlag_ClutTownPotion);

		script_end;
	}

	static b32 GoblinBattle()
	{
		entity* Goblin = GetEntity(SpriteId_Hobgoblin);
		script_begin;
		LockPlayer();
		ClearFlag(ProgFlag_ClutTownGoblinVis);

		SetOwningEntity(Goblin);
		
		v2 GoblinMovement = v2 { 0, 3 * 8 };
		MoveStraight(GoblinMovement);
		WaitMovement(WaitMovement_ScriptEntity);

		// v2 PlayerMovement = 

		Delay(10);
		ReleasePlayer();

		constexpr v2 PlayerBattlePos = v2 { 17 * 8, 17 * 8 };
		BeginBattle(g_ScriptCtx->Player, Goblin, PlayerBattlePos, Enemy_Hobgoblin);
		WaitBattle();

		Msg("If you're reading this: well done.\n"
			"Did you use cheat codes or something??");
		WaitMsg();
		SetVar(Var_ClutTownProgression, 3);

		script_end;
	}

	static b32 LabSign()
	{
		script_begin;
		Msg("Prof. Hokes lab");
		WaitMsg();
		script_end;
	}

	static b32 LockedDoor()
	{
		script_begin;
		Msg("Door locked.");
		WaitMsg();
		script_end;
	}

	static b32 RonniesHouseSign()
	{
		script_begin;
		Msg("Ronnies house");
		WaitMsg();
		script_end;
	}

	static b32 JacksHouseSign()
	{
		script_begin;
		Msg("Jacks house");
		WaitMsg();
		script_end;
	}
}