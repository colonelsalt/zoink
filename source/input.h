#pragma once

struct input
{
	static constexpr u32 BUFFER_SIZE = 32; // let's keep this a power of 2

	u32 KeysHeldBuffer[BUFFER_SIZE];
	u32 FramesSinceInput = 0;

	inline u32 GetKeys() const
	{
		u32 CurrIndex = g_FrameCount & (BUFFER_SIZE - 1);
		u32 Result = KeysHeldBuffer[CurrIndex];
		return Result;
	}

	inline b32 KeyDown(u32 Key) const
	{
		u32 CurrIndex = g_FrameCount & (BUFFER_SIZE - 1);
		b32 Result = KeysHeldBuffer[CurrIndex] & Key;
		return Result;
	}

	inline b32 KeyPressed(u32 Key) const
	{
		u32 CurrIndex = g_FrameCount & (BUFFER_SIZE - 1);
		u32 PrevIndex = (g_FrameCount - 1) & (BUFFER_SIZE - 1);
		
		b32 HeldLastFrame = KeysHeldBuffer[PrevIndex] & Key;
		b32 HeldThisFrame = KeysHeldBuffer[CurrIndex] & Key;
		b32 Result = HeldThisFrame && !HeldLastFrame;
		return Result;
	}

	inline u32 KeyHeldDownFrames(u32 Key) const
	{
		u32 Result = 0;

		s32 CurrIndex = g_FrameCount & (BUFFER_SIZE - 1);
		for (u32 i = 0; i < BUFFER_SIZE; i++)
		{
			u32 Keys = KeysHeldBuffer[CurrIndex];
			if (!(Keys & Key))
			{
				break;
			}
			CurrIndex--;
			if (CurrIndex < 0)
			{
				CurrIndex = BUFFER_SIZE;
			}
			Result++;
		}
		return Result;
	}
};

static input* g_Input;

inline static void InitInput(ewram_arena* Arena)
{
	g_Input = PushStructZeroed(Arena, input);
}

inline static void BufferInput()
{
	key_poll();

	u32 BufferIndex = g_FrameCount & (input::BUFFER_SIZE - 1);
	u32 KeyState = key_curr_state();
	g_Input->KeysHeldBuffer[BufferIndex] = KeyState;

	if (KeyState)
	{
		g_Input->FramesSinceInput = 0;
	}
	else
	{
		g_Input->FramesSinceInput++;
	}
}



