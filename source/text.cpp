#include "text.h"

#include <tonc.h>

#include "battle.h"
#include "memory.h"
#include "util.h"
#include "asset_load.h"
#include "dlgbox.cpp"

// Setup here adapted from tonc-code tte demo chr4 / 'text in a box' example:
// https://github.com/yoyz/gba/blob/master/tonclib/code/adv/tte_demo/source/main.c#L211

static void ResetCursor()
{
	TTC* TteContext = tte_get_context();
	TteContext->cursorX = TteContext->marginLeft;
	TteContext->cursorY = TteContext->marginTop;
}

// This really kind of needs to live in IWRAM for speed purposes.
static TSurface s_SrcSurface;

static constexpr const text_window s_DialogueWindow =
{
	.Rect = { .Left = 0, .Right = 240, .Top = 120, .Bottom = 160 },
	.WinMargin = { 7, 5 }, // window is 7x5 pixels in from the surface, either side
	.TextMargin = { 16, 7 } // text has another 9x2px margin from window
};

static constexpr const text_window s_InventoryWindow =
{
	.Rect = { .Left = 168, .Right = 240, .Top = 8, .Bottom = 112 },
	.WinMargin = { 7, 5 },
	.TextMargin = { 15, 6 }
};

static constexpr const text_window s_PlayerStatusWindow = 
{
	.Rect = { .Left = 0, .Right = 128, .Top = 96, .Bottom = 120 },
	.WinMargin = { 7, 5 },
	.TextMargin = { 8, 6 }
};

static constexpr const text_window s_ConfirmationWindow =
{
	.Rect = { .Left = 112, .Right = 168, .Top = 8, .Bottom = 72 },
	.WinMargin = { 7, 5 },
	.TextMargin = { 8, 6 }
};

static constexpr const text_window s_EnemyStatusWindow = 
{
	.Rect = { .Left = 0, .Right = 112, .Top = 0, .Bottom = 24 },
	.WinMargin = { 7, 5 },
	.TextMargin = { 8, 6 }
};

static dialogue_render_state* s_DiagState;
static inventory_render_state* s_InvState;

static void InitText(ewram_arena* Arena)
{
	// Init for text
	tte_init_chr4c(0,							// BG number (0 always reserved for text).
				   BG_CBB(2) | BG_SBB(31),		// BG control. (piggy, but CBBs 2-3 always reserved for text)
				   0xF000,						// Screen-entry offset
				   bytes2word(13, 14, 15, 0),	// Colour attributes: ink, shadow, paper, 'special'
				   CLR_BLACK,					// Ink colour
				   &verdana9_b4Font,			// Verdana 9, with shade.
				   (fnDrawg)chr4c_drawg_b4cts_fast);	// b4cts renderer, asm version
	//tte_init_con();

	// Copy text box and its palette.
	srf_init(&s_SrcSurface,
			 SRF_CHR4C,
			 grit::dlgbox::Tiles,
			 240, 160,
			 4,
			 (u16*)grit::dlgbox::Pal); // this ends up in the last BG pal slot (15)

	s_InvState = PushStructZeroed(Arena, inventory_render_state);
	s_InvState->ItemStartY = 16;

	s_DiagState = PushStructZeroed(Arena, dialogue_render_state);
	s_DiagState->PrintSpeed = fixed(1.0);
	s_DiagState->ScrollSpeed = 1;
	s_DiagState->LineSize = 12;

	// If we soft-reset, we may still have lingering tiles/SEs from previous run, so erase them
	u16 PaperColour = tte_get_paper();
	tte_set_paper(0);
	tte_erase_screen();
	tte_set_paper(PaperColour);
}

static b32 IsDialogueBoxActive()
{
	Assert(s_DiagState);
	b32 Result = s_DiagState->IsActive();
	return Result;
}

static void Render_BlitWindow(const text_window* Win)
{
	schr4c_blit(tte_get_surface(),
				Win->Rect.Left, Win->Rect.Top,
				Win->Rect.Width(), Win->Rect.Height(),
				&s_SrcSurface,
				Win->Rect.Left, Win->Rect.Top);
}

static void Render_HideWindow(const text_window* Win)
{
	u16 PaperColour = tte_get_paper();
	tte_set_paper(0);
	tte_erase_rect(Win->Rect.Left, Win->Rect.Top, Win->Rect.Right, Win->Rect.Bottom);
	tte_set_paper(PaperColour);
}

