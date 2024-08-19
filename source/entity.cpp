#include "entity.h"

static b32 RectIntersect(rect* A, rect* B)
{
	b32 Result = !(A->Pos.X + A->Dimensions.Width < B->Pos.X ||
				 B->Pos.X + B->Dimensions.Width < A->Pos.X ||
				 A->Pos.Y + A->Dimensions.Height < B->Pos.Y ||
				 B->Pos.Y + B->Dimensions.Height < A->Pos.Y);
	return Result;
}

static b32 SquareIntersect(v2 Square1Pos, v2 Square2Pos, u32 Size)
{
	b32 Result = !(Square1Pos.X + Size < Square2Pos.X ||
				 Square2Pos.X + Size < Square1Pos.X ||
				 Square1Pos.Y + Size < Square2Pos.Y ||
				 Square2Pos.Y + Size < Square1Pos.Y);
	return Result;
}

// https://stackoverflow.com/questions/4978323/how-to-calculate-distance-between-two-rectangles-context-a-game-in-lua
static v2 ShortestVec(rect* A, rect* B)
{
	b32 Left = B->Pos.X + B->Dimensions.Width < A->Pos.X;
	b32 Right = A->Pos.X + A->Dimensions.X < B->Pos.X;
	b32 Above = B->Pos.Y + B->Dimensions.Y < A->Pos.Y;
	b32 Below = A->Pos.Y + A->Dimensions.Y < B->Pos.Y;

	v2 Result;
	if (Above && Left)
	{
		Result = (B->Pos + B->Dimensions) - A->Pos;
	}
	else if (Left && Below)
	{
		Result = (B->Pos + v2 { B->Dimensions.Width, 0 }) - (A->Pos + v2 { 0, A->Dimensions.Height });
	}
	else if (Right && Below)
	{
		Result = B->Pos - (A->Pos + A->Dimensions);
	}
	else if (Above && Right)
	{
		Result = (B->Pos + v2 { 0, B->Dimensions.Height }) - (A->Pos + v2 { A->Dimensions.Width, 0 });
	}
	else if (Left)
	{
		Result = (B->Pos + v2 { B->Dimensions.Width, 0 }) - A->Pos;
	}
	else if (Right)
	{
		Result = B->Pos - (A->Pos + v2 { A->Dimensions.Width, 0 });
	}
	else if (Below)
	{
		Result = B->Pos - (A->Pos + v2 { 0, A->Dimensions.Height });
	}
	else if (Above)
	{
		Result = (B->Pos + v2 { 0, B->Dimensions.Height }) - A->Pos;
	}
	else
	{
		Assert(false); // Rects intersect - probably something is up
	}

	return Result;
}

static cardinal DominantDirectionFromVector(v2 Movement)
{
	// Cannot determine cardinal direction from zero vector
	Assert(!(Movement.X == 0 && Movement.Y == 0));
	if (Abs(Movement.X) > Abs(Movement.Y))
	{
		if (Movement.X > 0)
		{
			return Cardinal_Right;
		}
		return Cardinal_Left;
	}
	if (Movement.Y > 0)
	{
		return Cardinal_Down;
	}
	return Cardinal_Up;
}

static u8 DirectionsFromVector(v2 Movement)
{
	u8 Result = Dir_Undetermined;

	if (Movement.X > 0)
	{
		Result |= Dir_Right;
	}
	else if (Movement.X < 0)
	{
		Result |= Dir_Left;
	}

	if (Movement.Y > 0)
	{
		Result |= Dir_Down;
	}
	else if (Movement.Y < 0)
	{
		Result |= Dir_Up;
	}

	return Result;
}

static constexpr v2 GetCentre(entity* Entity)
{
	v2 Result;
	Result.X = Entity->WorldPos.X + (fixed(Entity->Width) >> 1);
	Result.Y = Entity->WorldPos.Y + (fixed(Entity->Height) >> 1);
	return Result;
}

static iv2 WorldToScreenPos(entity* Entity)
{
	// Convert to integer vector first to prevent rounding inconsistensies between screen and world pos.
	iv2 Result = iv2(Entity->WorldPos) - iv2(g_Camera.WorldPos);
	return Result;
}

static void TakeDamage(entity* Entity, s8 Amount)
{
	Assert(Entity->Type == Entity_Player || Entity->Type == Entity_Enemy);

	if (Entity->Health - Amount <= 0)
	{
		Entity->Health = 0;
	}
	else
	{
		Entity->Health -= Amount;
	}

	inventory_win_element WinElem = Entity->Type == Entity_Player ? InvElem_PlayerStatus : InvElem_EnemyStatus;
	RefreshInventoryDisplay(WinElem);
}

static v2 ShortestVec(entity* A, entity* B)
{
	rect RectA;
	RectA.Pos = A->WorldPos;
	RectA.Dimensions = iv2 { A->Width, A->Height };

	rect RectB;
	RectB.Pos = B->WorldPos;
	RectB.Dimensions = iv2 { B->Width, B->Height };

	v2 Result = ShortestVec(&RectA, &RectB);
	return Result;
}

