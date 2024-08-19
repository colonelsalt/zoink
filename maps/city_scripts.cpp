#pragma once

namespace scripts::city
{
	using namespace commands;

	static b32 OrcBattle()
	{
		entity* Orc = GetEntity(SpriteId_Orc);
		script_begin;
		ClearFlag(ProgFlag_CityOrcVis);
		LockPlayer();

		SetOwningEntity(Orc);
		
		v2 GoblinMovement = v2 { 0, 5 * 8 };
		MoveStraight(GoblinMovement);
		WaitMovement(WaitMovement_ScriptEntity);

		// v2 PlayerMovement = 

		Delay(10);
		ReleasePlayer();

		constexpr v2 PlayerBattlePos = v2 { 33 * 8, 30 * 8 };
		BeginBattle(g_ScriptCtx->Player, Orc, PlayerBattlePos, Enemy_Orc);
		WaitBattle();

		SetVar(Var_CityProgression, 1);
		SetFlag(ProgFlag_OrcDefeated);

		script_end;
	}

	static b32 OldDude()
	{
		script_begin;
		FacePlayer();
		Msg("Have no chance of destroying monster who\nblocks church !#p"
			"Best I ever hope for is #{ci:1;cs:9}run away#{ci:13;cs:14} and hope\n"
			"somebody other take handle of him !");
		WaitMsg();
		script_end;
	}

	static b32 Traveller()
	{
		script_begin;
		FacePlayer();
		Msg("If only had I further strength..\n"
			"Maybe should just #{ci:1;cs:9}keep training#{ci:13;cs:14}..?");
		WaitMsg();
		script_end;
	}

	static b32 SandboxGirl()
	{
		script_begin;
		FacePlayer();
		Msg("All grown ups saying that Orc is\n#{ci:1;cs:9}indefeatable#{ci:13;cs:14} !#p"
			"I guessing now it will be no more church for me\nany more, hihi !");
		WaitMsg();
		script_end;
	}

	static b32 Hobo()
	{
		script_begin;
		FacePlayer();
		Msg("Ever since Groofus came for that #{ci:1;cs:9}magic rock#{ci:13;cs:14}\n"
			"city has become such dangerous..");
		WaitMsg();
		script_end;
	}
}