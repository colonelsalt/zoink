#pragma once

struct cool_string
{
	u8* Data;
	s32 Length;

	u8 operator[](s32 Index)
	{
		Assert(Index >= 0 && Index < Length);
		return Data[Index];
	}
};
