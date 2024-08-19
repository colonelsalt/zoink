#include <tonc.h>
#include <maxmod.h>

#include "soundbank.h"
#include "soundbank_bin.h"

#include "util.h"
#include "typedefs.h"
#include "tonc_mgba.h"

#include "memory.h"
#include "input.h"

#include "maths.h"
#include "game.h"
#include "inventory.cpp"
#include "camera.cpp"


#include "oam.h"
#include "vram.h"
#include "palette.h"

#include "sprite.cpp"
#include "animation.cpp"
#include "entity.cpp"
#include "level.cpp"

#include "player.cpp"
#include "text.cpp"


#include "ow_scripts.cpp"
#include "battle.cpp"

#include "jack_home_floor1_scripts.cpp"
#include "jack_home_floorG_scripts.cpp"
#include "clut_town_scripts.cpp"
#include "hokes_lab_scripts.cpp"
#include "forbidden_forest_scripts.cpp"
#include "toll_gate_scripts.cpp"
#include "city_scripts.cpp"
#include "asset_load.cpp"

#include "splash_screen.cpp"
#include "title_screen.cpp"
#include "intro_sequence.cpp"
#include "end_sequence.cpp"

// Sample Maxmod setup code adapted from https://maxmod.org/ref/functions/mmInit.html
static u8 s_AudioMixingBuffer[ MM_MIXLEN_16KHZ ] __attribute((aligned(4)));
static void MaxmodInit(ewram_arena* Arena)
{
	constexpr u32 NUM_CHANNELS = 20;
 
    // allocate data for channel buffers & wave buffer (malloc'd data goes to EWRAM)
    // Use the SIZEOF definitions to calculate how many bytes to reserve
	u8* AudioEngineBuffer = PushSize(Arena, NUM_CHANNELS * (MM_SIZEOF_MODCH
									 + MM_SIZEOF_ACTCH
									 + MM_SIZEOF_MIXCH)
									 + MM_MIXLEN_16KHZ);
    
    // setup system info
	mm_gba_system GbaSystem;
    // 16KHz software mixing rate, select from mm_mixmode
    GbaSystem.mixing_mode       = MM_MIX_16KHZ;

    // number of module/mixing channels
    // higher numbers offer better polyphony at the expense
    // of more memory and/or CPU usage.
    GbaSystem.mod_channel_count = NUM_CHANNELS;
    GbaSystem.mix_channel_count = NUM_CHANNELS;
    
    // Assign memory blocks to pointers
    GbaSystem.module_channels   = (mm_addr)(AudioEngineBuffer + 0);
    GbaSystem.active_channels   = (mm_addr)(AudioEngineBuffer + (NUM_CHANNELS * MM_SIZEOF_MODCH));
    GbaSystem.mixing_channels   = (mm_addr)(AudioEngineBuffer + (NUM_CHANNELS * (MM_SIZEOF_MODCH + MM_SIZEOF_ACTCH)));
    GbaSystem.mixing_memory     = (mm_addr)s_AudioMixingBuffer;
    GbaSystem.wave_memory       = (mm_addr)(AudioEngineBuffer + (NUM_CHANNELS * (MM_SIZEOF_MODCH + MM_SIZEOF_ACTCH + MM_SIZEOF_MIXCH)));
    // Pass soundbank address
    GbaSystem.soundbank         = (mm_addr)soundbank_bin;

    // Initialize Maxmod
    mmInit( &GbaSystem );
}

