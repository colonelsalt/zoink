#pragma once

struct ewram_arena
{
	u8* Memory;
	u32 Size;
	u32 Cursor;
};

#define EWRAM_START ((u8*)MEM_EWRAM)
#define EWRAM_END ((u8*)(MEM_EWRAM + EWRAM_SIZE))

inline static u8* PushSize(ewram_arena* Arena, u32 Size)
{
	u8* Result = Arena->Memory + Arena->Cursor;
	Arena->Cursor += Size;
	Assert(Arena->Cursor < Arena->Size && Result < EWRAM_END);
	return Result;
}

inline static u8* PushSizeZeroed(ewram_arena* Arena, u32 Size)
{
	u8* Result = PushSize(Arena, Size);
	memset16(Result, 0, (Size + 1) >> 1); // if we overshoot by one byte, it makes no difference
	return Result;
}

inline static void ResetArena(ewram_arena* Arena)
{
	Arena->Cursor = 0;
}

#define PushArray(Arena, Type, Count) ((Type*)PushSize(Arena, sizeof(Type) * Count))
#define PushArrayZeroed(Arena, Type, Count) ((Type*)PushSizeZeroed(Arena, sizeof(Type) * Count))
#define PushStruct(Arena, Type) ((Type*)PushSize(Arena, sizeof(Type)))
#define PushStructZeroed(Arena, Type) (((Type*)PushSizeZeroed(Arena, sizeof(Type))))