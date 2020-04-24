// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/resource_entity.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace game
{
	class PlayerDataInventoryTest : public testing::Test
	{
		bool setup_cursor_ = false;

	protected:
		jactorio::game::Player_data player_data_{};

		jactorio::data::Item* cursor_ = nullptr;

		void TearDown() override {
			if (setup_cursor_) {
				jactorio::data::clear_data();
			}
		}

		///
		/// \brief Creates the cursor prototype which is hardcoded when an item is selected
		void setup_inventory_cursor() {
			setup_cursor_ = true;
			cursor_       = new jactorio::data::Item();

			data_raw_add(jactorio::game::Player_data::inventory_selected_cursor_iname, cursor_);
		}
	};

	TEST_F(PlayerDataInventoryTest, InventoryLclickSelectItemByReference) {
		// Left click on a slot picks up items by reference
		// The inventory slot becomes the cursor
		// The cursor holds the item
		setup_inventory_cursor();

		const auto item = std::make_unique<jactorio::data::Item>();


		// Position 3 should have the 50 items + item prototype after moving
		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 50;

		player_data_.inventory_click(0, 0, true, player_data_.inventory_player);

		EXPECT_EQ(player_data_.inventory_player[0].first, cursor_);
		EXPECT_EQ(player_data_.inventory_player[0].second, 0);

		const auto cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);
	}

	TEST_F(PlayerDataInventoryTest, InventoryDeselectReferencedItem) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot deselects the item
		setup_inventory_cursor();

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data_.inventory_player[0].first  = item.get();
			player_data_.inventory_player[0].second = 50;

			player_data_.inventory_click(0, 0, true, player_data_.inventory_player);  // Select
			player_data_.inventory_click(0, 0, true, player_data_.inventory_player);  // Deselect


			EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[0].second, 50);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
		player_data_.clear_player_inventory();
		// Right click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data_.inventory_player[0].first  = item.get();
			player_data_.inventory_player[0].second = 50;

			player_data_.inventory_click(0, 0, true, player_data_.inventory_player);  // Select
			player_data_.inventory_click(0, 1, true, player_data_.inventory_player);  // Deselect


			EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[0].second, 50);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}

	TEST_F(PlayerDataInventoryTest, InventoryDeselectReferencedItem2Inventories) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot in another inventory however will not deselect the item
		setup_inventory_cursor();

		const auto item = std::make_unique<jactorio::data::Item>();

		player_data_.clear_player_inventory();
		// Left click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data_.inventory_player[0].first  = item.get();
			player_data_.inventory_player[0].second = 50;

			player_data_.inventory_click(0, 0, true, player_data_.inventory_player);  // Select

			// Deselect into inv_2
			jactorio::data::item_stack inv_2[10];
			player_data_.inventory_click(0, 0, true, inv_2);  // Deselect

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 50);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);

			// Selected cursor should no longer exist in inventory_player
			EXPECT_EQ(player_data_.inventory_player[0].first, nullptr);
		}
		player_data_.clear_player_inventory();
		// Right click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data_.inventory_player[0].first  = item.get();
			player_data_.inventory_player[0].second = 50;

			player_data_.inventory_click(0, 0, true, player_data_.inventory_player);  // Select

			// Deselect into inv_2
			jactorio::data::item_stack inv_2[10];
			player_data_.inventory_click(0, 1, true, inv_2);  // Will NOT Deselect since in another inventory

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 1);

			// Cursor still holds 49
			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 49);

			// Selected cursor should STILL exist in inventory_player since not deselected
			EXPECT_NE(player_data_.inventory_player[0].first, nullptr);
		}
	}

	TEST_F(PlayerDataInventoryTest, InventoryMoveReferencedItem) {
		// Left click on a slot picks up items by reference
		// Left click on index 3 to drop off the item at index

		// Index 0 (origin) should be empty
		// Create the cursor prototype
		setup_inventory_cursor();

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data_.inventory_player[0].first  = item.get();
			player_data_.inventory_player[0].second = 50;

			player_data_.inventory_click(0, 0, true, player_data_.inventory_player);  // Select item
			player_data_.inventory_click(3, 0, true, player_data_.inventory_player);  // Drop item off


			EXPECT_EQ(player_data_.inventory_player[0].first, nullptr);
			EXPECT_EQ(player_data_.inventory_player[0].second, 0);

			EXPECT_EQ(player_data_.inventory_player[3].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[3].second, 50);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}


	TEST_F(PlayerDataInventoryTest, InventoryRclickSelectItemByUnique) {
		// Right click on a slot creates a new inventory slot in the cursor and places half from the inventory into it
		const auto item = std::make_unique<jactorio::data::Item>();


		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 40;

		player_data_.inventory_click(0, 1, true, player_data_.inventory_player);  // Pick up half

		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 20;

		const auto cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 20);
	}

	TEST_F(PlayerDataInventoryTest, InventoryDropSingleUniqueItem) {
		// Right click on item to pick up half into cursor
		// Right click on empty slot to drop 1 off from the cursor
		const auto item = std::make_unique<jactorio::data::Item>();

		// Drop one on another location
		{
			player_data_.inventory_player[0].first  = item.get();
			player_data_.inventory_player[0].second = 10;

			player_data_.inventory_click(0, 1, true, player_data_.inventory_player);  // Pick up half

			EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[0].second, 5);

			auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 5);


			player_data_.inventory_click(3, 1, true, player_data_.inventory_player);  // Drop 1 at index 3

			// Should remain unchanged
			EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[0].second, 5);

			// Loses 1
			cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Gain 1
			EXPECT_EQ(player_data_.inventory_player[3].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[3].second, 1);
		}

		player_data_.clear_player_inventory();
		player_data_.reset_inventory_variables();
		// Drop 1 on the original item stack where half was taken from
		{
			player_data_.inventory_player[0].first  = item.get();
			player_data_.inventory_player[0].second = 10;

			player_data_.inventory_click(0, 1, true, player_data_.inventory_player);  // Pick up half
			player_data_.
				inventory_click(0, 1, true, player_data_.inventory_player);  // Drop 1 one the stack it picked up from

			// Loses 1
			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Half (5) plus 1 = 6
			EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[0].second, 6);
		}
	}

	TEST_F(PlayerDataInventoryTest, InventoryDropStackUniqueItem) {
		// Right click on item to pick up half into cursor
		// Left click on empty slot to drop entire stack off from the cursor
		const auto item = std::make_unique<jactorio::data::Item>();


		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 10;


		player_data_.inventory_click(0, 1, true, player_data_.inventory_player);  // Pick up half

		EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[0].second, 5);

		auto cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 5);


		player_data_.inventory_click(3, 0, true, player_data_.inventory_player);  // Drop stack at index 3

		// Should remain unchanged
		EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[0].second, 5);

		// Empty
		cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Gain 10
		EXPECT_EQ(player_data_.inventory_player[3].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[3].second, 5);
	}

	TEST_F(PlayerDataInventoryTest, InventoryClickEmptySlot) {
		// Left click on empty slot
		// Should remain unchanged
		player_data_.inventory_player[0].first  = nullptr;
		player_data_.inventory_player[0].second = 0;

		player_data_.inventory_click(0, 0, true, player_data_.inventory_player);

		EXPECT_EQ(player_data_.inventory_player[0].first, nullptr);
		EXPECT_EQ(player_data_.inventory_player[0].second, 0);

		const auto cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);
	}


	// Increment / decrement selected item
	TEST_F(PlayerDataInventoryTest, IncrementSelectedItem) {
		// If player selects item by "unique" or "reference",
		// It should function the same as it only modifies the cursor item stack
		const auto item                         = std::make_unique<jactorio::data::Item>();
		item->stack_size                        = 50;
		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			player_data_.inventory_click(0, 1, true, player_data_.inventory_player);

			// Check if item was incremented
			EXPECT_EQ(player_data_.increment_selected_item(), true);

			EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[0].second, 5);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 6);  // This incremented by 1
		}

		// Drop item down at inv slot 1
		{
			player_data_.inventory_click(1, 0, true, player_data_.inventory_player);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_data_.inventory_player[1].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[1].second, 6);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST_F(PlayerDataInventoryTest, IncrementSelectedItemExceedItemStack) {
		// Attempting to increment an item exceeding item stack returns false and fails the increment
		const auto item                         = std::make_unique<jactorio::data::Item>();
		item->stack_size                        = 50;
		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 50;

		// Pickup
		player_data_.inventory_click(0, 0, true, player_data_.inventory_player);

		// Failed to add item: Item stack already full
		EXPECT_EQ(player_data_.increment_selected_item(), false);

		const auto cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);  // This unchanged
	}


	TEST_F(PlayerDataInventoryTest, DecrementSelectedItemUnique) {
		// If player selects item by "unique"
		// If decremented to 0, deselect the cursor item
		const auto item                         = std::make_unique<jactorio::data::Item>();
		item->stack_size                        = 50;
		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			player_data_.inventory_click(0, 1, true, player_data_.inventory_player);

			// Check if item was incremented
			EXPECT_EQ(player_data_.decrement_selected_item(), true);

			EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[0].second, 5);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);  // This decremented by 1
		}

		// Drop item down at inv slot 1
		{
			player_data_.inventory_click(1, 0, true, player_data_.inventory_player);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_data_.inventory_player[1].first, item.get());
			EXPECT_EQ(player_data_.inventory_player[1].second, 4);

			const auto cursor_item = player_data_.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST_F(PlayerDataInventoryTest, DeselectSelectedItem) {
		// If player selects item by "unique"
		// If decremented to 0, deselect the cursor item
		const auto item  = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 10;

		player_data_.inventory_click(0, 0, true, player_data_.inventory_player);

		EXPECT_TRUE(player_data_.deselect_selected_item());

		// Returned to where selection cursor was 
		EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[0].second, 10);

		const auto cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);
	}

	TEST_F(PlayerDataInventoryTest, DecrementSelectedItemReachZeroReference) {
		// Selected by reference
		// If decremented to 0, deselect the cursor item
		// If the selected item is empty after decrementing, return false
		setup_inventory_cursor();

		const auto item                         = std::make_unique<jactorio::data::Item>();
		item->stack_size                        = 50;
		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 1;

		// Pickup
		player_data_.inventory_click(0, 0, true, player_data_.inventory_player);

		EXPECT_EQ(player_data_.decrement_selected_item(), false);

		// Cursor is nullptr: no item selected
		const auto cursor_item = player_data_.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Should remove the selection cursor on the item
		EXPECT_EQ(player_data_.inventory_player[0].first, nullptr);
		EXPECT_EQ(player_data_.inventory_player[0].second, 0);
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySort) {
		const auto item  = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		const auto item2  = std::make_unique<jactorio::data::Item>();
		item2->stack_size = 10;

		// Item 1
		player_data_.inventory_player[0].first  = item.get();
		player_data_.inventory_player[0].second = 10;

		player_data_.inventory_player[10].first  = item.get();
		player_data_.inventory_player[10].second = 25;

		player_data_.inventory_player[20].first  = item.get();
		player_data_.inventory_player[20].second = 25;

		player_data_.inventory_player[13].first  = item.get();
		player_data_.inventory_player[13].second = 20;

		player_data_.inventory_player[14].first  = item.get();
		player_data_.inventory_player[14].second = 30;

		// Item 2
		player_data_.inventory_player[31].first  = item2.get();
		player_data_.inventory_player[31].second = 4;

		player_data_.inventory_player[32].first  = item2.get();
		player_data_.inventory_player[32].second = 6;

		player_data_.inventory_player[22].first  = item2.get();
		player_data_.inventory_player[22].second = 1;


		// Sorted inventory should be as follows
		// Item(count)
		// 1(50), 1(50), 1(10), 2(10), 2(1)
		player_data_.inventory_sort();

		EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[0].second, 50);
		EXPECT_EQ(player_data_.inventory_player[1].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[1].second, 50);
		EXPECT_EQ(player_data_.inventory_player[2].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[2].second, 10);

		EXPECT_EQ(player_data_.inventory_player[3].first, item2.get());
		EXPECT_EQ(player_data_.inventory_player[3].second, 10);
		EXPECT_EQ(player_data_.inventory_player[4].first, item2.get());
		EXPECT_EQ(player_data_.inventory_player[4].second, 1);
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySort2) {
		// Sorting will not move the item with inventory_selected_cursor_iname (to prevent breaking the inventory logic)
		setup_inventory_cursor();

		player_data_.inventory_player[10].first  = cursor_;
		player_data_.inventory_player[10].second = 0;


		player_data_.inventory_sort();

		EXPECT_EQ(player_data_.inventory_player[10].first, cursor_);
		EXPECT_EQ(player_data_.inventory_player[10].second, 0);

		// There should have been no new cursors created anywhere
		for (int i = 0; i < jactorio::game::Player_data::inventory_size; ++i) {
			if (i == 10)
				continue;
			EXPECT_NE(player_data_.inventory_player[i].first, cursor_);
		}
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySortFull) {
		// Sorting the inventory when it is full should also work
		const auto item  = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		for (auto& i : player_data_.inventory_player) {
			i.first  = item.get();
			i.second = 50;
		}

		player_data_.inventory_sort();


		// There should have been no new cursors created anywhere
		for (auto& i : player_data_.inventory_player) {
			EXPECT_EQ(i.first, item.get());
			EXPECT_EQ(i.second, 50);
		}
	}

	TEST_F(PlayerDataInventoryTest, PlayerInventorySortItemExcedingStack) {
		// If there is an item which exceeds its stack size, do not attempt to stack into it
		const auto item  = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		player_data_.inventory_player[10].first  = item.get();
		player_data_.inventory_player[10].second = 100;

		player_data_.inventory_player[11].first  = item.get();
		player_data_.inventory_player[11].second = 100;

		player_data_.inventory_player[12].first  = item.get();
		player_data_.inventory_player[12].second = 10;

		player_data_.inventory_sort();

		EXPECT_EQ(player_data_.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[0].second, 100);

		EXPECT_EQ(player_data_.inventory_player[1].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[1].second, 100);

		EXPECT_EQ(player_data_.inventory_player[2].first, item.get());
		EXPECT_EQ(player_data_.inventory_player[2].second, 10);
	}
}
