#pragma once

#define STRINGIZE2(X) #X
#define STRINGIZE(X) STRINGIZE2(X)
#define LINE_STRING STRINGIZE(__LINE__)

#define Assert(X) {if (!(X)) ErrorScreen(FileNameFromPath(__FILE__), LINE_STRING);}

#define SizeInBytes(X) (X * sizeof(X))
#define SizeInWords(X) (SizeInBytes(X) / 4)

typedef unsigned int b32;
typedef uint8_t b8;

u32 g_FrameCount = 0;

constexpr const char* FileNameFromPath(const char* PathName)
{
	const char* Result = PathName;
	while (*PathName)
	{
		if (*PathName++ == '/')
		{
			Result = PathName;
		}
	}
	return Result;
}

static void ErrorScreen(const char* FileName, const char* LineNo)
{
	REG_IME = 0;

	RegisterRamReset(RESET_VRAM | RESET_REG_SOUND | RESET_REG);

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;

	tte_init_chr4c_b4_default(0, BG_CBB(0)|BG_SBB(31));
	pal_bg_mem[0] = 0;

	tte_set_pos(8, 8);
	tte_set_margins(8, 8, 232, 152);
	tte_write("ERROR:\n");
	tte_write(FileName);
	tte_write(":");
	tte_write(LineNo);

	while (true)
	{
		asm("nop");
	}

}