static void LerpMoveEntity(entity* Entity, v2 TargetPos, u32 NumFrames)
{
	lerp_data LerpData = {};
	LerpData.StartPos = Entity->WorldPos;
	LerpData.TargetPos = TargetPos;
	LerpData.Step.RawValue = lu_div(NumFrames) >> 8;
	LerpData.t = 0;
	LerpData.TargetNumFrames = NumFrames;
	LerpData.FramesLerped = 0;

	Entity->LerpData = LerpData;
	Entity->StateFlags |= EFlag_ScriptLerpMovement;
}

static void UpdateEntity(entity* Entity)
{
	Entity->ScreenPos = WorldToScreenPos(Entity);

	if (GetFlag(Entity->VisibilityFlag) ||
		Entity->ScreenPos.X < -Entity->Width || Entity->ScreenPos.X > SCREEN_WIDTH ||
		Entity->ScreenPos.Y < -Entity->Height || Entity->ScreenPos.Y > SCREEN_HEIGHT)
	{
		Entity->StateFlags &= ~EFlag_Visible;
		if (Entity->SpriteId != SpriteId_None)
		{
			obj_hide(&Entity->Sprite->OamEntry->Attributes);
		}
	}
	else
	{
		Entity->StateFlags |= EFlag_Visible;
		if (Entity->SpriteId != SpriteId_None)
		{
			obj_unhide(&Entity->Sprite->OamEntry->Attributes, 0);
		}
	}
	if (Entity->SpriteId == SpriteId_None)
	{
		return;
	}


	if (Entity->StateFlags & EFlag_ScriptMovement)
	{
		b32 MoveComplete = false;
		v2 MoveDelta;
		if (Entity->StateFlags & EFlag_ScriptStraightLineMovement)
		{
			// TODO: This is super piggy code - one day I should get some A* up in this bitch
			if (Entity->ScriptMovementVector.X > Entity->ScriptMovementVector.Y)
			{
				MoveDelta = v2 { 1, 0 };
				if (Entity->StraightLineMovedSoFar.X >= Entity->ScriptMovementVector.X)
				{
					MoveComplete = true;
				}
			}
			else
			{
				MoveDelta = v2 { 0, 1 };
				if (Entity->StraightLineMovedSoFar.Y >= Entity->ScriptMovementVector.Y)
				{
					MoveComplete = true;
				}
			}
			Entity->StraightLineMovedSoFar += MoveDelta;
			Entity->WorldPos += MoveDelta;
		}
		if (Entity->StateFlags & EFlag_ScriptLerpMovement)
		{
			Entity->LerpData.t += Entity->LerpData.Step;
			Entity->LerpData.FramesLerped++;

			v2 NewPos = Lerp(Entity->LerpData.StartPos, Entity->LerpData.TargetPos, Entity->LerpData.t);
			if (Entity->LerpData.t >= 1)
			{
				MoveComplete = true;
				Entity->WorldPos = NewPos;
			}

			MoveDelta = NewPos - Entity->WorldPos;
			Entity->WorldPos = NewPos;

		}
		if (Entity->StateFlags & EFlag_ScriptManualMovement)
		{
			// TODO
		}

		if (MoveComplete)
		{
			Entity->StateFlags &= ~EFlag_ScriptMovement;
			if (Entity->Type == Entity_Enemy)
			{
				SetAnimation(Entity, &anim::enemy::Loiter);
			}
			else
			{
				SetAnimation(Entity, nullptr);
			}
		}
		else
		{
			Entity->PrevFacingCardinal = Entity->FacingCardinal;
			Entity->FacingCardinal = DominantDirectionFromVector(MoveDelta);

			if (Entity->Type == Entity_Enemy)
			{
				SetAnimation(Entity, &anim::enemy::WalkDown);
			}
			else
			{
				// Oink oink... if we're in battle, player always moves with 'walking up' animation
				if (g_GameState->StateFlags & GameState_InBattle)
				{
					SetAnimation(Entity, s_DirectionalAnims[Cardinal_Up].WalkAnim);
					Entity->FacingCardinal = Cardinal_Up;
				}
				else
				{
					SetAnimation(Entity, s_DirectionalAnims[Entity->FacingCardinal].WalkAnim);
				}
			}

		}
	}

	if (Entity->Type == Entity_Reflection)
	{
		ChangeOamEntryY(Entity->Sprite->OamEntry, 0);
	}
	else
	{
		s32 EntityY = Entity->WorldPos.Y.WholePart;
		if (Entity->SpriteId == SpriteId_Item)
		{
			EntityY -= 16;
		}
		ChangeOamEntryY(Entity->Sprite->OamEntry, EntityY);
	}

	obj_set_pos(&Entity->Sprite->OamEntry->Attributes, Entity->ScreenPos.X, Entity->ScreenPos.Y);
	AnimateEntity(Entity);
}