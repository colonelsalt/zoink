#include "end_seq_tiles.cpp"
#include "end_seq_map.cpp"

static void RunEndSequence(entity* Player, render_data* RenderData, ewram_arena* Arena)
{
	BeginAlphaFade(Player->Sprite);
	BeginSongFadeOut();

	g_GameState->OpenMenus = 0;

	entity Hoke = {};
	Hoke.FacingCardinal = Cardinal_Down;
	Hoke.Width = 32;
	Hoke.Height = 64;
	Hoke.WorldPos = v2 { 33, 15 } * 8;
	Hoke.SpriteId = SpriteId_BigHoke;
	Hoke.VisibilityFlag = ProgFlag_EndHokeVis;

	Player->StateFlags |= EFlag_Locked;

	u32 Step = 0;
	u32 PendingDelayFrames = 240;
	b32 HokeVisible = false;
	while (true)
	{
		BufferInput();
		UpdateFade();
		if (g_GameState->StateFlags & GameState_AlphaBlending)
		{
			constexpr fixed BLEND_SPEED = fixed(0.1);

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

		if (g_GameState->Volume == 0)
		{
		}

		if (PendingDelayFrames == 0 && !IsDialogueBoxActive())
		{
			switch (Step)
			{
				case 0:
				{
					Step++;
				} break;

				case 2:
				{
					PlaySong(MOD_AMB_SPACESHIP, true);
					mmEffect(SFX_KABOOM);
					ClearFlag(ProgFlag_EndHokeVis);
					Step++;
					PendingDelayFrames = 5;
				} break;

				case 3:
				{
					SetAnimation(&Hoke, &anim::npc::LoiterDown);
					Step++;
					PendingDelayFrames = 90;
				} break;

				case 4:
				{
					BeginTextBox("Well now, Jack..\n"
								 "I observe somebody not following rules..#p"
								 "I have gifted #{ci:1;cs:9}express instructions#{ci:13;cs:14}, but you do\nnot serve them..#p"
								"So I have no option..#p"
								 "#{ci:1;cs:9}I'll just have to put a stop to your adventure\n"
								 "a little earlier than planned...#{ci:13;cs:14}#p"
								"Oh, pardon beg !\n"
								"I mean to say.. bye bye good friend !\n"
								"..maybe you return to Zoink another day when\n"
								 "you have learn some manners??                       #{ci:1;cs:9}hahahahahahaahahahahahahahahahahahaahahah#{ci:13;cs:14}");
					Step++;
				} break;

				case 5:
				{
					Step++;
					PendingDelayFrames = 100;
				} break;

				case 6:
				{
					mmPause();
					BeginFadeOut(0xFFFF, CLR_BLACK, true, 100);
					Step++;
					PendingDelayFrames = 100;
				} break;

				case 8:
				{
					srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);
					tte_set_margins(80, 60, 231, 140);

					ResetCursor();
					tte_write("To be continued...\n");
					Step++;
					PendingDelayFrames = 140;
				} break;

				case 9:
				{
					TTC* Ttc = tte_get_context();
					Ttc->cursorY += 8;
					tte_write("http://sondre.io");
					Step++;
				} break;
			}
		}
		else if (PendingDelayFrames > 0)
		{
			PendingDelayFrames--;
		}

		if (HokeVisible)
		{
			UpdateEntity(&Hoke);
		}
		UpdateSongFade();
		RenderData->ReadyToDraw = true;
		VBlankIntrWait();
		g_FrameCount++;

		if (PendingDelayFrames == 0)
		{
			switch (Step)
			{
				case 1:
				{
					g_Camera.WorldPos = v2 { 164, 100 };
					InitOam(Arena);
					Hoke.Sprite = LoadOwSprite(SpriteId_BigHoke, &Hoke.WorldPos, SpriteSize_32x64, Arena);
					memcpy32(&tile_mem[0][0], grit::end_seq_tiles::Tiles, grit::end_seq_tiles::TilesCount / 4);
					memcpy32(&se_mem[27][0], bg::end_seq_map::Tile_Layer_1, bg::end_seq_map::SIZE / 4);
					REG_BG3CNT = BG_CBB(0) | 
								 BG_SBB(27) | 
								 BG_8BPP | 
								 BG_REG_32x32;
					REG_BG3VOFS = 0;
					REG_BG3HOFS = 0;
					Step++;
					HokeVisible = true;
				} break;

				case 7:
				{
					memset32((void*)MEM_VRAM, 0, VRAM_BG_SIZE / 4);
					g_GameState->StateFlags |= GameState_EndScreen;
					memset32(pal_bg_mem, 0, PAL_BG_SIZE);
					ResetFade();
					//tte_set_paper(0);
					//tte_erase_screen();
					InitText(Arena);
					REG_DISPCNT = DCNT_BG0;
					Step++;
				} break;
			}
		}

		if (HokeVisible)
		{
			Render_Sprite(Hoke.Sprite);
		}
		Render_Sprite(Player->Sprite);
	}
}