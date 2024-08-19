#include "intro_seq_tiles.cpp"
#include "intro_seq.cpp"


static void RunIntroQuotes()
{
	//mmPause();
	REG_DISPCNT = DCNT_BLANK;

	u16 InkColour = tte_get_ink();
	u16 ShadowColour = tte_get_shadow();
	
	// Write in black
	tte_set_ink(15);
	tte_set_shadow(14);

	tte_set_margins(9, 30, 231, 140);
	ResetCursor();

	REG_DISPCNT = DCNT_BG0;

	u32 Step = 0;
	u32 PendingDelay = 70;
	b32 Done = false;
	PlaySong(MOD_AMB_SERVERROOM, true);
	while (true)
	{
		BufferInput();

		if (Done)
		{
			break;
		}

		if (PendingDelay > 0)
		{
			PendingDelay--;
		}

		UpdateSongFade();
		VBlankIntrWait();
		if (PendingDelay == 0)
		{
			switch (Step)
			{
				case 0:
				{
					srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);
					tte_write("The simple believeth every word: but the prudent man looketh well to his going.\n"
							  "                                                 -Proverbs 14:15\n\n");
					Step++;
					PendingDelay = 310;
				} break;

				case 1:
				{
					tte_write("Sometimes I'm in a jam, I've gotta make a plan;\n"
							  "It might be crazy, I do it anyway.\n                                                    -H. Montana");
					Step++;
					PendingDelay = 350;
				} break;

				case 2:
				{
					REG_DISPCNT = DCNT_BLANK;
					PendingDelay = 40;
					Step++;
				} break;

				case 3:
				{
					mmPause();
					PendingDelay = 50;
					Step++;
				} break;

				case 4:
				{
					Done = true;
				} break;
			}
		}
	}


	tte_set_ink(InkColour);
	tte_set_shadow(ShadowColour);
	u16 PaperColour = tte_get_paper();
	tte_set_paper(0);
	tte_erase_screen();
	tte_set_paper(PaperColour);
}

