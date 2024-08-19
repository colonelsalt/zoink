namespace scripts::forbidden_forest
{
	using namespace commands;

	static b32 Potion()
	{
		script_begin;

		mmEffect(SFX_TREASURE);
		Msg("Jack obtain Poshon !");
		GiveItem(Item_Potion, 1);
		WaitMsg();
		SetFlag(ProgFlag_ForestPotion);

		script_end;
	}

	static b32 Ronnie()
	{
		if (GetFlag(ProgFlag_RonnieForestSpokenOnce))
		{
			script_begin;

			FacePlayer();
			Msg("Ronnie: I'm just, er... taking a little breather\n"
				"before I carry on.");
			WaitMsg();

			script_end;
		}
		else
		{
			script_begin;

			FacePlayer();
			Msg("Ronnie: Hey Jack, so you made it too?#p"
				"Say... you didn't happen to see me come in\nearlier, did you?#p"
				"...#p"
				"Oh nothing!\n"
				"Should've been there, man!#p"
				"Those monsters blocking the forest\n"
				"entrance were no match for me!#p"
				"Ha ha..!#p"
				"I'm just, er... taking a little breather before\n"
				"I carry on.");
			WaitMsg();
			SetFlag(ProgFlag_RonnieForestSpokenOnce);

			script_end;
		}
	}

	static b32 Traveller()
	{
		script_begin;

		FacePlayer();
		Msg("Wow, only the powerful hero can cross gates\n"
			"of the Forest of Forbidden !#p"
			"You must be super strong !");
		WaitMsg();

		script_end;
	}

	static b32 Adventurer()
	{
		script_begin;

		FacePlayer();
		Msg("I came long way to cross a forest,\n"
			"but its full with #{ci:1;cs:9}impregnable water#{ci:13;cs:14}.#p"
			"...Maybe should I go back to buy a boat ?");
		WaitMsg();

		script_end;
	}

	static b32 ForestBoy()
	{
		script_begin;

		FacePlayer();
		Msg("I like sitting to look at the water, but mumy\n"
			"thinks is lazy.#p"
			"Says '#{ci:1;cs:9}no good comes from staring at\nreflections#{ci:13;cs:14} all day!'#p"
			"She probly right.");
		WaitMsg();

		script_end;
	}

	static b32 MustacheMan()
	{
		script_begin;

		FacePlayer();
		Msg("Curses be Groofus !#p"
			"He flooded our forest, so there is #{ci:1;cs:9}no more\n"
			"path to the city#{ci:13;cs:14} !#p"
			"We must as well wait until sunshine returns\n"
			"to dry the land again..");
		WaitMsg();

		script_end;
	}
}