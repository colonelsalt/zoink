#include "splash_tiles.cpp"
#include "splash_screen_map.cpp"


static void RunSplashScreen(ewram_arena* TempArena)
{
	REG_DISPCNT = DCNT_BLANK;

	InitSpritePalettes(TempArena);
	memcpy32(&tile_mem[0][0], grit::splash_tiles::Tiles, grit::splash_tiles::TilesCount / 4);
	memcpy32(&se_mem[30][0], bg::splash_screen_map::Tile_Layer_1, bg::splash_screen_map::SIZE / 4);
	REG_BG1CNT = BG_CBB(0) | 
				 BG_SBB(30) | 
				 BG_8BPP | 
				 BG_REG_32x32;

	SetBgPalette(grit::splash_tiles::Pal);
	BeginFadeIn(0, CLR_WHITE, true, 1);
	Render_Palettes();

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG1;

	constexpr u32 SPLASH_TIME = 120; // num frames to show it
	u32 FramesWaited = 0;

	b32 SplashDone = false;
	while (true)
	{
		if (SplashDone)
		{
			ResetArena(TempArena);
			break;
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
				FramesWaited++;
				if (FramesWaited >= SPLASH_TIME)
				{
					BeginFadeOut(0, CLR_WHITE, true, 1);
				}
			} break;

			case PalFade_OutDone:
			{
				SplashDone = true;
			} break;

			case PalFade_None:
			default:
			{
				Assert(false);
			} break;
		}

		VBlankIntrWait();

		Render_Palettes();
	}
}