static render_data s_RenderData;
static void OnVBlank()
{
	mmVBlank();
	if (s_RenderData.ReadyToDraw)
	{
		s_RenderData.ReadyToDraw = false;

		if (g_GameState->StateFlags & (GameState_IntroSequence | GameState_EndSequence))
		{
			Render_Oam();
			Render_UpdateDialogue(s_RenderData.Player);

			REG_BG_OFS[1] = { (s16)g_Camera.WorldPos.X.WholePart, (s16)g_Camera.WorldPos.Y.WholePart };
			if (!s_RenderData.Bg1Only)
			{
				REG_BG_OFS[2] = { (s16)g_Camera.WorldPos.X.WholePart, (s16)g_Camera.WorldPos.Y.WholePart };
			}
		}
		else if (!(g_GameState->StateFlags & GameState_TitleScreen))
		{
			for (u32 i = 1; i <= g_Level->NumLayers; i++)
			{
				REG_BG_OFS[i] = { (s16)g_Camera.WorldPos.X.WholePart, (s16)g_Camera.WorldPos.Y.WholePart };
			}

			entity* Player = s_RenderData.Player;
			Render_Sprite(Player->Sprite);

			for (u32 i = 0; i < g_Level->NumEntities; i++)
			{
				entity* Entity = g_Level->Entities + i;

				if (Entity->SpriteId != SpriteId_None)
				{
					Render_Sprite(Entity->Sprite);
				}
			}
			if (g_Level->PlayerReflection->IsInteractable())
			{
				Render_Sprite(g_Level->PlayerReflection->Sprite);
			}

			if (g_Level->IsBigMap)
			{
				UpdateBigMap();
			}

			Render_UpdateDialogue(Player);
			Render_UpdateInventoryText(Player);
			Render_Oam();
		}

		Render_UpdateAlphaBlend();

		if (!(g_GameState->StateFlags & GameState_EndScreen))
		{
			Render_Palettes();
		}

	}
	mmFrame();
}

