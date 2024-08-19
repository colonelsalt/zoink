#include "sprite.h"

constexpr u32 FrameSizeInTiles(sprite_size Size)
{
	u32 Result = 0;
	switch(Size)
	{
		case SpriteSize_8x8:
		{
			Result = 1;
		} break;

		case SpriteSize_16x16:
		{
			Result = 2 * 2;
		} break;

		case SpriteSize_16x32:
		{
			Result = 2 * 4;
		} break;

		case SpriteSize_32x32:
		{
			Result = 4 * 4;
		} break;

		case SpriteSize_32x64:
		{
			Result = 4 * 8;
		} break;

		case SpriteSize_64x64:
		{
			Result = 8 * 8;
		} break;

		default:
			Assert(false); // Didn't implement other sizes yet
	}
	return Result;
}

constexpr u32 FrameSizeInBytes(sprite_size Size, bit_depth BitDepth)
{
	u32 TileSize = BitDepth == BitDepth_4bpp ? sizeof(TILE) : sizeof(TILE8);

	u32 Result = FrameSizeInTiles(Size) * TileSize;
	return Result;
}

static void BeginAlphaFade(sprite* Sprite, b32 FadeOut = true)
{
	if (FadeOut)
	{
		g_GameState->StateFlags |= GameState_AlphaBlendingOut;
	}
	else
	{
		g_GameState->StateFlags |= GameState_AlphaBlendingIn;
	}
	g_GameState->AlphaBlendAmount = FadeOut ? 16 : 0;

	Sprite->OamEntry->Attributes.attr0 |= ATTR0_BLEND;
	REG_BLDCNT |= BLD_BUILD(BLD_OBJ, BLD_BG1 | BLD_BG2 | BLD_BG3, BLD_STD);
}

static void Render_UpdateAlphaBlend()
{
	if (g_GameState->StateFlags & GameState_AlphaBlending)
	{
		REG_BLDALPHA = ((16 - g_GameState->AlphaBlendAmount.WholePart) << 8) | g_GameState->AlphaBlendAmount.WholePart;
	}
}

static void Render_Sprite(sprite* Sprite)
{
	u32 FrameSize = FrameSizeInBytes(Sprite->Size, Sprite->BitDepth);
	const u8* Frame = Sprite->TilesBasePtr + Sprite->FrameIndex * FrameSize;
	memcpy32(&tile_mem[4][Sprite->VramTileIndex], Frame, FrameSize >> 2);
}