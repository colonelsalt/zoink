#include "animation.h"

// e.g. non-looping attack animation
static b32 IsEntityInBlockingAnimation(entity* Entity)
{
	b32 Result = Entity->PlayingAnimation && !Entity->PlayingAnimation->ShouldLoop;
	return Result;
}

static void SetAnimation(entity* Entity, const anim_clip* Animation)
{
	if (Entity->PlayingAnimation != Animation)
	{
		Entity->PlayingAnimation = Animation;
		Entity->AnimFrameIndex = 0;
		if (Animation)
		{
			Entity->AnimTimer = Animation->FrameDelay;
		}
		else
		{
			Entity->AnimTimer = 0;
		}
	}
}

static void AnimateEntity(entity* Entity)
{
	Assert(Entity->SpriteId != SpriteId_None);

	const anim_clip* Animation = Entity->PlayingAnimation;
	if (!Animation)
	{
		Entity->Sprite->FrameIndex = Entity->Sprite->IdleFrames[Entity->FacingCardinal];
		return;
	}

	Entity->AnimTimer--;
	if (Entity->AnimTimer <= 0)
	{
		if (Animation->ShouldLoop)
		{
			Entity->AnimFrameIndex++;
			Entity->AnimTimer = Animation->FrameDelay;
			if (Entity->AnimFrameIndex >= Animation->Length)
			{
				Entity->AnimFrameIndex = 0;
			}
		}
		else
		{
			if (Entity->AnimFrameIndex < Animation->Length - 1)
			{
				Entity->AnimFrameIndex++;
				Entity->AnimTimer = Animation->FrameDelay;
			}
			else
			{
				Entity->PlayingAnimation = nullptr;
				return;
			}
		}
	}
	u32 FrameIndex = Animation->Frames[Entity->AnimFrameIndex];
	Entity->Sprite->FrameIndex = FrameIndex;
}