int main()
{
	ewram_arena PermArena = {};
	PermArena.Memory = EWRAM_START;
	PermArena.Size = 64 * 1'024;
	
	ewram_arena MapArena = {};
	MapArena.Memory = EWRAM_START + PermArena.Size;
	MapArena.Size = 64 * 1'024;

	ewram_arena ScriptArena = {};
	ScriptArena.Memory = EWRAM_START + PermArena.Size + MapArena.Size;
	ScriptArena.Size = 32 * 1'024;

	ewram_arena FrameArena = {};
	FrameArena.Memory = EWRAM_START + PermArena.Size + ScriptArena.Size + MapArena.Size;
	FrameArena.Size = 96 * 1'024;
	

	MaxmodInit(&PermArena);
	InitObjVram(&MapArena);

	irq_init(nullptr);
	irq_enable(II_VBLANK);
	RunSplashScreen(&FrameArena);
	irq_add(II_VBLANK, OnVBlank);
	InitGameState(&PermArena);
	InitInput(&PermArena);

	RunTitleScreen(&FrameArena, &s_RenderData);
	

	InitOam(&MapArena);
	InitSpritePalettes(&MapArena);
	InitText(&PermArena);
	InitBattleState(&PermArena);
	InitCamera();

	RunIntroSequence(&FrameArena, &s_RenderData);
	g_GameState->StateFlags = 0;

	InitOam(&MapArena);
	InitSpritePalettes(&MapArena);
	entity Player = {};
	Player.Type = Entity_Player;
	Player.Width = 16;
	Player.Height = 32;
	Player.WorldPos = v2 { 110, 69 };	// NOTE: This is the legit start pos!
	//Player.WorldPos = v2 { 15, 27 } * 8;	
	//Player.WorldPos = v2 { 13, 23 } * 8;  //v2 { 62, 33 } * 8;	
	Player.Health = PLAYER_MAX_HEALTH;
	Player.AttackPower = 1;

	Player.SpriteId = SpriteId_Player;
	Player.Sprite = LoadOwSprite(SpriteId_Player, &Player.WorldPos, SpriteSize_16x32, &MapArena);
	
	s_RenderData.Player = &Player;

	LoadLevel(Level_JacksHouse1F, &MapArena);
	//LoadLevel(Level_TollGate, &MapArena);
	//LoadLevel(Level_TollGate, &MapArena);
	//LoadLevel(Level_TollGate, &MapArena);
	//LoadLevel(Level_ClutTown, &MapArena);

	if (g_Level->IsBigMap)
	{
		g_Camera.State = CamState_FollowPlayer;
		UpdateCamera(&Player);
		InitBigMap();
	}

	REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | g_Level->BgFlags;
	InitText(&PermArena);

	while (true)
	{
		ResetArena(&FrameArena);
		BufferInput();

		MovePlayer(&Player);
		if (g_GameState->StateFlags & GameState_PendingWarp)
		{
			g_GameState->OpenMenus = 0;
			pal_fade_state FadeState = GetFadeState();
			s32 FadeSpeed;
			if (g_GameState->StateFlags & GameState_PendingManualWarp)
			{
				FadeSpeed = 1;
			}
			else
			{
				FadeSpeed = DEFAULT_FADE_SPEED;
			}

			switch (FadeState)
			{
				case PalFade_None:
				{
					Player.StateFlags |= EFlag_Locked;
					BeginSongFadeOut();
					BeginFadeOut(OBJPALS_ALL, CLR_BLACK, true, FadeSpeed);
				} break;

				case PalFade_In:
				case PalFade_Out:
				{
					UpdateFade();
				} break;

				case PalFade_OutDone:
				{
					Warp(&Player);
					BeginFadeIn(OBJPALS_ALL, CLR_BLACK, true, FadeSpeed);
					Player.StateFlags &= ~EFlag_Locked;
				} break;

				case PalFade_InDone:
				{
					g_GameState->StateFlags &= ~GameState_PendingWarp;
					ResetFade();
					// TODO
					//InitText();
				} break;
			}
			
		}
		else if (g_GameState->StateFlags & GameState_EndSequence)
		{
			g_GameState->OpenMenus = 0;
			RunEndSequence(&Player, &s_RenderData, &MapArena);
		}
		
		UpdateCamera(&Player);
		UpdateEntity(&Player);

		if (g_Level->PlayerReflection->IsInteractable())
		{
			MovePlayer(g_Level->PlayerReflection, &Player);
			UpdateEntity(g_Level->PlayerReflection);
		}

		UpdateLevel();

		if (g_GameState->StateFlags & GameState_PendingScriptTile)
		{
			g_GameState->StateFlags &= ~GameState_PendingScriptTile;
			scripts::InitTileScript(g_GameState->PendingScriptTile->Script,
									&ScriptArena, 
									g_GameState->PendingScriptTile, 
									&Player);
		}
		if (g_Input->KeyPressed(KEY_A) &&
			!(Player.StateFlags & EFlag_Locked) &&
			!(g_GameState->StateFlags & GameState_InBattle) &&
			!(g_GameState->OpenMenus & Menu_TwoChoiceWindow))
		{
			collision_result Collision = WillCollide(&Player, Player.WorldPos + Player.FacingVector * 4);
			entity* ColEntity = Collision.OtherEntity;
			if (Collision.Type == ColType_Npc && ColEntity->ScriptFunc)
			{
				scripts::InitNpcScript(ColEntity->ScriptFunc,
									   &ScriptArena,
									   ColEntity,
									   &Player);
			}
			else if (g_Level->PlayerReflection->IsInteractable())
			{
				collision_result Collision = WillCollide(g_Level->PlayerReflection, 
														 g_Level->PlayerReflection->WorldPos + g_Level->PlayerReflection->FacingVector * 4);
				entity* ColEntity = Collision.OtherEntity;
				if (Collision.Type == ColType_Npc && ColEntity->ScriptFunc)
				{
					scripts::InitNpcScript(ColEntity->ScriptFunc,
										   &ScriptArena,
										   ColEntity,
										   &Player);
				}
			}


		}
		UpdateBattle();
		scripts::Update();

		UpdateInventorySelection(&Player);
		UpdateSongFade();

		FrameArena.Cursor = 0;
		s_RenderData.ReadyToDraw = true;
		VBlankIntrWait();
		g_FrameCount++;
		g_GameState->PrevFrameMenus = g_GameState->OpenMenus;
	}
}
