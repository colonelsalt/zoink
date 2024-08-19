#include "inventory.h"

#include "text.h"
#include "memory.h"
#include "entity.h"

static inventory* s_Inventory;

static inventory* InitInventory(ewram_arena* Arena)
{
	s_Inventory = PushStructZeroed(Arena, inventory);
	s_Inventory->Arena = Arena;

	s_Inventory->Money = 20;

	return s_Inventory;
}

static void RefreshInventoryDisplay(u8 ElementsToRefresh);

static item_slot* GetItemSlot(u8 Index)
{
	u8 i = 0;
	item_slot* Result = s_Inventory->ItemHead;
	while (Result && i != Index)
	{
		Result = Result->Next;
		i++;
	}
	Assert(Result && i < s_Inventory->NumOccupiedSlots);
	return Result;
}

static b32 HasItem(item_type ItemType)
{
	b32 Result = false;
	item_slot* Current = s_Inventory->ItemHead;
	while (Current)
	{
		if (Current->Type == ItemType)
		{
			Result = true;
			break;
		}
		Current = Current->Next;
	}
	return Result;
}

static void GiveItem(item_type ItemType, u8 Quantity)
{
	item_slot* Prev = nullptr;
	item_slot* Current = s_Inventory->ItemHead;
	while (Current && Current->Type != ItemType)
	{
		Prev = Current;
		Current = Current->Next;
	}
	if (!Current)
	{
		Current = s_Inventory->FirstFreeSlot;
		if (Current)
		{
			s_Inventory->FirstFreeSlot = s_Inventory->FirstFreeSlot->Next;
			*Current = {};
		}
		else
		{
			Current = PushStructZeroed(s_Inventory->Arena, item_slot);
		}

		Current->Type = ItemType;
		if (Prev)
		{
			Prev->Next = Current;
		}
		else
		{
			s_Inventory->ItemHead = Current;
		}

		s_Inventory->NumOccupiedSlots++;
	}
	Current->Quantity += Quantity;

	RefreshInventoryDisplay(InvElem_InventoryList);
}

static void RemoveItem(item_type ItemType, u8 Quantity)
{
	item_slot* Prev = nullptr;
	item_slot* Current = s_Inventory->ItemHead;
	while (Current && Current->Type != ItemType)
	{
		Prev = Current;
		Current = Current->Next;
	}
	if (Current)
	{
		Current->Quantity -= Quantity;
		if (Current->Quantity == 0)
		{
			// Fix links
			if (Prev)
			{
				Prev->Next = Current->Next;
			}
			else
			{
				s_Inventory->ItemHead = Current->Next;
			}

			Current->Next = s_Inventory->FirstFreeSlot;
			s_Inventory->FirstFreeSlot = Current;

			s_Inventory->NumOccupiedSlots--;
		}
	}
	if (s_Inventory->SelectedSlotIndex >= s_Inventory->NumOccupiedSlots)
	{
		s_Inventory->SelectedSlotIndex = s_Inventory->NumOccupiedSlots - 1;
	}
	RefreshInventoryDisplay(InvElem_InventoryList);
}

static void GiveMoney(u16 Amount)
{
	s_Inventory->Money += Amount;
	RefreshInventoryDisplay(InvElem_PlayerStatus);
}

static void TakeMoney(u16 Amount)
{
	s_Inventory->Money -= Amount;
	RefreshInventoryDisplay(InvElem_PlayerStatus);
}

static void PowerPillBoost(b32 Active);
static b32 IsPowerPillActive();


