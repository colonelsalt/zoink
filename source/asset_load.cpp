#include <tonc.h>
#include "asset_load.h"

#include "memory.h"

// Sprites from grit
#include "jack.cpp"
#include "hoke.cpp"
#include "ronnie.cpp"
#include "mum.cpp"
#include "old_dude.cpp"
#include "forest_guard.cpp"
#include "sandbox_girl.cpp"
#include "hobo.cpp"
#include "bikini_lady.cpp"
#include "goblin.cpp"
#include "male_aide.cpp"
#include "female_aid.cpp"
#include "policeman.cpp"
#include "item_box.cpp"
#include "orc.cpp"
#include "big_hoke.cpp"
#include "groofus.cpp"
#include "helga.cpp"
#include "kimono_lady.cpp"
#include "mustache_man.cpp"
#include "forest_boy.cpp"
#include "adventurer.cpp"
#include "traveller.cpp"
#include "female_traveller.cpp"
#include "business_man.cpp"

// Tiles from grit
#include "JacksHouse_1F.cpp"
#include "JacksHouse_GF.cpp"
#include "clut_town.cpp"
#include "HokesLab.cpp"
#include "forest_tiles.cpp"
#include "toll_gate_tiles.cpp"
#include "city_tiles.cpp"

// Levels from Tiled
#include "jack_home_floor1.cpp"
#include "jack_home_floorG.cpp"
#include "clut_v1.cpp"
#include "hokes_lab.cpp"
#include "forbidden_forest.cpp"
#include "toll_gate.cpp"
#include "city.cpp"


static const grit_pic* s_OwSprites[SpriteId_Count] =
{
/* SpriteId_None = */		     nullptr,
/* SpriteId_Player = */          &grit::jack::Pic,
/* SpriteId_Hoke = */            &grit::hoke::Pic,
/* SpriteId_Ronnie = */          &grit::ronnie::Pic,
/* SpriteId_Mum = */             &grit::mum::Pic,
/* SpriteId_OldDude = */         &grit::old_dude::Pic,
/* SpriteId_ForestGuard = */     &grit::forest_guard::Pic,
/* SpriteId_SandboxGirl = */     &grit::sandbox_girl::Pic,
/* SpriteId_Hobo = */            &grit::hobo::Pic,
/* SpriteId_BikiniLady = */      &grit::bikini_lady::Pic,
/* SpriteId_Hobgoblin = */       &grit::goblin::Pic,
/* SpriteId_MaleAid = */	     &grit::male_aide::Pic,
/* SpriteId_FemaleAid = */	     &grit::female_aid::Pic,
/* SpriteId_Policeman = */       &grit::policeman::Pic,
/* SpriteId_Item = */		     &grit::item_box::Pic,
/* SpriteId_Orc = */		     &grit::orc::Pic,
/* SpriteId_BigHoke = */         &grit::big_hoke::Pic,
/* SpriteId_Groofus = */	     &grit::groofus::Pic,
/* SpriteId_Helga = */		     &grit::helga::Pic,
/* SpriteId_KimonoLady = */      &grit::kimono_lady::Pic,
/* SpriteId_MustacheMan = */     &grit::mustache_man::Pic,
/* SpriteId_ForestBoy = */       &grit::forest_boy::Pic,
/* SpriteId_Adventurer = */      &grit::adventurer::Pic,
/* SpriteId_Traveller = */       &grit::traveller::Pic,
/* SpriteId_FemaleTraveller = */ &grit::female_traveller::Pic,
/* SpriteId_BusinessMan = */     &grit::business_man::Pic
};