static void Render_ClearDialogueBox()
{
	Render_BlitWindow(&s_DialogueWindow);

	// Erase the bobbing arrow for when we first render the textbox
	win_rect WindowRext = s_DialogueWindow.GetWindowRect();
	win_rect TextRext = s_DialogueWindow.GetTextRect();
	tte_erase_rect(TextRext.Right, TextRext.Bottom - 8, WindowRext.Right, TextRext.Bottom);
}

static void BeginTextBox(const char* String)
{
	s_DiagState->ArrowOffset = 0;
	s_DiagState->ArrowBobSpeed = fixed(0.1);
	s_DiagState->ArrowMaxOffset = 3;
	s_DiagState->NumPendingChars = 0;
	s_DiagState->ScrollAmount = 0;
	s_DiagState->String = String;
	s_DiagState->CharIndex = 0;
	s_DiagState->OldestVisibleCharIndex = 0;
	s_DiagState->Flags = TextState_InitOnVblank;
	s_DiagState->NewlineIndex = 0;
}

static void Render_BeginTextBox()
{
	srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);

	win_rect TextRect = s_DialogueWindow.GetTextRect();
	tte_set_margins(TextRect.Left, TextRect.Top, TextRect.Right, TextRect.Bottom);
	s_DiagState->CursorX = TextRect.Left;
	s_DiagState->CursorY = TextRect.Top;

	Render_ClearDialogueBox();
	ResetCursor();
	
	s_DiagState->Flags = TextState_Printing | TextState_FreshPara;
}

static void HideTextBox()
{
	s_DiagState->Flags = TextState_HideOnVblank;
}

static void Render_HideTextBox()
{
	Render_HideWindow(&s_DialogueWindow);
	s_DiagState->Flags = TextState_Inactive;
}

// Adapted from `tte_write()` function from libtonc source:
// https://github.com/devkitPro/libtonc/blob/master/src/tte/tte_main.c#L576
static u32 Render_PrintNextChar(u32 CharIndex, b32 ShouldMutateState = true)
{
	char* StrPtr = (char*)(s_DiagState->String + CharIndex);
	u32 CurrentChar = StrPtr[0];
	StrPtr++;

	TTC* TteContext = tte_get_context();
	switch (CurrentChar)
	{
		// --- Newline/carriage return ---
		case '\r':
		{
			if (StrPtr[0] == '\n')	// deal with CRLF pair
			{
				StrPtr++;
			}
		} // FALLTHRU
		case '\n':
		{
			TteContext->cursorY += TteContext->font->charH;
			TteContext->cursorX = TteContext->marginLeft;

			if (ShouldMutateState)
			{
				s_DiagState->NewlineIndex++;
				if (s_DiagState->NewlineIndex == countof(s_DiagState->Last3Newlines))
				{
					// Poor man's modulo
					s_DiagState->NewlineIndex = 0;
				}
				s_DiagState->Last3Newlines[s_DiagState->NewlineIndex] = CharIndex;

				if (s_DiagState->Flags & TextState_SeenOneNewline)
				{
					s_DiagState->Flags |= TextState_PendingScroll;
				}
				else
				{
					s_DiagState->Flags |= TextState_SeenOneNewline;
				}
			}
		} break;
			
		// --- Tab ---
		case '\t':
		{
			TteContext->cursorX = (TteContext->cursorX / TTE_TAB_WIDTH + 1) * TTE_TAB_WIDTH;
		} break;

		// --- Normal char ---
		default:
		{
			// Command sequence
			if (CurrentChar == '#')
			{
				if (StrPtr[0] == '{')
				{
					StrPtr = tte_cmd_default(StrPtr + 1);
				}
				else if (StrPtr[0] == 'p')
				{
					StrPtr++;
					if (ShouldMutateState)
					{
						s_DiagState->Flags &= ~TextState_SeenOneNewline;
						s_DiagState->Flags |= TextState_PendingNextPara;
					}
				}
				break;
			}
			else if (CurrentChar == '\\' && StrPtr[0] == '#')
			{
				// Escaped command: skip '\\' and print '#'
				CurrentChar = *StrPtr++;
			}
			// Check for UTF8 code
			else if (CurrentChar >= 0x80)
			{
				//CurrentChar = utf8_decode_char(StrPtr - 1, &StrPtr);
				Assert(false); // Not handling UTF8 for now
			}

			// Get glyph index and call renderer
			TFont* Font = TteContext->font;
			u32 GlyphId = CurrentChar - Font->charOffset;
			if (TteContext->charLut)
			{
				GlyphId = TteContext->charLut[GlyphId];
			}

			// Character wrap
			u32 CharWidth = Font->widths ? Font->widths[GlyphId] : Font->charW;
			if (TteContext->cursorX + CharWidth > TteContext->marginRight)
			{
				TteContext->cursorY += Font->charH;
				TteContext->cursorX = TteContext->marginLeft;
			}

			// Draw and update position
			TteContext->drawgProc(GlyphId);
			TteContext->cursorX += CharWidth;
		};
	}

	if (!*StrPtr && ShouldMutateState)
	{
		// Reached end of string
		s_DiagState->Flags = TextState_PendingEnd;
	}

	u32 NextCharIndex = StrPtr - s_DiagState->String;
	return NextCharIndex;
}

