#pragma once

namespace scripts::jack_home_floorG
{
	using namespace commands;

	b32 HasSword = false;

	static b32 Mum()
	{
		script_begin;

		HasSword = HasItem(Item_Sword);

		FacePlayer();
		if (!GetFlag(ProgFlag_MumSpokenToOnce))
		{
			const char* TestText = "Mum: Happy morning Jack.\n"
								   "Adventure time is starting.#p"
								   "Every mother's know little boys\n"
								   "must fight evil when time is old !#p"
								   "Speak to me when needs restoring health..";

			Msg(TestText);
		}
		WaitMsg();

		if (!HasSword)
		{
			Msg("Oh Jack ?#pVisit Prof. Hokes lab first.\nHe has important present.");
		}
		else if (g_ScriptCtx->Player->Health < PLAYER_MAX_HEALTH)
		{
			Msg("Mum: Mother restore health fully, my son.");
		}
		else
		{
			Msg("Mum: Dont worry Jack, mothers understands.#p"
				"Just keep fighting monster, and #{ci:1;cs:9}dont run\n"
				"away#{ci:13;cs:14} ! You will do amazing.");
		}
		WaitMsg();


		SetFlag(ProgFlag_MumSpokenToOnce);
		script_end;
	}

	static b32 Cat()
	{
		script_begin;

		Msg("MEOW MEOW..");
		WaitMsg();

		script_end;
	}
}