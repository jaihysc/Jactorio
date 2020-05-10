// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/24/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "game/player/player_data.h"

namespace game
{
	class PlayerDataInventoryTest : public testing::Test
	{
		bool setupCursor_ = false;

	protected:
		jactorio::game::PlayerData playerData_{};

		jactorio::data::Item* cursor_ = nullptr;

		void TearDown() override {
			if (setupCursor_) {
				jactorio::data::ClearData();
			}
		}

		///
		/// \brief Creates the cursor prototype which is hardcoded when an item is selected
		void SetupInventoryCursor() {
			setupCursor_ = true;
			cursor_      = new jactorio::data::Item();

			DataRawAdd(jactorio::game::PlayerData::kInventorySelectedCursorIname, cursor_);
		}
	};

	TEST_F(PlayerDataInventoryTest, InventoryLclickSelectItemByReference) {
		// Left click on a slot picks up items by reference
		// The inventory slot becomes the cursor
		// The cursor holds the item
		SetupInventoryCursor();

		const auto item = std::make_unique<jactorio::data::Item>();


		// Position 3 should have the 50 items + item prototype after moving
		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 50;

		playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, cursor_);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 0);

		const auto* cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);
	}

	TEST_F(PlayerDataInventoryTest, InventoryDeselectReferencedItem) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot deselects the item
		SetupInventoryCursor();

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			playerData_.inventoryPlayer[0].first  = item.get();
			playerData_.inventoryPlayer[0].second = 50;

			playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);  // Select
			playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);  // Deselect


			EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 50);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item, nullptr);
		}
		playerData_.ClearPlayerInventory();
		// Right click
		{
			// Position 3 should have the 50 items + item prototype after moving
			playerData_.inventoryPlayer[0].first  = item.get();
			playerData_.inventoryPlayer[0].second = 50;

			playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);  // Select
			playerData_.InventoryClick(0, 1, true, playerData_.inventoryPlayer);  // Deselect


			EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 50);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}

	TEST_F(PlayerDataInventoryTest, InventoryDeselectReferencedItem2Inventories) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot in another inventory however will not deselect the item
		SetupInventoryCursor();

		const auto item = std::make_unique<jactorio::data::Item>();

		playerData_.ClearPlayerInventory();
		// Left click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			playerData_.inventoryPlayer[0].first  = item.get();
			playerData_.inventoryPlayer[0].second = 50;

			playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);  // Select

			// Deselect into inv_2
			jactorio::data::ItemStack inv_2[10];
			playerData_.InventoryClick(0, 0, true, inv_2);  // Deselect

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 50);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item, nullptr);

			// Selected cursor should no longer exist in inventory_player
			EXPECT_EQ(playerData_.inventoryPlayer[0].first, nullptr);
		}
		playerData_.ClearPlayerInventory();
		// Right click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			playerData_.inventoryPlayer[0].first  = item.get();
			playerData_.inventoryPlayer[0].second = 50;

			playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);  // Select

			// Deselect into inv_2
			jactorio::data::ItemStack inv_2[10];
			playerData_.InventoryClick(0, 1, true, inv_2);  // Will NOT Deselect since in another inventory

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 1);

			// Cursor still holds 49
			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 49);

			// Selected cursor should STILL exist in inventory_player since not deselected
			EXPECT_NE(playerData_.inventoryPlayer[0].first, nullptr);
		}
	}

	TEST_F(PlayerDataInventoryTest, InventoryMoveReferencedItem) {
		// Left click on a slot picks up items by reference
		// Left click on index 3 to drop off the item at index

		// Index 0 (origin) should be empty
		// Create the cursor prototype
		SetupInventoryCursor();

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			playerData_.inventoryPlayer[0].first  = item.get();
			playerData_.inventoryPlayer[0].second = 50;

			playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);  // Select item
			playerData_.InventoryClick(3, 0, true, playerData_.inventoryPlayer);  // Drop item off


			EXPECT_EQ(playerData_.inventoryPlayer[0].first, nullptr);
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 0);

			EXPECT_EQ(playerData_.inventoryPlayer[3].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[3].second, 50);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}


	TEST_F(PlayerDataInventoryTest, InventoryRclickSelectItemByUnique) {
		// Right click on a slot creates a new inventory slot in the cursor and places half from the inventory into it
		const auto item = std::make_unique<jactorio::data::Item>();


		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 40;

		playerData_.InventoryClick(0, 1, true, playerData_.inventoryPlayer);  // Pick up half

		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 20;

		const auto* cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 20);
	}

	TEST_F(PlayerDataInventoryTest, InventoryDropSingleUniqueItem) {
		// Right click on item to pick up half into cursor
		// Right click on empty slot to drop 1 off from the cursor
		const auto item = std::make_unique<jactorio::data::Item>();

		// Drop one on another location
		{
			playerData_.inventoryPlayer[0].first  = item.get();
			playerData_.inventoryPlayer[0].second = 10;

			playerData_.InventoryClick(0, 1, true, playerData_.inventoryPlayer);  // Pick up half

			EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 5);

			auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 5);


			playerData_.InventoryClick(3, 1, true, playerData_.inventoryPlayer);  // Drop 1 at index 3

			// Should remain unchanged
			EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 5);

			// Loses 1
			cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Gain 1
			EXPECT_EQ(playerData_.inventoryPlayer[3].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[3].second, 1);
		}

		playerData_.ClearPlayerInventory();
		playerData_.ResetInventoryVariables();
		// Drop 1 on the original item stack where half was taken from
		{
			playerData_.inventoryPlayer[0].first  = item.get();
			playerData_.inventoryPlayer[0].second = 10;

			playerData_.InventoryClick(0, 1, true, playerData_.inventoryPlayer);  // Pick up half
			playerData_.
				InventoryClick(0, 1, true, playerData_.inventoryPlayer);  // Drop 1 one the stack it picked up from

			// Loses 1
			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Half (5) plus 1 = 6
			EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 6);
		}
	}

	TEST_F(PlayerDataInventoryTest, InventoryDropStackUniqueItem) {
		// Right click on item to pick up half into cursor
		// Left click on empty slot to drop entire stack off from the cursor
		const auto item = std::make_unique<jactorio::data::Item>();


		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 10;


		playerData_.InventoryClick(0, 1, true, playerData_.inventoryPlayer);  // Pick up half

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 5);

		auto* cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 5);


		playerData_.InventoryClick(3, 0, true, playerData_.inventoryPlayer);  // Drop stack at index 3

		// Should remain unchanged
		EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 5);

		// Empty
		cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item, nullptr);

		// Gain 10
		EXPECT_EQ(playerData_.inventoryPlayer[3].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[3].second, 5);
	}

	TEST_F(PlayerDataInventoryTest, InventoryClickEmptySlot) {
		// Left click on empty slot
		// Should remain unchanged
		playerData_.inventoryPlayer[0].first  = nullptr;
		playerData_.inventoryPlayer[0].second = 0;

		playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, nullptr);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 0);

		const auto* cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item, nullptr);
	}


	// Increment / decrement selected item
	TEST_F(PlayerDataInventoryTest, IncrementSelectedItem) {
		// If player selects item by "unique" or "reference",
		// It should function the same as it only modifies the cursor item stack
		const auto item                       = std::make_unique<jactorio::data::Item>();
		item->stackSize                       = 50;
		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			playerData_.InventoryClick(0, 1, true, playerData_.inventoryPlayer);

			// Check if item was incremented
			EXPECT_EQ(playerData_.IncrementSelectedItem(), true);

			EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 5);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 6);  // This incremented by 1
		}

		// Drop item down at inv slot 1
		{
			playerData_.InventoryClick(1, 0, true, playerData_.inventoryPlayer);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(playerData_.inventoryPlayer[1].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[1].second, 6);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST_F(PlayerDataInventoryTest, IncrementSelectedItemExceedItemStack) {
		// Attempting to increment an item exceeding item stack returns false and fails the increment
		const auto item                       = std::make_unique<jactorio::data::Item>();
		item->stackSize                       = 50;
		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 50;

		// Pickup
		playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);

		// Failed to add item: Item stack already full
		EXPECT_EQ(playerData_.IncrementSelectedItem(), false);

		const auto* cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);  // This unchanged
	}


	TEST_F(PlayerDataInventoryTest, DecrementSelectedItemUnique) {
		// If player selects item by "unique"
		// If decremented to 0, deselect the cursor item
		const auto item                       = std::make_unique<jactorio::data::Item>();
		item->stackSize                       = 50;
		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			playerData_.InventoryClick(0, 1, true, playerData_.inventoryPlayer);

			// Check if item was incremented
			EXPECT_EQ(playerData_.DecrementSelectedItem(), true);

			EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[0].second, 5);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);  // This decremented by 1
		}

		// Drop item down at inv slot 1
		{
			playerData_.InventoryClick(1, 0, true, playerData_.inventoryPlayer);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(playerData_.inventoryPlayer[1].first, item.get());
			EXPECT_EQ(playerData_.inventoryPlayer[1].second, 4);

			const auto* cursor_item = playerData_.GetSelectedItem();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST_F(PlayerDataInventoryTest, DeselectSelectedItem) {
		// If player selects item by "unique"
		// If decremented to 0, deselect the cursor item
		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 10;

		playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);

		EXPECT_TRUE(playerData_.DeselectSelectedItem());

		// Returned to where selection cursor was 
		EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 10);

		const auto* cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item, nullptr);
	}

	TEST_F(PlayerDataInventoryTest, DecrementSelectedItemReachZeroReference) {
		// Selected by reference
		// If decremented to 0, deselect the cursor item
		// If the selected item is empty after decrementing, return false
		SetupInventoryCursor();

		const auto item                       = std::make_unique<jactorio::data::Item>();
		item->stackSize                       = 50;
		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 1;

		// Pickup
		playerData_.InventoryClick(0, 0, true, playerData_.inventoryPlayer);

		EXPECT_EQ(playerData_.DecrementSelectedItem(), false);

		// Cursor is nullptr: no item selected
		const auto* cursor_item = playerData_.GetSelectedItem();
		EXPECT_EQ(cursor_item, nullptr);

		// Should remove the selection cursor on the item
		EXPECT_EQ(playerData_.inventoryPlayer[0].first, nullptr);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 0);
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySort) {
		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		const auto item2 = std::make_unique<jactorio::data::Item>();
		item2->stackSize = 10;

		// Item 1
		playerData_.inventoryPlayer[0].first  = item.get();
		playerData_.inventoryPlayer[0].second = 10;

		playerData_.inventoryPlayer[10].first  = item.get();
		playerData_.inventoryPlayer[10].second = 25;

		playerData_.inventoryPlayer[20].first  = item.get();
		playerData_.inventoryPlayer[20].second = 25;

		playerData_.inventoryPlayer[13].first  = item.get();
		playerData_.inventoryPlayer[13].second = 20;

		playerData_.inventoryPlayer[14].first  = item.get();
		playerData_.inventoryPlayer[14].second = 30;

		// Item 2
		playerData_.inventoryPlayer[31].first  = item2.get();
		playerData_.inventoryPlayer[31].second = 4;

		playerData_.inventoryPlayer[32].first  = item2.get();
		playerData_.inventoryPlayer[32].second = 6;

		playerData_.inventoryPlayer[22].first  = item2.get();
		playerData_.inventoryPlayer[22].second = 1;


		// Sorted inventory should be as follows
		// Item(count)
		// 1(50), 1(50), 1(10), 2(10), 2(1)
		playerData_.InventorySort();

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 50);
		EXPECT_EQ(playerData_.inventoryPlayer[1].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[1].second, 50);
		EXPECT_EQ(playerData_.inventoryPlayer[2].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[2].second, 10);

		EXPECT_EQ(playerData_.inventoryPlayer[3].first, item2.get());
		EXPECT_EQ(playerData_.inventoryPlayer[3].second, 10);
		EXPECT_EQ(playerData_.inventoryPlayer[4].first, item2.get());
		EXPECT_EQ(playerData_.inventoryPlayer[4].second, 1);
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySort2) {
		// Sorting will not move the item with inventory_selected_cursor_iname (to prevent breaking the inventory logic)
		SetupInventoryCursor();

		playerData_.inventoryPlayer[10].first  = cursor_;
		playerData_.inventoryPlayer[10].second = 0;


		playerData_.InventorySort();

		EXPECT_EQ(playerData_.inventoryPlayer[10].first, cursor_);
		EXPECT_EQ(playerData_.inventoryPlayer[10].second, 0);

		// There should have been no new cursors created anywhere
		for (int i = 0; i < jactorio::game::PlayerData::kInventorySize; ++i) {
			if (i == 10)
				continue;
			EXPECT_NE(playerData_.inventoryPlayer[i].first, cursor_);
		}
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySortFull) {
		// Sorting the inventory when it is full should also work
		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		for (auto& i : playerData_.inventoryPlayer) {
			i.first  = item.get();
			i.second = 50;
		}

		playerData_.InventorySort();


		// There should have been no new cursors created anywhere
		for (auto& i : playerData_.inventoryPlayer) {
			EXPECT_EQ(i.first, item.get());
			EXPECT_EQ(i.second, 50);
		}
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySortItemExcedingStack) {
		// If there is an item which exceeds its stack size, do not attempt to stack into it
		const auto item = std::make_unique<jactorio::data::Item>();
		item->stackSize = 50;

		playerData_.inventoryPlayer[10].first  = item.get();
		playerData_.inventoryPlayer[10].second = 100;

		playerData_.inventoryPlayer[11].first  = item.get();
		playerData_.inventoryPlayer[11].second = 100;

		playerData_.inventoryPlayer[12].first  = item.get();
		playerData_.inventoryPlayer[12].second = 10;

		playerData_.InventorySort();

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 100);

		EXPECT_EQ(playerData_.inventoryPlayer[1].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[1].second, 100);

		EXPECT_EQ(playerData_.inventoryPlayer[2].first, item.get());
		EXPECT_EQ(playerData_.inventoryPlayer[2].second, 10);
	}
}
