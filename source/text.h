#pragma once

enum text_state_flags : u32
{
	TextState_Inactive        = 0,
	TextState_InitOnVblank    = 1,
	TextState_Printing        = 1 << 1,
	TextState_SeenOneNewline  = 1 << 2,
	TextState_PendingScroll   = 1 << 3,
	TextState_Scrolling       = 1 << 4,
	TextState_FreshPara       = 1 << 5, // i.e. this paragraph has not been scrolled yet
	TextState_PendingNextPara = 1 << 6,
	TextState_PendingEnd      = 1 << 7,
	TextState_HideOnVblank    = 1 << 8,

	TextState_PendingInput    = TextState_PendingScroll | TextState_PendingNextPara | TextState_PendingEnd,
	TextState_CntArrowVisible = TextState_PendingScroll | TextState_PendingNextPara
};

struct win_rect
{
	u32 Left;
	u32 Right;
	u32 Top;
	u32 Bottom;

	inline u32 Width() const
	{
		u32 Result = Right - Left;
		return Result;
	}

	inline u32 Height() const
	{
		u32 Result = Bottom - Top;
		return Result;
	}
};

struct text_window
{
	win_rect Rect; // defines the dimensions of the TTE surface on which the text box is displayed
	iv2 WinMargin; // how much smaller the window (win0) is than the rect
	iv2 TextMargin; // how much smaller the effective writeable text box is than the rect

	inline win_rect GetWindowRect() const
	{
		win_rect Result;
		Result.Left = Rect.Left + WinMargin.X;
		Result.Right = Rect.Right - WinMargin.X;
		Result.Top = Rect.Top + WinMargin.Y;
		Result.Bottom = Rect.Bottom - WinMargin.Y;
		return Result;
	}

	inline win_rect GetTextRect() const
	{
		win_rect Result;
		Result.Left = Rect.Left + TextMargin.X;
		Result.Right = Rect.Right - TextMargin.X;
		Result.Top = Rect.Top + TextMargin.Y;
		Result.Bottom = Rect.Bottom - TextMargin.Y;
		return Result;
	}
};

struct dialogue_render_state
{
	// -- Config-level vars
	fixed PrintSpeed; // chars / frame
	u32 ScrollSpeed;
	u32 LineSize; // i.e. how much to scroll to get to the next line
	// --

	u32 Flags;
	fixed NumPendingChars; // number of new chars that should be printed this frame
	u32 ScrollAmount;
	u32 OldestVisibleCharIndex; // tells us the earliest currently visible line (for scrolling visibility)
	
	// Hacky memoing of newlines to know which char to start scrolling off-screen
	u32 Last3Newlines[3];
	s32 NewlineIndex; // mod 3; index is the *last* most recent newline (increment before store)

	// TODO: Switch to using cool_string?
	const char* String;
	u32 CharIndex; // i.e. index of char to be displayed next

	fixed ArrowOffset;
	fixed ArrowBobSpeed; // pixels / frame
	u32 ArrowMaxOffset;

	// Cache these in case we've rendered text elsewhere on the screen and changed the cursors/margins
	s16 CursorX;
	s16 CursorY;

	inline b32 IsActive() const
	{
		b32 Result = Flags != TextState_Inactive;
		return Result;
	}
};

enum inventory_win_element : u8
{
	InvElem_InventoryList   = 1,
	InvElem_SelectionArrow  = 1 << 1,
	InvElem_PlayerStatus    = 1 << 2,
	InvElem_EnemyStatus		= 1 << 3,
	InvElem_YesNoArrow      = 1 << 4,
};

struct inventory_render_state
{
	s16 ItemStartY;
	
	u8 ElemsPendingRefresh;
};

static void RefreshInventoryDisplay(u8 ElementsToRefresh);
static void BeginTextBox(const char* String);
static b32 IsDialogueBoxActive();