static void Render_DrawPlayerStatus(entity* Player);

static void Render_UpdateDialogue(entity* Player)
{
	if (s_DiagState->Flags == TextState_Inactive)
	{
		return;
	}

	if (s_DiagState->Flags & TextState_InitOnVblank)
	{
		Render_BeginTextBox();
	}
	else if (s_DiagState->Flags & TextState_HideOnVblank)
	{
		Render_HideTextBox();
		return;
	}

	TTC* TteContext = tte_get_context();
	TteContext->cursorX = s_DiagState->CursorX;
	TteContext->cursorY = s_DiagState->CursorY;
	win_rect TextRect = s_DialogueWindow.GetTextRect();
	tte_set_margins(TextRect.Left, TextRect.Top, TextRect.Right, TextRect.Bottom);

	if (s_DiagState->Flags & TextState_Scrolling)
	{
		s_DiagState->ScrollAmount += s_DiagState->ScrollSpeed;
		if (s_DiagState->ScrollAmount >= s_DiagState->LineSize)
		{
			s_DiagState->ScrollAmount = s_DiagState->LineSize;
			s_DiagState->Flags &= ~TextState_Scrolling;
		}
	
		TteContext->cursorY = TteContext->marginTop - s_DiagState->ScrollAmount;
		s16 CursorTop = TteContext->cursorY;

		// Re-print the 2 currently visible lines each frame while we scroll
		Render_ClearDialogueBox();
		u32 NextCharIndex = s_DiagState->OldestVisibleCharIndex;
		while (NextCharIndex < s_DiagState->CharIndex)
		{
			NextCharIndex = Render_PrintNextChar(NextCharIndex, false);
		}

		if (!(g_GameState->StateFlags & GameState_EndSequence))
		{
			// This is kind of gross and probably inefficient, but it does the job for now...
			// Temporarily set the 'paper' (erase) colour to transparent
			u16 PaperColour = tte_get_paper();
			tte_set_paper(0);
			tte_erase_rect(TteContext->marginLeft, CursorTop, TteContext->marginRight, s_DialogueWindow.Rect.Top);
			tte_set_paper(PaperColour);

			// Re-blit the part of the text box that was just written over with partially scrolled-off text...
			schr4c_blit(tte_get_surface(),
						s_DialogueWindow.Rect.Left, s_DialogueWindow.Rect.Top,
						s_DialogueWindow.Rect.Width(), s_DialogueWindow.TextMargin.Y, // just the text margin's worth...
						&s_SrcSurface,
						s_DialogueWindow.Rect.Left, s_DialogueWindow.Rect.Top);

			// Oink oink... and if the status window is open, re-draw it, because it has been overwritten...
			if (g_GameState->OpenMenus & Menu_PlayerStatus)
			{
				s16 CursX = TteContext->cursorX;
				s16 CursY = TteContext->cursorY;
				Render_DrawPlayerStatus(Player);
				TteContext->cursorX = CursX;
				TteContext->cursorY = CursY;
			}
		}

	}
	else if (s_DiagState->Flags & TextState_PendingInput)
	{
		if (g_Input->KeyPressed(KEY_A) || g_Input->KeyPressed(KEY_B))
		{
			if (s_DiagState->Flags & TextState_PendingEnd)
			{
				Render_HideTextBox();
				return;
			}
			else
			{
				mmEffect(SFX_INTERACT);
				if (s_DiagState->Flags & TextState_PendingScroll)
				{
					s_DiagState->Flags &= ~TextState_PendingScroll;
					s_DiagState->Flags |= TextState_Scrolling;
					if (s_DiagState->Flags & TextState_FreshPara)
					{
						// If it's a fresh paragraph, the last visible char has already been set to para start
						s_DiagState->Flags &= ~TextState_FreshPara;
					}
					else
					{
						// Finding the third most recent newline - the char after it is the oldest visible
						s32 NewlineIndex = s_DiagState->NewlineIndex - 2;
						if (NewlineIndex < 0)
						{
							NewlineIndex += countof(s_DiagState->Last3Newlines);
						}

						s_DiagState->OldestVisibleCharIndex = s_DiagState->Last3Newlines[NewlineIndex] + 1;
					}
					s_DiagState->ScrollAmount = 0;
					ResetCursor();
				}
				else if (s_DiagState->Flags & TextState_PendingNextPara)
				{
					Render_ClearDialogueBox();
					ResetCursor();
					s_DiagState->OldestVisibleCharIndex = s_DiagState->CharIndex;
					s_DiagState->Flags &= ~TextState_PendingNextPara;
					s_DiagState->Flags |= TextState_FreshPara;
					s_DiagState->NumPendingChars = 0;
				}
			}
		}
	}
	else if (s_DiagState->Flags & TextState_Printing)
	{
		s_DiagState->NumPendingChars += s_DiagState->PrintSpeed;
		while (s_DiagState->NumPendingChars.WholePart > 0 && !(s_DiagState->Flags & TextState_PendingInput))
		{
			s_DiagState->CharIndex = Render_PrintNextChar(s_DiagState->CharIndex);
			s_DiagState->NumPendingChars.WholePart--;
		}
	}

	win_rect WindowRext = s_DialogueWindow.GetWindowRect();
	win_rect TextRext = s_DialogueWindow.GetTextRect();
	if (s_DiagState->Flags & TextState_CntArrowVisible)
	{
		schr4c_blit(tte_get_surface(),
					TextRext.Right, TextRext.Bottom - 8,
					8, 8,
					&s_SrcSurface,
					224, 144 + s_DiagState->ArrowOffset.WholePart);

		s_DiagState->ArrowOffset += s_DiagState->ArrowBobSpeed;
		if (s_DiagState->ArrowOffset > s_DiagState->ArrowMaxOffset || s_DiagState->ArrowOffset <= 0)
		{
			s_DiagState->ArrowOffset = FClamp(s_DiagState->ArrowOffset, 0, fixed(s_DiagState->ArrowMaxOffset, 0));
			s_DiagState->ArrowBobSpeed = -s_DiagState->ArrowBobSpeed;
		}
	}
	else
	{
		s_DiagState->ArrowOffset = 0;
		tte_erase_rect(TextRext.Right, TextRext.Bottom - 8, WindowRext.Right, TextRext.Bottom);
	}
	s_DiagState->CursorX = TteContext->cursorX;
	s_DiagState->CursorY = TteContext->cursorY;
}

