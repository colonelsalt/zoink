#include "camera.h"
#include <tonc.h>

#include "util.h"
#include "level.h"

static camera g_Camera;

static void InitCamera()
{
	g_Camera = {};
}

static v2 GetCameraCentredPos(entity* Entity)
{
	v2 Result = GetCentre(Entity);

	v2 ScrollLimit = { g_Level->TileWidth * 8 - SCREEN_WIDTH, g_Level->TileHeight * 8 - SCREEN_HEIGHT };
	Result.X = FClamp(Result.X - SCREEN_WIDTH / 2, 0, ScrollLimit.X);
	Result.Y = FClamp(Result.Y - SCREEN_HEIGHT / 2, 0, ScrollLimit.Y);

	return Result;
}

static void UpdateCamera(entity* Player)
{
	if (g_Level && g_Level->IsCameraLocked)
	{
		// Keep camera at centre of map - slight hack for 32x32 maps
		g_Camera.State = CamState_Fixed;
		Assert(!g_Level->IsBigMap && g_Level->TileWidth == 32 && g_Level->TileHeight == 32);
		g_Camera.WorldPos = v2 { 0, 4 };
		return;
	}

	switch (g_Camera.State)
	{
		case CamState_FollowPlayer:
		{
			Assert(Player);
			g_Camera.WorldPos = GetCameraCentredPos(Player);
		} break;

		case CamState_Lerping:
		{
			g_Camera.TimeLerped++;
			g_Camera.WorldPos = Lerp(g_Camera.LerpStartPos, g_Camera.TargetPos, g_Camera.tLerp);
			g_Camera.tLerp += g_Camera.Step;
			if (g_Camera.tLerp >= 1)
			{
				g_Camera.WorldPos = g_Camera.TargetPos;
				g_Camera.State = CamState_Fixed;
			}
		} break;

		case CamState_Fixed:
			// Do nothing lol
			break;
	}
}

static void LerpCamera(v2 Target, u32 NumFrames)
{
	g_Camera.LerpStartPos = g_Camera.WorldPos;
	g_Camera.TargetPos = Target;
	g_Camera.State = CamState_Lerping;
	g_Camera.TimeLerped = 0;
	g_Camera.tLerp = 0;
	g_Camera.TargetLerpFrames = NumFrames;

	// Step = 1.0 / NumFrames
	g_Camera.Step.RawValue = lu_div(NumFrames) >> 8;
}