static b32 UseSelectedItem(entity* Player)
{
	b32 Result = false;
	item_slot* Item = GetItemSlot(s_Inventory->SelectedSlotIndex);
	switch (Item->Type)
	{
		case Item_Potion:
		{
			if (Player->Health >= PLAYER_MAX_HEALTH)
			{
				BeginTextBox("No use. Health full !");
			}
			else
			{
				mmEffect(SFX_POTION);
				Player->Health += 10;
				if (Player->Health > PLAYER_MAX_HEALTH)
				{
					Player->Health = PLAYER_MAX_HEALTH;
				}
				RemoveItem(Item_Potion, 1);
				RefreshInventoryDisplay(InvElem_PlayerStatus | InvElem_InventoryList);
				BeginTextBox("Jack rained 10HP!!");
				Result = true;
			}
		} break;

		case Item_Sword:
		{
			BeginTextBox("Sword alredy equip.");
		} break;

		case Item_Water:
		case Item_FizzyDrink:
		case Item_ChocolateMilk:
		{
			RemoveItem(Item->Type, 1);
			RefreshInventoryDisplay(InvElem_InventoryList);
			BeginTextBox("Mmmmm ! Very refreshing drink !");
		} break;

		case Item_PowerPill:
		{
			if (g_GameState->StateFlags & GameState_InBattle)
			{
				if (IsPowerPillActive())
				{
					BeginTextBox("MAX POWER alredy active !");
				}
				else
				{
					mmEffect(SFX_BONUS);
					PowerPillBoost(true);

					RemoveItem(Item_PowerPill, 1);
					RefreshInventoryDisplay(InvElem_PlayerStatus | InvElem_InventoryList);
					BeginTextBox("Jack attains MAX POWER!!");
					Result = true;
				}
			}
			else
			{
				BeginTextBox("No use. Only in battle !");
			}
		} break;

		case Item_None:
		default:
			break;
	}
	return Result;
}

static void BuySelectedItem(entity* Player)
{
	mmEffect(SFX_MONEY);
	const vending_item* Item = s_VendingMachineItems + s_Inventory->SelectedSlotIndex;
	s_Inventory->Money -= Item->Cost;
	GiveItem(Item->Type, 1);
	RefreshInventoryDisplay(InvElem_PlayerStatus);
	BeginTextBox("Buyed item.");
}