static void Render_DrawYesNoArrow()
{
	win_rect WinRect = s_ConfirmationWindow.GetWindowRect();
	tte_erase_rect(WinRect.Left, WinRect.Top + 24, WinRect.Left + 8, WinRect.Bottom);
	s16 ArrowY = s_InvState->ItemStartY + 12 * (g_GameState->Inventory->YesNoIndex + 1);

	schr4c_blit(tte_get_surface(),
				WinRect.Left, WinRect.Top + ArrowY,
				8, 8,
				&s_SrcSurface, 
				232, 0);
}

static void Render_DrawConfirmationWindow(CSTR Verb, CSTR Noun, CSTR Option1, CSTR Option2)
{
	srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);

	Render_BlitWindow(&s_ConfirmationWindow);
	win_rect TextRect = s_ConfirmationWindow.GetTextRect();
	tte_set_margins(TextRect.Left, TextRect.Top, TextRect.Right, TextRect.Bottom);
	ResetCursor();

	TTC* TteContext = tte_get_context();
	TteContext->cursorX += (TextRect.Width() / 2) - 12;
	tte_write(Verb);
	tte_write("\n");
	tte_write(Noun);
	tte_write("?\n");

	TteContext->cursorX += 8;
	tte_write(Option1);
	tte_write("\n");
	
	TteContext->cursorX += 8;
	tte_write(Option2);

	Render_DrawYesNoArrow();
}

