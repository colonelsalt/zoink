#pragma once

namespace scripts::toll_gate
{
	using namespace commands;

	static b32 VendingMachine()
	{
		b32 NotThirsty = g_GameState->Inventory->Money >= 65'500 && g_GameState->Inventory->Money < 65'512;

		script_begin;
		LockPlayer();
		FacePlayer();

		g_GameState->OpenMenus = 0;
		if (NotThirsty)
		{
			Msg("Not so thirsty anymore...");
			ReleasePlayer();
		}
		else
		{
			const char* TestText = "What buy?";
			Msg(TestText);
		}
		WaitMsg();
		if (NotThirsty)
		{
			return true;
		}

		g_GameState->OpenMenus = Menu_VendingMachine | Menu_PlayerStatus;
		WaitVending();

		ReleasePlayer();
		script_end;
	}

	static b32 ShortVersion = false;

	static b32 TollBooth()
	{
		b32 HasEnoughMoney = g_GameState->Inventory->Money >= 65'500;
		script_begin;
		ShortVersion = GetFlag(ProgFlag_TollGuardSpokenOnce);

		LockPlayer();

		Face(Cardinal_Up, g_ScriptCtx->Player);
		g_GameState->OpenMenus = Menu_PlayerStatus;
		if (ShortVersion)
		{
			Msg("It's $65,500 to pass, ok??");
		}
		else
		{

			const char* TestText = "Oy there mate !\n"
								   "Where goin to??#p"
								   "Got to pay toll!!\n"
								   "Busy road innit?!#p"
								   "I not make rules, but time is tough !\n"
								   "It's $65,500 to pass, ok??";
			Msg(TestText);
		}
		WaitMsg();
		Delay(20);
		if (HasEnoughMoney)
		{
			mmEffect(SFX_MONEY);
			Msg("Oh much obliged sir !\nGood day, God bless ! Enjoy nice city !");
			TakeMoney(65'500);
			SetVar(Var_TollGateProgression, 1);
		}
		else
		{
			Msg("Common boy, not enough cash !\nGo make some money mate !");
			SetFlag(ProgFlag_TollGuardSpokenOnce);
		}
		WaitMsg();

		if (GetVar(Var_TollGateProgression) == 0)
		{
			v2 PlayerMove = v2 { 16, 0 };
			MoveStraight(PlayerMove, g_ScriptCtx->Player);
		}
		WaitMovement(WaitMovement_Player);
		g_GameState->OpenMenus = 0;
		ReleasePlayer();

		script_end;
	}

	static b32 Policeman()
	{
		script_begin;

		Msg("Happy day govnor !");
		WaitMsg();

		script_end;
	}

	static b32 PowerPill()
	{
		script_begin;

		Msg("Jack obtain PwrPill !");
		mmEffect(SFX_TREASURE);
		GiveItem(Item_PowerPill, 1);
		WaitMsg();
		SetFlag(ProgFlag_TollGatePowerPill);

		script_end;
	}

	static b32 KimonoLady()
	{
		script_begin;
		FacePlayer();
		Msg("I haven't got the money to pass into city,\n"
			"what a madness !#p"
			"The only course to make money in Zoink is\n"
			"#{ci:1;cs:9}defeating monsters#{ci:13;cs:14}, and I'm only old lady !#p"
			"World has no fairness !");
		WaitMsg();
		script_end;
	}

	static b32 Ronnie()
	{
		script_begin;
		FacePlayer();
		if (GetFlag(ProgFlag_RonnieTollGateSpokenOnce))
		{
			Msg("Ronnie: Well, the first magical stone is\nstraight ahead.#p"
				"Can't wait to kill some more of Groofus's\n"
				"goonies!\n"
				"...right?");
		}
		else
		{
			Msg("Ronnie: ...\n"
				"Hey, Jack, you made it...#p"
				"So we're almost at Cyan City.\n"
				"E-exciting, right..?#p"
				"...#p"
				"Say, Jack...#p"
				"I don't suppose you've noticed anything...\n"
				"strange going on during your adventure..?#p"
				"...#p"
				"No... Actually, you know what...\n"
				"I'm probably just imagining things.#p"
				"Well, the first magical stone is straight\nahead.#p"
				"Can't wait to kill some more of Groofus's\n"
				"goonies!\n"
				"...right?");
		}
		WaitMsg();
		SetFlag(ProgFlag_RonnieTollGateSpokenOnce);
		script_end;
	}

	static b32 FemaleTraveller()
	{
		script_begin;
		FacePlayer();
		Msg("Oh silly me !\n"
			"I exhausted all supplies of Poshon in battle !#p"
			"I always forgetting that #{ci:1;cs:9}items can only be\n"
			"used once#{ci:13;cs:14}, and then be gone !");
		WaitMsg();
		script_end;
	}

	static b32 ForestGuard()
	{
		script_begin;
		FacePlayer();
		Msg("Recognise, young adventure boy !#p"
			"The solitary way to kill big boss monsters is\n"
			"#{ci:1;cs:9}gain experience#{ci:13;cs:14} of destroying smallest ones.#p"
			"Such is the lane to much strength !");
		WaitMsg();
		script_end;
	}

	static b32 BusinessMan()
	{
		script_begin;
		FacePlayer();
		Msg("I hear some people around city found\n"
			"medicine of ultimate power, bestowing titanic\n"
			"strength for battle !#p"
			"But such is #{ci:1;cs:9}very rare#{ci:13;cs:14}, so if found a singular \n"
			"one I save it for only strongest boss !");
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
}