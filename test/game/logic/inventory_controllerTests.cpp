// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include <gtest/gtest.h>

#include <memory>

#include "game/logic/inventory_controller.h"

namespace game
{
	TEST(InventoryController, MoveStackToEmptySlot) {
		// Moving from inventory position 0 to position 3

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		// Position 3 should have the 50 items + item prototype after moving
		inv[0].first  = item.get();
		inv[0].second = 50;

		inv[3].first  = nullptr;
		inv[3].second = 0;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].first, nullptr);
		EXPECT_EQ(inv[0].second, 0);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(InventoryController, MoveStackToPartialFilledSlotNonExceeding) {
		// Moving from inventory position 0 to position 3. not exceeding the max stack size
		// moving 10 items:
		// position 3 already has 30 items, adding 10 from position 0 to equal 40.

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		inv[0].first  = item.get();
		inv[0].second = 10;

		inv[3].first  = item.get();
		inv[3].second = 30;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].first, nullptr);
		EXPECT_EQ(inv[0].second, 0);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 40);
	}

	TEST(InventoryController, MoveStackToPartialFilledSlotExceeding) {
		// Moving from inventory position 0 to position 3. EXCEEDING the max stack size
		// moving 30 items:
		// position 3 already has 30 items, meaning only 20 can be moved into it to reach the stack size of 50.
		// this leaves 10 in the original location (0) and 50 in the target location (3)

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		inv[0].first  = item.get();
		inv[0].second = 30;

		inv[3].first  = item.get();
		inv[3].second = 30;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 10);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(InventoryController, MoveStackDifferentItemStacks) {
		// Moving from inventory position 0 to position 3
		// The item stacks are of different items, therefore only swapping positions

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		const auto item2 = std::make_unique<jactorio::data::Item>();
		item2->stackSize = 100;

		// Position 3 should have the 50 items + item prototype after moving
		inv[0].first  = item.get();
		inv[0].second = 50;

		inv[3].first  = item2.get();
		inv[3].second = 10;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item2.get());
		EXPECT_EQ(inv[0].second, 10);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(InventoryController, MoveStackFullTargetSlot) {
		// Moving from inventory position 0 to position 3.
		// The target slot is full, origin slot has something
		// swap the 2 items
		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		inv[0].first  = item.get();
		inv[0].second = 18;

		inv[3].first  = item.get();
		inv[3].second = 50;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 50);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 18);
	}

	TEST(InventoryController, MoveEmptySlotToEmptySlot) {
		// Moving from inventory position 0 to position 3
		// Moving nothing to nothing results in nothing!

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		inv[0].first  = nullptr;
		inv[0].second = 0;

		inv[3].first  = nullptr;
		inv[3].second = 0;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].first, nullptr);
		EXPECT_EQ(inv[0].second, 0);

		EXPECT_EQ(inv[3].first, nullptr);
		EXPECT_EQ(inv[3].second, 0);
	}

	// Items somehow exceeding their item stacks

	TEST(InventoryController, MoveExceedingStackToEmptySlot) {
		// Moving from inventory position 0 to position 3.
		// The origin item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only the stack size into the empty slot

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		inv[0].first  = item.get();
		inv[0].second = 9000;

		inv[3].first  = nullptr;
		inv[3].second = 0;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 8950);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(InventoryController, MoveEmptySlotToExceedingStack) {
		// Moving from inventory position 0 to position 3.
		// The TARGET item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only the stack size into the empty origin slot

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		inv[0].first  = nullptr;
		inv[0].second = 0;

		inv[3].first  = item.get();
		inv[3].second = 9000;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 50);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 8950);
	}

	TEST(InventoryController, MoveExceedingStackToPartialSlot) {
		// Moving from inventory position 0 to position 3.
		// The origin item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only 10 to reach the stack size in the target slot

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		inv[0].first  = item.get();
		inv[0].second = 9000;

		inv[3].first  = item.get();
		inv[3].second = 40;

		const bool result = jactorio::game::MoveItemstackToIndex(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 8990);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	// ------------------------------------------------------
	// RIGHT click tests
	// ------------------------------------------------------
	TEST(InventoryController, MoveRclickEmptyOriginInv) {
		// Moving from inventory position 0 to position 3.
		// Origin inventory is empty, right clicking on an item takes half of it
		// round down, unless there is only 1, where one is taken

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		auto item       = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		// Case 1, even number
		{
			inv[0].first  = nullptr;
			inv[0].second = 0;

			inv[3].first  = item.get();
			inv[3].second = 40;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 20);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 20);
		}
		// Case 2, odd number
		{
			inv[0].first  = nullptr;
			inv[0].second = 0;

			inv[3].first  = item.get();
			inv[3].second = 39;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 19);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 20);
		}
		// Case 3, 1 item
		{
			inv[0].first  = nullptr;
			inv[0].second = 0;

			inv[3].first  = item.get();
			inv[3].second = 1;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 1);

			EXPECT_EQ(inv[3].first, nullptr);
			EXPECT_EQ(inv[3].second, 0);
		}
		// Case 4, Exceed stack size
		{
			inv[0].first  = nullptr;
			inv[0].second = 0;

			inv[3].first  = item.get();
			inv[3].second = 110;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 50);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 60);
		}
	}

	TEST(InventoryController, MoveRclickEmptyTargetInv) {
		// Moving from inventory position 0 to position 3.
		// Target inventory is empty, right clicking drops 1 item off

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		auto item       = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		// Case 1, > 1 item
		{
			inv[0].first  = item.get();
			inv[0].second = 10;

			inv[3].first  = nullptr;
			inv[3].second = 0;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 9);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 1);
		}
		// Case 2, 1 item
		{
			inv[0].first  = item.get();
			inv[0].second = 1;

			inv[3].first  = nullptr;
			inv[3].second = 0;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, true);

			EXPECT_EQ(inv[0].first, nullptr);
			EXPECT_EQ(inv[0].second, 0);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 1);
		}
		// Case 3, Target has 1 of the item
		{
			inv[0].first  = item.get();
			inv[0].second = 1;

			inv[3].first  = item.get();
			inv[3].second = 1;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, true);

			EXPECT_EQ(inv[0].first, nullptr);
			EXPECT_EQ(inv[0].second, 0);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 2);
		}
		// Case 4, target has > 1 of the item
		{
			inv[0].first  = item.get();
			inv[0].second = 10;

			inv[3].first  = item.get();
			inv[3].second = 1;

			const bool result = jactorio::game::MoveItemstackToIndex(inv,
			                                                         0,
			                                                         inv,
			                                                         3,
			                                                         1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 9);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 2);
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
	// add_stack
	TEST(InventoryController, AddStackAddToEmptySlot) {
		// Should find the first empty slot and add the item there
		// Slots, 0, 1 Will be with another item
		// Should place in slot 2

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto item  = std::make_unique<jactorio::data::Item>();
		const auto item2 = std::make_unique<jactorio::data::Item>();

		// Another item
		inv[0].first  = item.get();
		inv[0].second = 10;
		inv[1].first  = item.get();
		inv[1].second = 21;

		auto add_item = jactorio::data::ItemStack(item2.get(), 20);
		EXPECT_TRUE(jactorio::game::CanAddStack(inv, inv_size, add_item));
		EXPECT_TRUE(jactorio::game::AddStackSub(inv, inv_size, add_item));

		EXPECT_EQ(add_item.second, 0);

		// Did not modify existing items
		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 10);
		EXPECT_EQ(inv[1].first, item.get());
		EXPECT_EQ(inv[1].second, 21);

		// Added itemstack
		EXPECT_EQ(inv[2].first, item2.get());
		EXPECT_EQ(inv[2].second, 20);
	}

	TEST(InventoryController, AddStackAddToExistingSlot) {
		// Should find slot with item of same type, respecting max stack size, add the remaining at the next
		// available slot which is another item of the same type
		// 
		// Slots, 0 Will be with another item
		// Should place in slot 1 2 3 | Add amounts: (10, 10, 30)

		constexpr unsigned short inv_size = 10;
		jactorio::data::ItemStack inv[inv_size];

		const auto another_item   = std::make_unique<jactorio::data::Item>();
		const auto item_we_add_to = std::make_unique<jactorio::data::Item>();
		item_we_add_to->stackSize = 50;

		inv[0].first  = another_item.get();
		inv[0].second = 10;

		// Will fill up first 2, then dump the remaining in 3
		inv[1].first  = item_we_add_to.get();
		inv[1].second = 40;
		inv[2].first  = item_we_add_to.get();
		inv[2].second = 40;

		inv[3].first  = item_we_add_to.get();
		inv[3].second = 20;

		auto add_item = jactorio::data::ItemStack(item_we_add_to.get(), 50);
		EXPECT_TRUE(jactorio::game::CanAddStack(inv, inv_size, add_item));
		EXPECT_TRUE(jactorio::game::AddStackSub(inv, inv_size, add_item));

		EXPECT_EQ(add_item.second, 0);

		// Did not modify other items
		EXPECT_EQ(inv[0].first, another_item.get());
		EXPECT_EQ(inv[0].second, 10);

		// Added correctly??
		EXPECT_EQ(inv[1].first, item_we_add_to.get());
		EXPECT_EQ(inv[1].second, 50);
		EXPECT_EQ(inv[2].first, item_we_add_to.get());
		EXPECT_EQ(inv[2].second, 50);

		EXPECT_EQ(inv[3].first, item_we_add_to.get());
		EXPECT_EQ(inv[3].second, 50);

		// No items should be added to the 4th slot index
		EXPECT_EQ(inv[4].first, nullptr);
		EXPECT_EQ(inv[4].second, 0);
	}

	TEST(InventoryController, AddStackNoAvailableSlots) {
		// Slots 1 is full, inv size is 1, will return false
		constexpr unsigned short inv_size = 1;
		jactorio::data::ItemStack inv[inv_size];

		const auto item  = std::make_unique<jactorio::data::Item>();
		const auto item2 = std::make_unique<jactorio::data::Item>();

		inv[0].first  = item.get();
		inv[0].second = 10;

		auto add_item = jactorio::data::ItemStack(item2.get(), 20);
		EXPECT_FALSE(jactorio::game::CanAddStack(inv, inv_size, add_item));
		EXPECT_FALSE(jactorio::game::AddStackSub(inv, inv_size, add_item));

		EXPECT_EQ(add_item.second, 20);

		// Did not modify existing items
		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 10);
	}


	//
	//
	//
	// Get item stack count
	//
	//
	TEST(InventoryController, GetInvItemCount) {
		const auto item  = std::make_unique<jactorio::data::Item>();
		const auto item2 = std::make_unique<jactorio::data::Item>();

		jactorio::data::ItemStack inv[30];

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

		EXPECT_EQ(jactorio::game::GetInvItemCount(inv, 30, item.get()), 101);
	}

	TEST(InventoryController, RemoveInvItemS) {
		using namespace jactorio;
		data::ItemStack inv[30];

		const auto item = std::make_unique<data::Item>();
		inv[20]         = {item.get(), 5};
		inv[23]         = {item.get(), 5};

		EXPECT_EQ(
			jactorio::game::RemoveInvItemS(inv, 30, item.get(), 10),
			true);

		// Inventory should be empty
		for (auto& i : inv) {
			EXPECT_EQ(i.first, nullptr);
			// EXPECT_EQ(i.second, 0);
		}
	}

	TEST(InventoryController, RemoveInvItemSInvalid) {
		jactorio::data::ItemStack inv[30];

		const auto item = std::make_unique<jactorio::data::Item>();
		inv[20]         = {item.get(), 5};

		// Attempting to remove 10 when only 5 exists
		EXPECT_EQ(
			jactorio::game::RemoveInvItemS(inv, 30, item.get(), 10),
			false);

		// Inventory unchanged
		EXPECT_EQ(inv[20].first, item.get());
		EXPECT_EQ(inv[20].second, 5);
	}
}
