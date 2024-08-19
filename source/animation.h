#pragma once

struct anim_clip
{
	u32* Frames;
	u32 Length;
	u32 FrameDelay; // How many game frames (VBlanks) to wait in between animation frames
	b32 ShouldLoop;
};

namespace anim
{
	namespace npc
	{
		static constexpr u32 WALK_FRAME_DELAY = 6;

		static constexpr anim_clip WalkDown =
		{
			.Frames = (u32[]) { 46, 47, 48, 49, 50, 51 },
			.Length = 6,
			.FrameDelay = WALK_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip WalkLeft =
		{
			.Frames = (u32[]) { 40, 41, 42, 43, 44, 45 },
			.Length = 6,
			.FrameDelay = WALK_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip WalkRight =
		{
			.Frames = (u32[]) { 28, 29, 30, 31, 32, 33 },
			.Length = 6,
			.FrameDelay = WALK_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip WalkUp =
		{
			.Frames = (u32[]) { 34, 35, 36, 37, 38, 39 },
			.Length = 6,
			.FrameDelay = WALK_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr u32 LOITER_FRAME_DELAY = 7;

		static constexpr anim_clip LoiterRight =
		{
			.Frames = (u32[]) { 4, 5, 6, 7, 8, 9 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip LoiterUp =
		{
			.Frames = (u32[]) { 10, 11, 12, 13, 14, 15 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip LoiterLeft =
		{
			.Frames = (u32[]) { 16, 17, 18, 19, 20, 21 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip LoiterDown =
		{
			.Frames = (u32[]) { 22, 23, 24, 25, 26, 27 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip AttackRight =
		{
			.Frames = (u32[]) { 52, 53, 54, 55, 56, 57 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = false
		};

		static constexpr anim_clip AttackUp =
		{
			.Frames = (u32[]) { 58, 59, 60, 61, 62, 63 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = false
		};

		static constexpr anim_clip AttackLeft =
		{
			.Frames = (u32[]) { 64, 65, 66, 67, 68, 69 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = false
		};

		static constexpr anim_clip AttackDown =
		{
			.Frames = (u32[]) { 70, 71, 72, 73, 74, 75 },
			.Length = 6,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = false
		};
	}

	namespace enemy
	{
		constexpr u32 ATTACK_FRAME_DELAY = 10;
		constexpr u32 LOITER_FRAME_DELAY = 10;

		static constexpr anim_clip Loiter =
		{
			.Frames = (u32[]) { 0, 1, 2, 3, 4 },
			.Length = 5,
			.FrameDelay = LOITER_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip WalkDown =
		{
			.Frames = (u32[]) { 5, 6, 7, 8 },
			.Length = 4,
			.FrameDelay = npc::WALK_FRAME_DELAY,
			.ShouldLoop = true
		};

		static constexpr anim_clip AttackDown =
		{
			.Frames = (u32[]) { 17, 18, 19, 20 },
			.Length = 4,
			.FrameDelay = ATTACK_FRAME_DELAY,
			.ShouldLoop = false
		};
	}

	namespace groofus
	{
		static constexpr anim_clip Loiter =
		{
			.Frames = (u32[]) { 0, 1, 2, 3, 4, 5, 6, 7 },
			.Length = 8,
			.FrameDelay = 6,
			.ShouldLoop = true
		};
	}
}

struct directional_anim
{
	const anim_clip* WalkAnim;
	const anim_clip* LoiterAnim;
	u32 Button;
};

constexpr directional_anim SetUpDirAnims(cardinal Cardinal)
{
	switch (Cardinal)
	{
		case Cardinal_Up:
			return directional_anim{ &anim::npc::WalkUp, &anim::npc::LoiterUp, KEY_UP };
		case Cardinal_Right:
			return directional_anim{ &anim::npc::WalkRight, &anim::npc::LoiterRight, KEY_RIGHT };
		case Cardinal_Left:
			return directional_anim{ &anim::npc::WalkLeft, &anim::npc::LoiterLeft, KEY_LEFT };
		default:
			return directional_anim{ &anim::npc::WalkDown, &anim::npc::LoiterDown, KEY_DOWN };
	}
}

static constexpr directional_anim s_DirectionalAnims[] =
{
	SetUpDirAnims(Cardinal_Down),
	SetUpDirAnims(Cardinal_Right),
	SetUpDirAnims(Cardinal_Left),
	SetUpDirAnims(Cardinal_Up)
};