#include "title_screen_tiles.cpp"
#include "title_screen_map.cpp"


static void RunTitleScreen(ewram_arena* TempArena, render_data* RenderData)
{
	g_GameState->StateFlags = GameState_TitleScreen;

	REG_DISPCNT = DCNT_BLANK;

	InitSpritePalettes(TempArena);
	memcpy32(&tile_mem[0][0], grit::title_screen_tiles::Tiles, grit::title_screen_tiles::TilesCount / 4);
	

	memcpy32(&se_mem[31][0], bg::title_screen_map::BG0, bg::title_screen_map::SIZE / 4);
	REG_BG0CNT = BG_CBB(0) | 
				 BG_SBB(31) | 
				 BG_8BPP | 
				 BG_REG_32x32;

	memcpy32(&se_mem[30][0], bg::title_screen_map::BG1, bg::title_screen_map::SIZE / 4);
	REG_BG1CNT = BG_CBB(0) | 
				 BG_SBB(30) | 
				 BG_8BPP | 
				 BG_REG_32x32;

	memcpy32(&se_mem[29][0], bg::title_screen_map::BG2, bg::title_screen_map::SIZE / 4);
	REG_BG2CNT = BG_CBB(0) | 
				 BG_SBB(29) | 
				 BG_8BPP | 
				 BG_REG_32x32;

	SetBgPalette(grit::title_screen_tiles::Pal);
	BeginFadeIn(0, CLR_WHITE, true, 1);
	Render_Palettes();

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2;

	//PlaySong(MOD_AMB_SERVERROOM, true); // this is annoyingly song ID 0...
	PlaySong(MOD_COZY_PRETTYPLATE);

	b32 ReadyToStart = false;
	b32 TitleDone = false;

	u32 FramesPassed = 0;
	b32 PressStartVisible = false;

	constexpr fixed SCROLL_SPEED = fixed(0.4);
	fixed ScrollAmount = 0;
	while (true)
	{
		if (TitleDone)
		{
			ResetArena(TempArena);
			break;
		}

		BufferInput();
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
				ReadyToStart = true;
			} break;

			case PalFade_OutDone:
			{
				TitleDone = true;
			} break;

			case PalFade_None:
			default:
			{
				Assert(false);
			} break;
		}


		if (ReadyToStart && (g_Input->KeyPressed(KEY_A) || g_Input->KeyPressed(KEY_START)))
		{
			BeginFadeOut(0, CLR_WHITE, true, 1);
			BeginSongFadeOut();
		}

		RenderData->ReadyToDraw = true;
		if ((FramesPassed % 58) == 0)
		{
			PressStartVisible = !PressStartVisible;
		}
		if (PressStartVisible)
		{
			REG_DISPCNT |= DCNT_BG1;
		}
		else
		{
			REG_DISPCNT &= ~DCNT_BG1;
		}

		ScrollAmount += SCROLL_SPEED;
		FramesPassed++;
		UpdateSongFade();
		VBlankIntrWait();
		REG_BG2HOFS = ScrollAmount.WholePart;
	}

}