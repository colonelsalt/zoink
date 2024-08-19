#pragma once

enum camera_state
{
	CamState_FollowPlayer,
	CamState_Fixed,
	CamState_Lerping
};

struct camera
{
	camera_state State;
	v2 WorldPos;

	v2 LerpStartPos;
	v2 TargetPos;
	fixed tLerp;
	fixed Step;
	u32 TimeLerped;
	u32 TargetLerpFrames;
};