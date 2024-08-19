#pragma once

#include "sprite.h"

enum cardinal : u8
{
	Cardinal_Down,
	Cardinal_Right,
	Cardinal_Left,
	Cardinal_Up,
};

enum direction : u8
{
	Dir_Undetermined  = 0,
	Dir_Down          = 1,
	Dir_Left          = 1 << 2,
	Dir_Right         = 1 << 3,
	Dir_Up            = 1 << 4,

	Dir_Horizontal    = Dir_Left | Dir_Right,
	Dir_Vertical      = Dir_Up   | Dir_Down
};

inline direction CardinalToDirection(cardinal Cardinal)
{
	switch (Cardinal)
	{
		case Cardinal_Down:
			return Dir_Down;
		case Cardinal_Left:
			return Dir_Left;
		case Cardinal_Right:
			return Dir_Right;
		case Cardinal_Up:
			return Dir_Up;
	}
	Assert(false);
	return Dir_Undetermined;
}

enum entity_flags : u32
{
	EFlag_Disabled 		               = 0,
	EFlag_Visible  		               = 1,
	EFlag_Locked   		               = 1 << 1, // i.e. player does not move when d-pad pressed
	EFlag_ScriptStraightLineMovement   = 1 << 2, // moving in straight line according to script
	EFlag_ScriptManualMovement         = 1 << 3, // moving tile-by-tile according to script movement buffer,
	EFlag_ScriptLerpMovement		   = 1 << 4,

	EFlag_ScriptMovement = EFlag_ScriptStraightLineMovement | EFlag_ScriptManualMovement | EFlag_ScriptLerpMovement
};

struct rect
{
	v2 Pos; // top left
	iv2 Dimensions;
};

static constexpr s8 PLAYER_MAX_HEALTH = 20;

enum entity_type
{
	Entity_Player,
	Entity_Enemy,
	Entity_Npc,
	Entity_Reflection,
	//Entity_Sign
};

struct anim_clip;

struct entity
{
	entity_type Type;

	// Position of top left of sprite in the current level, pixel units
	v2 WorldPos;
	iv2 ScreenPos;

	s8 Health;
	s8 AttackPower;

	u32 VisibilityFlag; // if set, this entity should be invisible

	u32 StateFlags;

	cardinal FacingCardinal;
	cardinal PrevFacingCardinal; // the direction we were facing before this one

	b8 DirChangeInertia;
	v2 Velocity;
	v2 FacingVector;

	u16 Width;
	u16 Height;

	sprite_id SpriteId;
	sprite* Sprite;
	const anim_clip* PlayingAnimation;
	s32 AnimTimer; // Counts down from animation::FrameDelay to 0 between every animation frame
	u32 AnimFrameIndex; // Which frame of the currently playing animation are we on right now

	script_func ScriptFunc;

	iv2 ScriptMovementVector; // straight-line vector locked in from script
	iv2 StraightLineMovedSoFar;

	lerp_data LerpData;

	inline rect GetRect() const
	{
		rect Result;
		Result.Pos = WorldPos;
		
		if (Width == 16 && Height == 16)
		{
			Result.Dimensions = { 16, 16 };
		}
		else if (Width == 32 && Height == 32)
		{
			Result.Pos.X += 8;
			Result.Pos.Y += 14;
			Result.Dimensions = iv2 { 16, 10 };
		}
		else
		{
			Result.Pos.Y += 16;
			Result.Dimensions = iv2 { 16, 20 };
		}

		return Result;
	}

	inline iv2 TileDimensions() const
	{
		iv2 Result;
		Result.Width = Width >> 3;
		Result.Height = Height >> 3;
		return Result;
	}

	inline v2 TilePos() const
	{
		v2 Result;
		Result.X = WorldPos.X >> 3;
		Result.Y = WorldPos.Y >> 3;
		return Result;
	}

	inline b32 IsInteractable() const
	{
		b32 Result = (StateFlags & EFlag_Visible) && !(StateFlags & EFlag_Disabled);
		return Result;
	}

};

struct level_char // data for a character sprite specified in a level file
{
	sprite_id SpriteId;
	iv2 WorldPos;
	script_func ScriptPtr;
	cardinal FacingDirection;
	u32 VisibilityFlag;
	b32 IsLarge;
};

static constexpr v2 GetCentre(entity* Entity);