static sprite* LoadOwSprite(sprite_id SpriteId, v2* WorldPos, sprite_size Size, ewram_arena* Arena)
{
	Assert(SpriteId != SpriteId_None && SpriteId < SpriteId_Count && SpriteId < countof(s_OwSprites));

	sprite* Result = PushStructZeroed(Arena, sprite);

	const grit_pic* Pic = s_OwSprites[SpriteId];

	// TODO: Detect whether we're loading a copy of a sprite that's already on the screen?
	// then we'd only need a new OAM entry
	u32 RequiredSize = FrameSizeInBytes(Size, BitDepth_4bpp);
	Result->VramTileIndex = AllocObjVram(RequiredSize);
	oam_entry* Oam = AllocOamEntry(WorldPos->Y.WholePart);

	Result->Size = Size;
	Result->PaletteBank = AllocPalBank(Pic->Palette);

	Result->TilesBasePtr = Pic->Tiles;
	Result->PalettePtr = Pic->Palette;

	if (Size == SpriteSize_16x32)
	{
		Result->IdleFrames[Cardinal_Down] = 3;
		Result->IdleFrames[Cardinal_Left] = 2;
		Result->IdleFrames[Cardinal_Right] = 0;
		Result->IdleFrames[Cardinal_Up] = 1;

		// Oink oink: policeman sits behind the BG2 layer counter
		u32 Priority = SpriteId == SpriteId_Policeman ? 2 : 1;

		obj_set_attr(&Oam->Attributes,
					 ATTR0_TALL,  // Tall, regular sprite
					 ATTR1_SIZE_16x32, // 16x32 pixels,
					 ATTR2_PALBANK(Result->PaletteBank) | ATTR2_PRIO(Priority) | ATTR2_ID(Result->VramTileIndex));
	}
	else if (Size == SpriteSize_32x32)
	{
		Result->IdleFrames[Cardinal_Down] = 0;
		Result->IdleFrames[Cardinal_Left] = 10; // TODO: HFlip for left
		Result->IdleFrames[Cardinal_Right] = 10;
		Result->IdleFrames[Cardinal_Up] = 1;

		obj_set_attr(&Oam->Attributes,
					 ATTR0_SQUARE,
					 ATTR1_SIZE_32x32,
					 ATTR2_PALBANK(Result->PaletteBank) | ATTR2_PRIO(1) | ATTR2_ID(Result->VramTileIndex));
	}
	else if (Size == SpriteSize_32x64)
	{
		Result->IdleFrames[Cardinal_Down] = 3;
		Result->IdleFrames[Cardinal_Left] = 2;
		Result->IdleFrames[Cardinal_Right] = 0;
		Result->IdleFrames[Cardinal_Up] = 1;

		obj_set_attr(&Oam->Attributes,
					 ATTR0_TALL,
					 ATTR1_SIZE_32x64,
					 ATTR2_PALBANK(Result->PaletteBank) | ATTR2_PRIO(0) | ATTR2_ID(Result->VramTileIndex));
	}
	else if (Size == SpriteSize_16x16)
	{
		memset32(Result->IdleFrames, 0, 1);
		//Result->IdleFrames = {};

		obj_set_attr(&Oam->Attributes,
					 ATTR0_SQUARE,
					 ATTR1_SIZE_16x16,
					 ATTR2_PALBANK(Result->PaletteBank) | ATTR2_PRIO(1) | ATTR2_ID(Result->VramTileIndex));

	}
	else if (Size == SpriteSize_64x64)
	{
		memset32(Result->IdleFrames, 0, 1);
		
		obj_set_attr(&Oam->Attributes,
					 ATTR0_SQUARE,
					 ATTR1_SIZE_64x64,
					 ATTR2_PALBANK(Result->PaletteBank) | ATTR2_PRIO(0) | ATTR2_ID(Result->VramTileIndex));
	}
	else
	{
		// TODO
		Assert(false);
	}

	Result->OamEntry = Oam;
	return Result;
}

static const level_tiles_mapping s_Levels[] =
{
/* Level_JacksHouse1F = */    { &bg::jack_home_floor1::Spec, &grit::JacksHouse_1F::Pic, false },
/* Level_JacksHouseGF = */    { &bg::jack_home_floorG::Spec, &grit::JacksHouse_GF::Pic, false },
/* Level_ClutTown = */        { &bg::clut_town::Spec, &grit::clut_v1::Pic, true },
/* Level_HokesLab = */	      { &bg::hokes_lab::Spec, &grit::HokesLab::Pic, true },
/* Level_ForbiddenForest = */ { &bg::forbidden_forest::Spec, &grit::forest_tiles::Pic, true },
/* Level_TollGate = */        { &bg::toll_gate::Spec, &grit::toll_gate_tiles::Pic, true },
/* Level_City = */			  { &bg::city::Spec, &grit::city_tiles::Pic, true }
};