static void RunIntroSequence(ewram_arena* TempArena, render_data* RenderData)
{
	g_GameState->StateFlags |= GameState_IntroSequence;

	REG_DISPCNT = DCNT_BLANK;

	RunIntroQuotes();
	//memset32((void*)MEM_VRAM, 0, VRAM_BG_SIZE / 4);

	InitSpritePalettes(TempArena);
	memcpy32(&tile_mem[0][0], grit::intro_seq_tiles::Tiles, grit::intro_seq_tiles::TilesCount / 4);
	

	memcpy32(&se_mem[30][0], bg::intro_seq::BG0, bg::intro_seq::SIZE / 4);
	REG_BG1CNT = BG_CBB(0) | 
				 BG_SBB(30) | 
				 BG_8BPP | 
				 BG_REG_32x32;

	memcpy32(&se_mem[29][0], bg::intro_seq::BG1, bg::intro_seq::SIZE / 4);
	REG_BG2CNT = BG_CBB(0) | 
				 BG_SBB(29) | 
				 BG_8BPP | 
				 BG_REG_32x32;

	entity Hoke = {};
	Hoke.FacingCardinal = Cardinal_Down;
	Hoke.Width = 32;
	Hoke.Height = 64;
	Hoke.WorldPos = v2 { 100, 25 };
	Hoke.SpriteId = SpriteId_BigHoke;
	Hoke.Sprite = LoadOwSprite(SpriteId_BigHoke, &Hoke.WorldPos, SpriteSize_32x64, TempArena);

	entity Groofus = {};
	Groofus.Width = 64;
	Groofus.Height = 64;
	Groofus.WorldPos = v2 { 140, -20 };
	Groofus.SpriteId = SpriteId_Groofus;
	Groofus.Sprite = LoadOwSprite(SpriteId_Groofus, &Groofus.WorldPos, SpriteSize_64x64, TempArena);
	Groofus.Sprite->OamEntry->Attributes.attr1 |= ATTR1_HFLIP;
	Groofus.VisibilityFlag = ProgFlag_GroofusVis;

	entity Helga = {};
	Helga.Width = 16;
	Helga.Height = 32;
	Helga.WorldPos = v2 { 54, 4 };
	Helga.SpriteId = SpriteId_Helga;
	Helga.Sprite = LoadOwSprite(SpriteId_Helga, &Helga.WorldPos, SpriteSize_16x32, TempArena);
	Helga.Sprite->OamEntry->Attributes.attr2 &= ~ATTR2_PRIO(1);
	Helga.VisibilityFlag = ProgFlag_HelgaVis;

	SetBgPalette(grit::intro_seq_tiles::Pal);
	BeginFadeIn(1 << Hoke.Sprite->PaletteBank, CLR_WHITE, true, 1);
	Render_Palettes();

	SetAnimation(&Groofus, &anim::groofus::Loiter);
	SetAnimation(&Helga, &anim::npc::LoiterDown);
	g_Camera.WorldPos = v2 { 0, 50 };
	g_Camera.State = CamState_Fixed;

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;

	PlaySong(MOD_COZY_WELCOME);

	b32 IntroComplete = false;
	b32 FadeInDone = false;
	u32 Step = 0;
	u32 PendingDelay = 25;
	LerpCamera(v2 { 0, 0 }, 80);
	while (true)
	{
		if (IntroComplete)
		{
			ResetArena(TempArena);
			break;
		}
		BufferInput();
		if (g_GameState->StateFlags & GameState_AlphaBlending)
		{
			constexpr fixed BLEND_SPEED = fixed(0.5);

			if (g_GameState->StateFlags & GameState_AlphaBlendingIn)
			{
				g_GameState->AlphaBlendAmount += BLEND_SPEED;
			}
			else
			{
				Assert(g_GameState->StateFlags & GameState_AlphaBlendingOut);
				g_GameState->AlphaBlendAmount -= BLEND_SPEED;
			}

			if (g_GameState->AlphaBlendAmount > 16 || g_GameState->AlphaBlendAmount < 0)
			{
				g_GameState->AlphaBlendAmount = 0;
				g_GameState->StateFlags &= ~GameState_AlphaBlending;
			}
		}

		pal_fade_state FadeState = GetFadeState();
		switch (FadeState)
		{
			case PalFade_In:
			case PalFade_Out:
			{
				UpdateFade();
			} break;

			case PalFade_InDone:
			{
				FadeInDone = true;
				ResetFade();
			} break;

			case PalFade_OutDone:
			{
				IntroComplete = true;
			} break;

			case PalFade_None:
			default:
			{
					
			} break;
		}
		if (g_Camera.State != CamState_Lerping && !IsDialogueBoxActive())
		{
			if (FadeInDone && PendingDelay == 0)
			{
				switch (Step)
				{
					case 0:
					{
						SetAnimation(&Hoke, &anim::npc::LoiterDown);
						PendingDelay = 5;
						Step++;
					} break;

					case 1:
					{
						BeginTextBox("Hello friend !\n"
									 "Wellcome to the word of Zoink!!#p"
									 "This word inhabits many wired people,\n"
									 "and contains many mystery's!#p"
									 "My name #{ci:1;cs:9}Prof. Hoke#{ci:13;cs:14}..!\n"
									 "Its very nice to meat you.#p"
									 "You funny boy..\n"
									 "Maybe I call you #{ci:1;cs:9}Jack#{ci:13;cs:14}, yes ?");

						Step++;
					} break;

					case 2:
					{
						SetAnimation(&Hoke, nullptr);
						RenderData->Bg1Only = true;
						LerpCamera(v2 { 0, -20 }, 64);
						Step++;
					} break;

					case 3:
					{
						SetAnimation(&Hoke, &anim::npc::LoiterDown);
						BeginTextBox("Jack, attention to me !\n"
									 "Zoink craves help!!");
						Step++;
					} break;

					case 4:
					{
						Step++;
					} break;

					case 5:
					{
						BeginTextBox("The evil #{ci:1;cs:9}Groofus#{ci:13;cs:14} demon king !\nTerrible man !");
						BeginAlphaFade(Groofus.Sprite, false);
						ClearFlag(ProgFlag_GroofusVis);
						Step++;
						PendingDelay = 10;
					} break;

					case 6:
					{
						Groofus.Sprite->OamEntry->Attributes.attr0 &= ~ATTR0_BLEND;
						BeginTextBox("He kidnap our princes, beutiful girl #{ci:1;cs:9}Helga#{ci:13;cs:14}.\n"
									 "We need strong hero to save Helga, prompt !#p"
									 "The legendary hero must gather #{ci:1;cs:9}seven magic\n"
									 "rocks#{ci:13;cs:14} and use power to kill evil Groofus and\n"
									 "sow peace upon the kingdom!!");
						BeginAlphaFade(Helga.Sprite, false);
						ClearFlag(ProgFlag_HelgaVis);
						Step++;
						PendingDelay = 10;
					} break;

					case 7:
					{
						BeginAlphaFade(Groofus.Sprite);
						BeginAlphaFade(Helga.Sprite);
						SetAnimation(&Hoke, nullptr);
						LerpCamera(v2 { 0, 0 }, 64);
						PendingDelay = 30;
						Step++;
					} break;

					case 8:
					{
						SetAnimation(&Hoke, &anim::npc::LoiterDown);
						PendingDelay = 5;
						Step++;
					} break;

					case 9:
					{
						BeginTextBox("Jack!!\n"
									 "You fathom ?#p"
									 "Your adventure is abound to unfold..#p"
									 "Remember speak to men of Zoink.\n"
									 "Make everybody happy friends.#p"
									 "They will give #{ci:1;cs:9}important hints#{ci:13;cs:14} among journey.\n"
									 "Never will be stuck. You will understand.");
						Step++;
					} break;

					case 10:
					{
						SetAnimation(&Hoke, nullptr);
						Step++;
						PendingDelay = 20;
					} break;

					case 11:
					{
						mmPause();
						BeginTextBox("Good luck..");
						Step++;
					} break;

					case 12:
					{
						LerpCamera(v2 { 0, 50 }, 80);
						BeginFadeOut(1 << Hoke.Sprite->PaletteBank, CLR_BLACK, true, 1);
					} break;
				}
			}
			else if (PendingDelay > 0)
			{
				PendingDelay--;
			}

		}
		UpdateCamera(nullptr);
		UpdateEntity(&Hoke);
		UpdateEntity(&Groofus);
		UpdateEntity(&Helga);
		UpdateSongFade();
		RenderData->ReadyToDraw = true;
		VBlankIntrWait();

		Render_Sprite(Hoke.Sprite);
		Render_Sprite(Groofus.Sprite);
		Render_Sprite(Helga.Sprite);
	}

}