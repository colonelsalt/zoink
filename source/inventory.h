#pragma once

enum item_type
{
	Item_None,

	Item_Potion,
	Item_Sword,
	Item_Water,
	Item_FizzyDrink,
	Item_ChocolateMilk,
	Item_PowerPill
};

struct item_slot
{
	item_type Type;
	u8 Quantity;
	item_slot* Next; // if allocated: next item in inventory - otherwise: next free slot
};

enum confirmation_options
{
	Confirm_Yes,
	Confirm_No
};

enum battle_menu_options
{
	BattleMenu_Fight,
	BattleMenu_Pak
};

struct inventory
{
	item_slot* ItemHead;
	item_slot* FirstFreeSlot;
	u8 NumOccupiedSlots;
	u16 Money;

	s8 SelectedSlotIndex;
	s8 YesNoIndex; // 0 for YES (option 1); 1 for NO (option 2)

	ewram_arena* Arena;
};

struct vending_item
{
	item_type Type;
	u8 Cost;
};

static constexpr vending_item s_VendingMachineItems[] =
{
	{ Item_Water, 5 },
	{ Item_FizzyDrink, 10 },
	{ Item_ChocolateMilk, 12 }
};

static inventory* InitInventory(ewram_arena* Arena);