static void LoadLevel(level_name LevelName, ewram_arena* Arena)
{
	Assert(LevelName < Level_End);
	REG_DISPCNT = 0;
	REG_BLDCNT = 0;

	level_tiles_mapping LevelMapping = s_Levels[LevelName];
	const level_spec* Spec = LevelMapping.LevelSpec;

	g_Level = PushStructZeroed(Arena, level);
	g_Level->Name = LevelName;
	g_Level->Arena = Arena;
	g_Level->TileWidth = Spec->TileWidth;
	g_Level->TileHeight = Spec->TileHeight;
	g_Level->IsBigMap = LevelMapping.IsBigMap;
	g_Level->IsCameraLocked = !LevelMapping.IsBigMap;
	g_Level->Tiles = LevelMapping.Pic->Tiles;
	g_Level->TilesSize = LevelMapping.Pic->TilesSize;
	g_Level->CollisionMap = Spec->CollisionMap;
	
	g_Level->NumLayers = Spec->NumMapLayers;
	g_Level->Layers = PushArrayZeroed(Arena, map_layer, Spec->NumMapLayers);
	g_Level->BgFlags = 0;
	for (u32 i = 0; i < Spec->NumMapLayers; i++)
	{
		map_layer* Layer = g_Level->Layers + i;

		Layer->Type = g_Level->IsBigMap ? MapType_Bigmap : MapType_32x32;
		Layer->MapSize = g_Level->TileWidth * g_Level->TileHeight * 2; // TODO: No need for per-layer value here
		Layer->Src = Spec->Layers[i];
		Layer->TileDimensions = iv2(g_Level->TileWidth, g_Level->TileHeight);
		Layer->ScreenBaseBlock = 30 - i;
		Layer->BgNo = i + 1;
		Layer->Dest = g_Level->IsBigMap ? se_mem[Layer->ScreenBaseBlock] : nullptr;
		
		Layer->BgControl = BG_PRIO(g_Level->NumLayers - i - 1) |
						   BG_CBB(0) | 
						   BG_SBB(Layer->ScreenBaseBlock) | 
						   BG_8BPP | 
						   BG_REG_32x32;

		REG_BGCNT[Layer->BgNo] = Layer->BgControl;

		if (!g_Level->IsBigMap)
		{
			memcpy32(&se_mem[Layer->ScreenBaseBlock][0], Layer->Src, Layer->MapSize / 4);
		}

		g_Level->BgFlags |= GetBgBit(Layer->BgNo);
	}

	// Sprites/entities
	g_Level->NumEntities = Spec->NumCharacters;
	g_Level->Entities = PushArrayZeroed(Arena, entity, g_Level->NumEntities);
	for (u32 i = 0; i < g_Level->NumEntities; i++)
	{
		const level_char* Char = Spec->Characters + i;
		entity* Entity = g_Level->Entities + i;

		Entity->VisibilityFlag = Char->VisibilityFlag;

		Entity->WorldPos = v2 { Char->WorldPos.X, Char->WorldPos.Y };
		Entity->SpriteId = Char->SpriteId;
		Entity->FacingCardinal = Char->FacingDirection;
		
		Entity->ScriptFunc = Char->ScriptPtr;

		Entity->Type = Char->IsLarge ? Entity_Enemy : Entity_Npc;

		if (Char->SpriteId != SpriteId_None)
		{
			sprite_size Size;
   			if (Char->SpriteId == SpriteId_Item)
			{
				Entity->Width = 16;
				Entity->Height = 16;
				Size = SpriteSize_16x16;
			}
			else if (Char->IsLarge)
			{
				Entity->Width = 32;
				Entity->Height = 32;
				Size = SpriteSize_32x32;
			}
			else
			{
				Entity->Width = 16;
				Entity->Height = 32;
				Size = SpriteSize_16x32;
			}

			Entity->Sprite = LoadOwSprite(Entity->SpriteId, &Entity->WorldPos, Size, Arena);
		}

		if (Entity->Type == Entity_Enemy)
		{
			if (Entity->SpriteId == SpriteId_Hobgoblin)
			{
				Entity->Health = 50;
				Entity->AttackPower = 5;
			}
			else if (Entity->SpriteId == SpriteId_Orc)
			{
				Entity->Health = 127;
				Entity->AttackPower = 10;
			}
		}
	}

	g_Level->PlayerReflection = PushStructZeroed(Arena, entity);
	g_Level->PlayerReflection->Sprite = LoadOwSprite(SpriteId_Player, &g_Level->PlayerReflection->WorldPos, SpriteSize_16x32, Arena);
	g_Level->PlayerReflection->Sprite->OamEntry->Attributes.attr1 |= ATTR1_VFLIP;
	obj_hide(&g_Level->PlayerReflection->Sprite->OamEntry->Attributes);

	// Warp tiles
	g_Level->NumWarps = Spec->NumWarps;
	g_Level->Warps = Spec->Warps;

	// Script tiles
	g_Level->NumScriptTiles = Spec->NumScripts;
	g_Level->ScriptTiles = Spec->ScriptTiles;

	SetBgPalette(LevelMapping.Pic->Palette);
	//memcpy32(pal_bg_mem, LevelMapping.Pic->Palette, LevelMapping.Pic->PaletteSize / 4);
	memcpy32(&tile_mem[0][0], g_Level->Tiles, g_Level->TilesSize / 4);

	OnLevelLoad();
}

static void UnloadLevel()
{
	ResetArena(g_Level->Arena);
	scripts::EndScript();
	g_GameState->OpenMenus = 0;
}