static void Render_DrawItemSelectionArrow()
{
	win_rect WinRect = s_InventoryWindow.GetWindowRect();
	tte_erase_rect(WinRect.Left, WinRect.Top, WinRect.Left + 8, WinRect.Bottom);

	b32 ShouldRenderArrow = false;
	if (g_GameState->OpenMenus & Menu_VendingMachine)
	{
		ShouldRenderArrow = true;
	}
	else
	{
		ShouldRenderArrow = g_GameState->Inventory->NumOccupiedSlots > 0;
	}

	if (ShouldRenderArrow)
	{
		s16 ArrowY = s_InvState->ItemStartY + 12 * g_GameState->Inventory->SelectedSlotIndex;

		schr4c_blit(tte_get_surface(),
					WinRect.Left, WinRect.Top + ArrowY,
					8, 8,
					&s_SrcSurface, 
					232, 0);
	}
}

static void Render_DrawInventory()
{
	Render_BlitWindow(&s_InventoryWindow);

	win_rect TextRect = s_InventoryWindow.GetTextRect();
	tte_set_margins(TextRect.Left, TextRect.Top, TextRect.Right + 8, TextRect.Bottom);

	ResetCursor();
	u16 InkColour = tte_get_ink();
	u16 ShadowColour = tte_get_shadow();
	// Temporarily write in blue
	tte_set_ink(5);
	tte_set_shadow(6);

	TTC* TteContext = tte_get_context();
	TteContext->cursorX += (TextRect.Width() / 2) - 12;


	CSTR Noun = (g_GameState->OpenMenus & Menu_VendingMachine) ? "VEND\n" : "PAK\n";
	tte_write(Noun);
	tte_set_ink(InkColour);
	tte_set_shadow(ShadowColour);
	TteContext->cursorY += 1;

	if (g_GameState->OpenMenus & Menu_VendingMachine)
	{
		for (u32 i = 0; i < countof(s_VendingMachineItems); i++)
		{
			const vending_item* Item = s_VendingMachineItems + i;
			tte_write(GetItemString(Item->Type));
			
			tte_set_ink(3);
			tte_set_shadow(4);
			tte_write(" $");
			const char* CostStr = INT_STR_LUT.Get(Item->Cost);
			tte_write(CostStr);
			tte_write("\n");

			tte_set_ink(InkColour);
			tte_set_shadow(ShadowColour);
		}
	}
	else
	{
		item_slot* CurrItem = g_GameState->Inventory->ItemHead;
		while (CurrItem)
		{
			const char* ItemName = GetItemString(CurrItem->Type);
			tte_write(ItemName);
			tte_write(" ×");
			const char* QuantityStr = INT_STR_LUT.Get(CurrItem->Quantity);
			tte_write(QuantityStr);

			tte_write("\n");
			CurrItem = CurrItem->Next;
		}
	}

	Render_DrawItemSelectionArrow();
}

static void RefreshInventoryDisplay(u8 ElementsToRefresh)
{
	s_InvState->ElemsPendingRefresh |= ElementsToRefresh;
}

static void Render_DrawPlayerStatus(entity* Player)
{
	srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);

	Render_BlitWindow(&s_PlayerStatusWindow);
	win_rect TextRect = s_PlayerStatusWindow.GetTextRect();
	tte_set_margins(TextRect.Left, TextRect.Top, TextRect.Right, TextRect.Bottom);
	ResetCursor();

	tte_write("Jack   ");

	u16 InkColour = tte_get_ink();
	u16 ShadowColour = tte_get_shadow();
	tte_set_ink(1);
	tte_set_shadow(9);

	tte_write("HP:");
	CSTR HealthStr = INT_STR_LUT.Get(Player->Health);
	tte_write(HealthStr);

	tte_set_ink(3);
	tte_set_shadow(4);

	tte_write("    $");
	CSTR MoneyStr = INT_STR_LUT.Get(g_GameState->Inventory->Money);
	tte_write(MoneyStr);

	tte_set_ink(InkColour);
	tte_set_shadow(ShadowColour);
}

static void Render_DrawEnemyStatus(entity* Enemy)
{
	srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);

	Render_BlitWindow(&s_EnemyStatusWindow);
	win_rect TextRect = s_EnemyStatusWindow.GetTextRect();
	tte_set_margins(TextRect.Left, TextRect.Top, TextRect.Right, TextRect.Bottom);
	ResetCursor();

	tte_write(GetEnemyName());
	tte_write("   ");

	u16 InkColour = tte_get_ink();
	u16 ShadowColour = tte_get_shadow();
	tte_set_ink(1);
	tte_set_shadow(9);

	tte_write("HP:");
	CSTR HealthStr = INT_STR_LUT.Get(Enemy->Health);
	tte_write(HealthStr);

	tte_set_ink(InkColour);
	tte_set_shadow(ShadowColour);
}

