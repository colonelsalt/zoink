#pragma once

namespace scripts::hokes_lab
{
	using namespace commands;

	static b32 HasSword = false;

	static b32 Hoke()
	{
		CSTR PermMsg = "Hoke: Recognise, Jack, when monster attacks\n"
					   "you must #{ci:1;cs:9}fight him until the end#{ci:13;cs:14}!#p"
					   "Smack monsters hard!\n"
					   "#{ci:1;cs:9}No running away#{ci:13;cs:14}!";
		  
		script_begin;
		HasSword = HasItem(Item_Sword);
		FacePlayer();

		if (!HasSword)
		{
			Msg("Hoke: Hello Jack !\n"
				"Its time for adventure beginnings !#p"
				"They say first magic rock is hidden\n"
				"in church in Cyan City.#p"
				"To arrive, Jack must pass beyond\n"
				"the Forest of Forbidden.#p"
				"But forest is filled up with monsters, and\n"
				"they jump out without signal !#p"
				"...#p"
				"So you understand, it's perilous to solitary\n"
				"go, take this!!");
		}
		WaitMsg();
		if (!HasSword)
		{
			mmEffect(SFX_TREASURE);
			GiveItem(Item_Sword, 1);
			Msg("Jack obtain Sword !");
			SetVar(Var_ClutTownProgression, 1);
		}
		WaitMsg();
		if (!HasSword)
		{
			Msg("Jack make Sword equipped.");
		}
		WaitMsg();
		
		Msg(PermMsg);
		WaitMsg();

		script_end;
	}

	static b32 MaleAid()
	{
		script_begin;

		FacePlayer();

		Msg("We needed great man like Prof. Hoke to\n"
			"realise what must doing to destroy Groofus.#p"
			"Magic rocks !\n"
			"Who could have think of this?!");
		WaitMsg();

		script_end;
	}

	static b32 FemaleAid()
	{
		script_begin;

		FacePlayer();

		Msg("It is a grand privilege to get help for\n"
			"your travel from such as Prof. Hoke.#p"
			"Dont waste opportunity's !\n"
			"Behave good !");
		WaitMsg();

		script_end;
	}
}