#pragma once

namespace scripts::jack_home_floor1
{
	using namespace commands;

	static b32 Hoke()
	{
		script_begin;
		FacePlayer();

		const char* TestText = "Hello friend!!\n"
							   "My name Prof Hoke !\n"
							   "Wellcome to the word of Zoink !#p"
							   "This word inhabits many wired people,\n"
							   "and contaims many mysteriouses!!";
		GiveItem(Item_Sword, 1);
		GiveItem(Item_Potion, 1);
		GiveMoney(10'000);
		TakeDamage(g_ScriptCtx->Player, 5);

		Msg(TestText);
		WaitMsg();
		script_end;
	}
}