static void Render_UpdateInventoryText(entity* Player)
{
	if (MenuFlipped(Menu_Inventory) || MenuFlipped(Menu_VendingMachine))
	{
		if (g_GameState->OpenMenus & (Menu_Inventory | Menu_VendingMachine))
		{
			srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);
			Render_DrawInventory();
		}
		else
		{
			Render_HideWindow(&s_InventoryWindow);
		}
	}
	if (MenuFlipped(Menu_PlayerStatus))
	{
		if (g_GameState->OpenMenus & Menu_PlayerStatus)
		{
			Render_DrawPlayerStatus(Player);
		}
		else
		{
			Render_HideWindow(&s_PlayerStatusWindow);
		}
	}

	if (MenuFlipped(Menu_ConfirmWindow))
	{
		if (g_GameState->OpenMenus & Menu_ConfirmWindow)
		{
			// Both can't be open at once, since they use the same text window
			Assert(!(g_GameState->OpenMenus & Menu_CommandWindow));

			const char* Verb;
			const char* Noun;
			if (g_GameState->OpenMenus & Menu_VendingMachine)
			{
				Verb = "Buy";
				const vending_item* Item = s_VendingMachineItems + g_GameState->Inventory->SelectedSlotIndex;
				Noun = GetItemString(Item->Type);
			}
			else
			{
				Verb = "Exec";
				item_slot* TargetItem = GetItemSlot(g_GameState->Inventory->SelectedSlotIndex);
				Noun = GetItemString(TargetItem->Type);

			}
			Render_DrawConfirmationWindow(Verb, Noun, "YES", "NO");
		}
		else
		{
			Render_HideWindow(&s_ConfirmationWindow);
		}
	}
	if (MenuFlipped(Menu_CommandWindow))
	{
		if (g_GameState->OpenMenus & Menu_CommandWindow)
		{
			Assert(!(g_GameState->OpenMenus & Menu_ConfirmWindow));

			srf_pal_copy(tte_get_surface(), &s_SrcSurface, 16);
			Render_DrawConfirmationWindow("What", "   do", "FIGHT", "PAK");
		}
		else
		{
			Render_HideWindow(&s_ConfirmationWindow);
		}
	}
	if (MenuFlipped(Menu_EnemyStatus))
	{
		if (g_GameState->OpenMenus & Menu_EnemyStatus)
		{
			Render_DrawEnemyStatus(GetEnemy());
		}
		else
		{
			Render_HideWindow(&s_EnemyStatusWindow);
		}
	}


	if (g_GameState->OpenMenus & (Menu_Inventory | Menu_VendingMachine))
	{
		if (s_InvState->ElemsPendingRefresh & InvElem_InventoryList)
		{
			s_InvState->ElemsPendingRefresh &= ~(InvElem_InventoryList | InvElem_SelectionArrow);
			Render_DrawInventory();
		}
		if (s_InvState->ElemsPendingRefresh & InvElem_SelectionArrow)
		{
			s_InvState->ElemsPendingRefresh &= ~InvElem_SelectionArrow;
			Render_DrawItemSelectionArrow();
		}
	}

	if (g_GameState->OpenMenus & Menu_PlayerStatus)
	{
		if (s_InvState->ElemsPendingRefresh & InvElem_PlayerStatus)
		{
			s_InvState->ElemsPendingRefresh &= ~InvElem_PlayerStatus;
			Render_DrawPlayerStatus(Player);
		}
	}

	if (g_GameState->OpenMenus & Menu_EnemyStatus)
	{
		if (s_InvState->ElemsPendingRefresh & InvElem_EnemyStatus)
		{
			s_InvState->ElemsPendingRefresh &= ~InvElem_EnemyStatus;
			entity* Enemy = GetEnemy();
			Render_DrawEnemyStatus(Enemy);
		}
	}


	if (g_GameState->OpenMenus & Menu_TwoChoiceWindow)
	{
		if (s_InvState->ElemsPendingRefresh & InvElem_YesNoArrow)
		{
			s_InvState->ElemsPendingRefresh &= ~InvElem_YesNoArrow;
			Render_DrawYesNoArrow();
		}
	}
}