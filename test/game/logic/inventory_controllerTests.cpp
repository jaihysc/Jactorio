// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include <gtest/gtest.h>

#include <memory>

#include "game/logic/inventory_controller.h"

namespace jactorio::game
{
	TEST(InventoryController, MoveStackToEmptySlot) {
		// Moving from inventory position 0 to position 3

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		// Position 3 should have the 50 items + item prototype after moving
		inv[0].item  = item.get();
		inv[0].count = 50;

		inv[3].item  = nullptr;
		inv[3].count = 0;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].item, nullptr);
		EXPECT_EQ(inv[0].count, 0);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 50);
	}

	TEST(InventoryController, MoveStackToPartialFilledSlotNonExceeding) {
		// Moving from inventory position 0 to position 3. not exceeding the max stack size
		// moving 10 items:
		// position 3 already has 30 items, adding 10 from position 0 to equal 40.

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		inv[0].item  = item.get();
		inv[0].count = 10;

		inv[3].item  = item.get();
		inv[3].count = 30;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].item, nullptr);
		EXPECT_EQ(inv[0].count, 0);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 40);
	}

	TEST(InventoryController, MoveStackToPartialFilledSlotExceeding) {
		// Moving from inventory position 0 to position 3. EXCEEDING the max stack size
		// moving 30 items:
		// position 3 already has 30 items, meaning only 20 can be moved into it to reach the stack size of 50.
		// this leaves 10 in the original location (0) and 50 in the target location (3)

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		inv[0].item  = item.get();
		inv[0].count = 30;

		inv[3].item  = item.get();
		inv[3].count = 30;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].item, item.get());
		EXPECT_EQ(inv[0].count, 10);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 50);
	}

	TEST(InventoryController, MoveStackDifferentItemStacks) {
		// Moving from inventory position 0 to position 3
		// The item stacks are of different items, therefore only swapping positions

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		const auto item2 = std::make_unique<data::Item>();
		item2->stackSize = 100;

		// Position 3 should have the 50 items + item prototype after moving
		inv[0].item  = item.get();
		inv[0].count = 50;

		inv[3].item  = item2.get();
		inv[3].count = 10;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].item, item2.get());
		EXPECT_EQ(inv[0].count, 10);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 50);
	}

	TEST(InventoryController, MoveStackFullTargetSlot) {
		// Moving from inventory position 0 to position 3.
		// The target slot is full, origin slot has something
		// swap the 2 items
		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		inv[0].item  = item.get();
		inv[0].count = 18;

		inv[3].item  = item.get();
		inv[3].count = 50;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].item, item.get());
		EXPECT_EQ(inv[0].count, 50);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 18);
	}

	TEST(InventoryController, MoveEmptySlotToEmptySlot) {
		// Moving from inventory position 0 to position 3
		// Moving nothing to nothing results in nothing!

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		inv[0].item  = nullptr;
		inv[0].count = 0;

		inv[3].item  = nullptr;
		inv[3].count = 0;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].item, nullptr);
		EXPECT_EQ(inv[0].count, 0);

		EXPECT_EQ(inv[3].item, nullptr);
		EXPECT_EQ(inv[3].count, 0);
	}

	// Items somehow exceeding their item stacks

	TEST(InventoryController, MoveExceedingStackToEmptySlot) {
		// Moving from inventory position 0 to position 3.
		// The origin item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only the stack size into the empty slot

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		inv[0].item  = item.get();
		inv[0].count = 9000;

		inv[3].item  = nullptr;
		inv[3].count = 0;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].item, item.get());
		EXPECT_EQ(inv[0].count, 8950);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 50);
	}

	TEST(InventoryController, MoveEmptySlotToExceedingStack) {
		// Moving from inventory position 0 to position 3.
		// The TARGET item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only the stack size into the empty origin slot

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		inv[0].item  = nullptr;
		inv[0].count = 0;

		inv[3].item  = item.get();
		inv[3].count = 9000;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].item, item.get());
		EXPECT_EQ(inv[0].count, 50);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 8950);
	}

	TEST(InventoryController, MoveExceedingStackToPartialSlot) {
		// Moving from inventory position 0 to position 3.
		// The origin item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only 10 to reach the stack size in the target slot

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		const auto item = std::make_unique<data::Item>();
		item->stackSize = 50;

		inv[0].item  = item.get();
		inv[0].count = 9000;

		inv[3].item  = item.get();
		inv[3].count = 40;

		const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].item, item.get());
		EXPECT_EQ(inv[0].count, 8990);

		EXPECT_EQ(inv[3].item, item.get());
		EXPECT_EQ(inv[3].count, 50);
	}

	// ------------------------------------------------------
	// RIGHT click tests
	// ------------------------------------------------------
	TEST(InventoryController, MoveRclickEmptyOriginInv) {
		// Moving from inventory position 0 to position 3.
		// Origin inventory is empty, right clicking on an item takes half of it
		// round down, unless there is only 1, where one is taken

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		auto item       = std::make_unique<data::Item>();
		item->stackSize = 50;

		// Case 1, even number
		{
			inv[0].item  = nullptr;
			inv[0].count = 0;

			inv[3].item  = item.get();
			inv[3].count = 40;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].item, item.get());
			EXPECT_EQ(inv[0].count, 20);

			EXPECT_EQ(inv[3].item, item.get());
			EXPECT_EQ(inv[3].count, 20);
		}
		// Case 2, odd number
		{
			inv[0].item  = nullptr;
			inv[0].count = 0;

			inv[3].item  = item.get();
			inv[3].count = 39;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].item, item.get());
			EXPECT_EQ(inv[0].count, 19);

			EXPECT_EQ(inv[3].item, item.get());
			EXPECT_EQ(inv[3].count, 20);
		}
		// Case 3, 1 item
		{
			inv[0].item  = nullptr;
			inv[0].count = 0;

			inv[3].item  = item.get();
			inv[3].count = 1;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].item, item.get());
			EXPECT_EQ(inv[0].count, 1);

			EXPECT_EQ(inv[3].item, nullptr);
			EXPECT_EQ(inv[3].count, 0);
		}
		// Case 4, Exceed stack size
		{
			inv[0].item  = nullptr;
			inv[0].count = 0;

			inv[3].item  = item.get();
			inv[3].count = 110;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].item, item.get());
			EXPECT_EQ(inv[0].count, 50);

			EXPECT_EQ(inv[3].item, item.get());
			EXPECT_EQ(inv[3].count, 60);
		}
	}

	TEST(InventoryController, MoveRclickEmptyTargetInv) {
		// Moving from inventory position 0 to position 3.
		// Target inventory is empty, right clicking drops 1 item off

		constexpr unsigned short inv_size = 10;
		data::Item::Stack inv[inv_size];

		auto item       = std::make_unique<data::Item>();
		item->stackSize = 50;

		// Case 1, > 1 item
		{
			inv[0].item  = item.get();
			inv[0].count = 10;

			inv[3].item  = nullptr;
			inv[3].count = 0;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].item, item.get());
			EXPECT_EQ(inv[0].count, 9);

			EXPECT_EQ(inv[3].item, item.get());
			EXPECT_EQ(inv[3].count, 1);
		}
		// Case 2, 1 item
		{
			inv[0].item  = item.get();
			inv[0].count = 1;

			inv[3].item  = nullptr;
			inv[3].count = 0;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, true);

			EXPECT_EQ(inv[0].item, nullptr);
			EXPECT_EQ(inv[0].count, 0);

			EXPECT_EQ(inv[3].item, item.get());
			EXPECT_EQ(inv[3].count, 1);
		}
		// Case 3, Target has 1 of the item
		{
			inv[0].item  = item.get();
			inv[0].count = 1;

			inv[3].item  = item.get();
			inv[3].count = 1;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, true);

			EXPECT_EQ(inv[0].item, nullptr);
			EXPECT_EQ(inv[0].count, 0);

			EXPECT_EQ(inv[3].item, item.get());
			EXPECT_EQ(inv[3].count, 2);
		}
		// Case 4, target has > 1 of the item
		{
			inv[0].item  = item.get();
			inv[0].count = 10;

			inv[3].item  = item.get();
			inv[3].count = 1;

			const bool result = MoveItemstackToIndex(inv[0],
			                                         inv[3],
			                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].item, item.get());
			EXPECT_EQ(inv[0].count, 9);

			EXPECT_EQ(inv[3].item, item.get());
			EXPECT_EQ(inv[3].count, 2);
		}
	}


	//
	//
	//
	//
	//
	//
	//
	//
	TEST(InventoryController, AddStackAddToEmptySlot) {
		// Should find the first empty slot and add the item there
		// Slots, 0, 1 Will be with another item
		// Should place in slot 2

		constexpr unsigned short inv_size = 10;
		data::Item::Inventory inv{inv_size};

		const auto item  = std::make_unique<data::Item>();
		const auto item2 = std::make_unique<data::Item>();

		// Another item
		inv[0].item  = item.get();
		inv[0].count = 10;
		inv[1].item  = item.get();
		inv[1].count = 21;

		auto add_item = data::Item::Stack{item2.get(), 20};
		EXPECT_TRUE(CanAddStack(inv, add_item).first);
		EXPECT_TRUE(AddStackSub(inv, add_item));

		EXPECT_EQ(add_item.count, 0);

		// Did not modify existing items
		EXPECT_EQ(inv[0].item, item.get());
		EXPECT_EQ(inv[0].count, 10);
		EXPECT_EQ(inv[1].item, item.get());
		EXPECT_EQ(inv[1].count, 21);

		// Added itemstack
		EXPECT_EQ(inv[2].item, item2.get());
		EXPECT_EQ(inv[2].count, 20);
	}

	TEST(InventoryController, AddStackAddToExistingSlot) {
		// Should find slot with item of same type, respecting max stack size, add the remaining at the next
		// available slot which is another item of the same type
		// 
		// Slots, 0 Will be with another item
		// Should place in slot 1 2 3 | Add amounts: (10, 10, 30)

		constexpr unsigned short inv_size = 10;
		data::Item::Inventory inv{inv_size};

		const auto another_item   = std::make_unique<data::Item>();
		const auto item_we_add_to = std::make_unique<data::Item>();
		item_we_add_to->stackSize = 50;

		inv[0].item  = another_item.get();
		inv[0].count = 10;

		// Will fill up first 2, then dump the remaining in 3
		inv[1].item  = item_we_add_to.get();
		inv[1].count = 40;
		inv[2].item  = item_we_add_to.get();
		inv[2].count = 40;

		inv[3].item  = item_we_add_to.get();
		inv[3].count = 20;

		auto add_item = data::Item::Stack{item_we_add_to.get(), 50};
		EXPECT_TRUE(CanAddStack(inv, add_item).first);
		EXPECT_TRUE(AddStackSub(inv, add_item));

		EXPECT_EQ(add_item.count, 0);

		// Did not modify other items
		EXPECT_EQ(inv[0].item, another_item.get());
		EXPECT_EQ(inv[0].count, 10);

		// Added correctly??
		EXPECT_EQ(inv[1].item, item_we_add_to.get());
		EXPECT_EQ(inv[1].count, 50);
		EXPECT_EQ(inv[2].item, item_we_add_to.get());
		EXPECT_EQ(inv[2].count, 50);

		EXPECT_EQ(inv[3].item, item_we_add_to.get());
		EXPECT_EQ(inv[3].count, 50);

		// No items should be added to the 4th slot index
		EXPECT_EQ(inv[4].item, nullptr);
		EXPECT_EQ(inv[4].count, 0);
	}

	TEST(InventoryController, AddStackNoAvailableSlots) {
		// Slots 1 is full, inv size is 1, will return false
		constexpr unsigned short inv_size = 1;
		data::Item::Inventory inv{inv_size};

		const auto item  = std::make_unique<data::Item>();
		const auto item2 = std::make_unique<data::Item>();

		inv[0].item  = item.get();
		inv[0].count = 10;

		auto add_item = data::Item::Stack{item2.get(), 20};
		EXPECT_FALSE(CanAddStack(inv, add_item).first);
		EXPECT_FALSE(AddStackSub(inv, add_item));

		EXPECT_EQ(add_item.count, 20);

		// Did not modify existing items
		EXPECT_EQ(inv[0].item, item.get());
		EXPECT_EQ(inv[0].count, 10);
	}

	TEST(InventoryController, AddStackFiltered) {
		// Item must match filter, otherwise it cannot be at the slot with the filter
		
		data::Item filtered_item{};
		data::Item not_filtered_item{};

		// Slot 0 is filtered
		data::Item::Inventory inv{2};
		inv[0].filter =  &filtered_item;


		// Cannot insert into slot 0
		EXPECT_EQ(CanAddStack(inv, {&not_filtered_item, 10}).second, 1);
		AddStack(inv, {&not_filtered_item, 10});

		EXPECT_EQ(inv[0].item, nullptr);
		EXPECT_EQ(inv[1].item, &not_filtered_item);
	}

	//
	//
	//
	// Get item stack count
	//
	//
	TEST(InventoryController, GetInvItemCount) {
		const auto item  = std::make_unique<data::Item>();
		const auto item2 = std::make_unique<data::Item>();

		constexpr auto inv_size = 30;
		data::Item::Inventory inv{inv_size};

		// Count these to a sum of 101
		inv[0]  = {item.get(), 20};
		inv[5]  = {item.get(), 30};
		inv[8]  = {item.get(), 20};
		inv[14] = {item.get(), 20};
		inv[20] = {item.get(), 5};
		inv[29] = {item.get(), 6};

		// Should not count these
		inv[3]  = {item2.get(), 200};
		inv[9]  = {item2.get(), 200};
		inv[13] = {item2.get(), 200};
		inv[28] = {item2.get(), 200};

		EXPECT_EQ(GetInvItemCount(inv, item.get()), 101);
	}

	TEST(InventoryController, GetFirstItem) {
		const auto item  = std::make_unique<data::Item>();
		const auto item2 = std::make_unique<data::Item>();

		constexpr auto inv_size = 10;
		data::Item::Inventory inv{inv_size};

		EXPECT_EQ(GetFirstItem(inv), nullptr);  // No items

		inv[3] = {item2.get(), 32};
		inv[4] = {item.get(), 2};

		EXPECT_EQ(GetFirstItem(inv), item2.get());
	}

	TEST(InventoryController, RemoveInvItemS) {
		constexpr auto inv_size = 30;
		data::Item::Inventory inv{inv_size};

		const auto item = std::make_unique<data::Item>();
		inv[20]         = {item.get(), 5};
		inv[23]         = {item.get(), 5};

		EXPECT_TRUE(RemoveInvItem(inv, item.get(), 10));

		// Inventory should be empty
		for (auto& i : inv) {
			EXPECT_EQ(i.item, nullptr);
			// EXPECT_EQ(i.second, 0);
		}
	}

	TEST(InventoryController, RemoveInvItemSInvalid) {
		constexpr auto inv_size = 30;
		data::Item::Inventory inv{inv_size};

		const auto item = std::make_unique<data::Item>();
		inv[20]         = {item.get(), 5};

		// Attempting to remove 10 when only 5 exists
		EXPECT_FALSE(RemoveInvItem(inv, item.get(), 10));

		// Inventory unchanged
		EXPECT_EQ(inv[20].item, item.get());
		EXPECT_EQ(inv[20].count, 5);
	}
}
