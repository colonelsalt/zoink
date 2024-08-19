#pragma once

#include <tonc.h>
#include "memory.h"
#include "util.h"

enum pal_fade_state
{
	PalFade_None,
	PalFade_In,
	PalFade_Out,
	PalFade_OutDone,
	PalFade_InDone
};

struct palette_state
{
	const COLOR* BgPalette; // atm this is always 256 colours

	const u16* ObjPalettes[16];
	u16 AllocatedBanks; // bit is 1 if bank allocated; 0 if free
	pal_fade_state FadeState;
	s32 FadeAmount; // 0-32

	COLOR TargetColour;
	u16 AffectedObjPalettes;
	b32 ShouldFadeBgPalette;
	s32 FadeSpeed;
};
static palette_state* s_Palettes;

static constexpr u16 OBJPALS_ALL = 0xFF'FF;
static constexpr s32 DEFAULT_FADE_SPEED = 6;

static void InitSpritePalettes(ewram_arena* Arena)
{
	memset32((void*)MEM_PAL_OBJ, 0, PAL_OBJ_SIZE >> 2);
	s_Palettes = PushStructZeroed(Arena, palette_state);
}

static void BeginFadeOut(u16 ObjPalFlags, COLOR TargetColour, b32 FadeBgPalette, s32 FadeSpeed = DEFAULT_FADE_SPEED)
{
	s_Palettes->FadeState = PalFade_Out;
	s_Palettes->FadeAmount = 0;

	s_Palettes->AffectedObjPalettes = ObjPalFlags;
	s_Palettes->TargetColour = TargetColour;
	s_Palettes->ShouldFadeBgPalette = FadeBgPalette;
	s_Palettes->FadeSpeed = FadeSpeed;
}

static void BeginFadeIn(u16 ObjPalFlags, COLOR TargetColour, b32 FadeBgPalette, s32 FadeSpeed = DEFAULT_FADE_SPEED)
{
	s_Palettes->FadeState = PalFade_In;
	s_Palettes->FadeAmount = 31;

	s_Palettes->AffectedObjPalettes = ObjPalFlags;
	s_Palettes->TargetColour = TargetColour;
	s_Palettes->ShouldFadeBgPalette = FadeBgPalette;
	s_Palettes->FadeSpeed = FadeSpeed;
}

static void UpdateFade()
{
	if (s_Palettes->FadeState == PalFade_Out)
	{
		s_Palettes->FadeAmount += s_Palettes->FadeSpeed;
		if (s_Palettes->FadeAmount > 32)
		{
			s_Palettes->FadeAmount = 32;
			s_Palettes->FadeState = PalFade_OutDone;
		}
	}
	else if (s_Palettes->FadeState == PalFade_In)
	{
		s_Palettes->FadeAmount -= s_Palettes->FadeSpeed;
		if (s_Palettes->FadeAmount < 0)
		{
			s_Palettes->FadeAmount = 0;
			s_Palettes->FadeState = PalFade_InDone;
		}
	}
}

static void ResetFade()
{
	s_Palettes->FadeState = PalFade_None;
}

static pal_fade_state GetFadeState()
{
	pal_fade_state Result = s_Palettes->FadeState;
	return Result;
}

static u32 AllocPalBank(const u16* Palette)
{
	u32 Result = 16;
	for (u32 i = 0; i < 16; i++)
	{
		if (!(s_Palettes->AllocatedBanks & (1 << i)))
		{
			s_Palettes->ObjPalettes[i] = Palette;
			s_Palettes->AllocatedBanks |= (1 << i);
			Result = i;
			break;
		}
	}
	Assert(Result < 16);
	return Result;
}

static void FreePalBank(u32 PalIndex)
{
	Assert(PalIndex < 16);
	s_Palettes->AllocatedBanks &= ~(1 << PalIndex);
}

static void SetBgPalette(const COLOR* Palette)
{
	s_Palettes->BgPalette = Palette;
}

inline static void Render_Palettes()
{
	for (u32 i = 0; i < 16; i++)
	{
		if (s_Palettes->FadeState == PalFade_None || !(s_Palettes->AffectedObjPalettes & (1 << i)))
		{
			memcpy32(&pal_obj_bank[i], s_Palettes->ObjPalettes[i], 32 >> 2);
		}
		else if (s_Palettes->AffectedObjPalettes & (1 << i))
		{
			COLOR* Target = pal_obj_bank[i];
			COLOR* Source = (COLOR*)s_Palettes->ObjPalettes[i];

			clr_fade_fast(Source, s_Palettes->TargetColour, Target, 16, s_Palettes->FadeAmount);
		}
	}

	if (s_Palettes->FadeState == PalFade_None)
	{
		memcpy32(pal_bg_mem, s_Palettes->BgPalette, (240 * 2) / 4);
	}
	else if (s_Palettes->ShouldFadeBgPalette)
	{
		COLOR* Target = pal_bg_bank[0];
		COLOR* Source = (COLOR*)s_Palettes->BgPalette;

		clr_fade_fast(Source, s_Palettes->TargetColour, Target, 256, s_Palettes->FadeAmount);
	}
}