static void UpdateInventorySelection(entity* Player)
{
	if (g_Input->KeyPressed(KEY_START) && 
		!(Player->StateFlags & EFlag_Locked) &&
		!(g_GameState->StateFlags & GameState_InBattle) && 
		!(g_GameState->OpenMenus & Menu_VendingMachine))
	{
		g_GameState->OpenMenus ^= Menu_StartMenu;
		mmEffect(SFX_MENUOPEN);
	}

	if ((g_GameState->OpenMenus & Menu_VendingMachine) && s_Inventory->Money >= 65'500 && s_Inventory->Money < 65'512)
	{
		//BeginTextBox("Not so thirsty anymore...");
		g_GameState->OpenMenus &= ~(Menu_VendingMachine | Menu_PlayerStatus);
	}
		
	if (((g_GameState->OpenMenus & Menu_StartMenu) || (g_GameState->OpenMenus & Menu_VendingMachine)) &&
		!IsDialogueBoxActive() &&
		!(g_GameState->StateFlags & GameState_InBattle))
	{
		b32 HasSomethingToSelect = (g_GameState->OpenMenus & Menu_VendingMachine) ? true : (g_GameState->Inventory->NumOccupiedSlots > 0);
		if (g_Input->KeyPressed(KEY_A) && HasSomethingToSelect)
		{
			mmEffect(SFX_INTERACT);
			if ((g_GameState->OpenMenus & Menu_ConfirmWindow))
			{
				if (s_Inventory->YesNoIndex == Confirm_Yes)
				{
					if (g_GameState->OpenMenus & Menu_VendingMachine)
					{
						BuySelectedItem(Player);
					}
					else
					{
						UseSelectedItem(Player);
					}
				}
				g_GameState->OpenMenus &= ~Menu_ConfirmWindow;
			}
			else
			{
				g_GameState->OpenMenus |= Menu_ConfirmWindow;
				s_Inventory->YesNoIndex = Confirm_Yes;
			}

		}
		else if (g_Input->KeyPressed(KEY_B))
		{
			if (g_GameState->OpenMenus & Menu_ConfirmWindow)
			{
				g_GameState->OpenMenus &= ~Menu_ConfirmWindow;
			}
			else
			{
				g_GameState->OpenMenus &= ~(Menu_StartMenu | Menu_VendingMachine);
			}
		}
	}
	else
	{
		g_GameState->OpenMenus &= ~Menu_ConfirmWindow;
	}

	b32 YesNoConfirmationInFocus = !(g_GameState->StateFlags & GameState_InBattle) && (g_GameState->OpenMenus & Menu_ConfirmWindow);
	b32 BattleCommandMenuInFocus = (g_GameState->StateFlags & GameState_InBattle) && !(g_GameState->OpenMenus & Menu_Inventory);

	if (g_Input->KeyPressed(KEY_DOWN))
	{
		if (YesNoConfirmationInFocus || BattleCommandMenuInFocus)
		{
			if (s_Inventory->YesNoIndex == 0)
			{
				mmEffect(SFX_MENUSELECT);
				s_Inventory->YesNoIndex = 1;
				RefreshInventoryDisplay(InvElem_YesNoArrow);
			}
		}
		else if (g_GameState->OpenMenus & (Menu_Inventory | Menu_VendingMachine))
		{
			s8 MaxSelection = (g_GameState->OpenMenus & Menu_VendingMachine) ? countof(s_VendingMachineItems) - 1 : s_Inventory->NumOccupiedSlots - 1;
			if (s_Inventory->SelectedSlotIndex < MaxSelection)
			{
				mmEffect(SFX_MENUSELECT);
				s_Inventory->SelectedSlotIndex++;
				RefreshInventoryDisplay(InvElem_SelectionArrow);
			}
		}
	}
	else if (g_Input->KeyPressed(KEY_UP))
	{
		if (YesNoConfirmationInFocus || BattleCommandMenuInFocus)
		{
			if (s_Inventory->YesNoIndex == 1)
			{
				mmEffect(SFX_MENUSELECT);
				s_Inventory->YesNoIndex = 0;
				RefreshInventoryDisplay(InvElem_YesNoArrow);
			}
		}
		else if (g_GameState->OpenMenus & (Menu_Inventory | Menu_VendingMachine))
		{
			if (s_Inventory->SelectedSlotIndex > 0)
			{
				mmEffect(SFX_MENUSELECT);
				s_Inventory->SelectedSlotIndex--;
				RefreshInventoryDisplay(InvElem_SelectionArrow);
			}
		}
	}

	if (g_GameState->OpenMenus & Menu_VendingMachine)
	{
		s_Inventory->SelectedSlotIndex = clamp(s_Inventory->SelectedSlotIndex, 0, countof(s_VendingMachineItems));
	}
	else
	{
		s_Inventory->SelectedSlotIndex = clamp(s_Inventory->SelectedSlotIndex, 0, s_Inventory->NumOccupiedSlots);
	}
}

static constexpr const char* GetItemString(item_type Item)
{
	switch (Item)
	{
		case Item_Potion:
			return "Poshon";
		case Item_Sword:
			return "Sword";
		case Item_Water:
			return "Agua";
		case Item_FizzyDrink:
			return "Soda";
		case Item_ChocolateMilk:
			return "Milk";
		case Item_PowerPill:
			return "PwrPill";
		case Item_None:
			break;
	}
	Assert(false);
	return "???";
}


// piggy bit of C++ crap
struct int_str_lut
{ 
	constexpr int_str_lut() : Values()
	{
		for (u32 i = 0; i < 65'536; i++)
		{
			char Temp[7] = { 0 };

			char* Str = Temp;
			u32 Num = i;
			u32 NumDigits = 0;
			do
			{
				u32 Digit = Num % 10;
				*Str++ = '0' + Digit;
				Num /= 10;
				if (++NumDigits == 3 && Num != 0)
				{
					*Str++ = ',';
				}
			} while (Num != 0);
			Str--;

			char* Result = Values[i];
			while (Str >= Temp)
			{
				*Result++ = *Str--;
			}
			*Result = 0;
		}
	}

	inline const char* Get(u16 Value) const
	{
		const char* Result = (const char*)Values[Value];
		return Result;
	}

	char Values[65'536][7];
};

static constexpr int_str_lut INT_STR_LUT;