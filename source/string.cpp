#include "string.h"

static void CoolToC(cool_string String, char* OutBuffer)
{
	s32 i;
	for (i = 0; i < String.Length; i++)
	{
		OutBuffer[i] = String[i];
	}
	OutBuffer[i] = 0;
}

// Inclusive on both ends
constexpr cool_string Substring(cool_string String, s32 StartIndex, s32 EndIndex = -1)
{
	Assert(StartIndex >= 0 && StartIndex < String.Length);

	cool_string Result = {};
	Result.Data = String.Data + StartIndex;
	if (EndIndex == -1)
	{
		Result.Length = String.Length - StartIndex;
	}
	else
	{
		Assert(EndIndex > 0 && EndIndex < String.Length && StartIndex <= EndIndex);
		Result.Length = EndIndex - StartIndex + 1;
	}
	return Result;
}

static constexpr s32 IndexOfChar(cool_string String, u8 Char)
{
	for (s32 i = 0; i < String.Length; i++)
	{
		if (String[i] == Char)
		{
			return i;
		}
	}
	return -1;
}

static constexpr s32 LastIndexOfChar(cool_string String, u8 Char)
{
	for (s32 i = String.Length - 1; i >= 0; i--)
	{
		if (String[i] == Char)
		{
			return i;
		}
	}
	return -1;
}

static constexpr s32 IndexOfCString(cool_string String, const char* CString)
{
	s32 Result = -1;
	s32 CIndex = 0;
	for (s32 i = 0; i < String.Length; i++)
	{
		if (String[i] == CString[CIndex])
		{
			if (Result == -1)
			{
				Result = i;
			}

			CIndex++;
			if (!CString[CIndex])
			{
				// Matched the whole substring
				break;
			}
		}
		else
		{
			Result = -1;
			CIndex = 0;
		}
	}
	return Result;
}