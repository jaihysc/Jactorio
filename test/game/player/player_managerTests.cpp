#include <gtest/gtest.h>

#include "game/player/player_manager.h"
#include "data/data_manager.h"
#include "core/resource_guard.h"

namespace game
{
	// Clears the player inventory for the next test
	void clear_player_inventory() {
		using namespace jactorio::game::player_manager;

		for (auto& i : player_inventory) {
			i.first = nullptr;
			i.second = 0;
		}
	}
	
	TEST(player_manager, inventory_lclick_select_item_by_reference) {
		// Left click on a slot picks up items by reference
		// The inventory slot becomes the cursor
		// The cursor holds the item
		namespace data_manager = jactorio::data::data_manager;
		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();
		
		// Setup
		auto data_manager_guard = jactorio::core::Resource_guard(data_manager::clear_data);

		// Create the cursor prototype
		auto* cursor = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, inventory_selected_cursor_iname, cursor);
		
		const auto item = std::make_unique<jactorio::data::Item>();

		
		// Position 3 should have the 50 items + item prototype after moving
		player_inventory[0].first = item.get();
		player_inventory[0].second = 50;

		set_clicked_inventory(0, 0);

		EXPECT_EQ(player_inventory[0].first, cursor);
		EXPECT_EQ(player_inventory[0].second, 0);

		const auto cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);
	}

	TEST(player_manager, inventory_deselect_referenced_item) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot deselects the item
		namespace data_manager = jactorio::data::data_manager;
		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		// Setup
		auto data_manager_guard = jactorio::core::Resource_guard(data_manager::clear_data);

		// Create the cursor prototype
		auto* cursor = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, inventory_selected_cursor_iname, cursor);

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_inventory[0].first = item.get();
			player_inventory[0].second = 50;

			set_clicked_inventory(0, 0);  // Select
			set_clicked_inventory(0, 0);  // Deselect


			EXPECT_EQ(player_inventory[0].first, item.get());
			EXPECT_EQ(player_inventory[0].second, 50);

			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
		clear_player_inventory();
		// Right click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_inventory[0].first = item.get();
			player_inventory[0].second = 50;

			set_clicked_inventory(0, 0);  // Select
			set_clicked_inventory(0, 1);  // Deselect


			EXPECT_EQ(player_inventory[0].first, item.get());
			EXPECT_EQ(player_inventory[0].second, 50);

			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}

	TEST(player_manager, inventory_move_referenced_item) {
		// Left click on a slot picks up items by reference
		// Left click on index 3 to drop off the item at index

		// Index 0 (origin) should be empty
		namespace data_manager = jactorio::data::data_manager;
		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		// Setup
		auto data_manager_guard = jactorio::core::Resource_guard(data_manager::clear_data);

		// Create the cursor prototype
		auto* cursor = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, inventory_selected_cursor_iname, cursor);

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_inventory[0].first = item.get();
			player_inventory[0].second = 50;

			set_clicked_inventory(0, 0);  // Select item
			set_clicked_inventory(3, 0);  // Drop item off


			EXPECT_EQ(player_inventory[0].first, nullptr);
			EXPECT_EQ(player_inventory[0].second, 0);

			EXPECT_EQ(player_inventory[3].first, item.get());
			EXPECT_EQ(player_inventory[3].second, 50);

			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}


	TEST(player_manager, inventory_Rclick_select_item_by_unique) {
		// Right click on a slot creates a new inventory slot in the cursor and places half from the inventory into it
		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		const auto item = std::make_unique<jactorio::data::Item>();


		player_inventory[0].first = item.get();
		player_inventory[0].second = 40;

		set_clicked_inventory(0, 1);  // Pick up half

		player_inventory[0].first = item.get();
		player_inventory[0].second = 20;
		
		const auto cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 20);
	}

	TEST(player_manager, inventory_drop_single_unique_item) {
		// Right click on item to pick up half into cursor
		// Right click on empty slot to drop 1 off from the cursor

		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		const auto item = std::make_unique<jactorio::data::Item>();

		// Drop one on another location
		{
			player_inventory[0].first = item.get();
			player_inventory[0].second = 10;

			set_clicked_inventory(0, 1);  // Pick up half

			EXPECT_EQ(player_inventory[0].first, item.get());
			EXPECT_EQ(player_inventory[0].second, 5);

			auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 5);


			set_clicked_inventory(3, 1);  // Drop 1 at index 3

			// Should remain unchanged
			EXPECT_EQ(player_inventory[0].first, item.get());
			EXPECT_EQ(player_inventory[0].second, 5);

			// Loses 1
			cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Gain 1
			EXPECT_EQ(player_inventory[3].first, item.get());
			EXPECT_EQ(player_inventory[3].second, 1);
		}

		clear_player_inventory();
		reset_inventory_variables();
		// Drop 1 on the original item stack where half was taken from
		{
			player_inventory[0].first = item.get();
			player_inventory[0].second = 10;

			set_clicked_inventory(0, 1);  // Pick up half
			set_clicked_inventory(0, 1);  // Drop 1 one the stack it picked up from

			// Loses 1
			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Half (5) plus 1 = 6
			EXPECT_EQ(player_inventory[0].first, item.get());
			EXPECT_EQ(player_inventory[0].second, 6);
		}
	}

	TEST(player_manager, inventory_drop_stack_unique_item) {
		// Right click on item to pick up half into cursor
		// Left click on empty slot to drop entire stack off from the cursor

		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		const auto item = std::make_unique<jactorio::data::Item>();


		player_inventory[0].first = item.get();
		player_inventory[0].second = 10;

		
		set_clicked_inventory(0, 1);  // Pick up half

		EXPECT_EQ(player_inventory[0].first, item.get());
		EXPECT_EQ(player_inventory[0].second, 5);

		auto cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 5);


		set_clicked_inventory(3, 0);  // Drop stack at index 3

		// Should remain unchanged
		EXPECT_EQ(player_inventory[0].first, item.get());
		EXPECT_EQ(player_inventory[0].second, 5);

		// Empty
		cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Gain 10
		EXPECT_EQ(player_inventory[3].first, item.get());
		EXPECT_EQ(player_inventory[3].second, 5);
	}

	TEST(player_manager, inventory_click_empty_slot) {
		// Left click on empty slot
		// Should remain unchanged

		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		player_inventory[0].first = nullptr;
		player_inventory[0].second = 0;

		set_clicked_inventory(0, 0);

		EXPECT_EQ(player_inventory[0].first, nullptr);
		EXPECT_EQ(player_inventory[0].second, 0);

		const auto cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);
	}


	// Increment / decrement selected item
	TEST(player_manager, increment_selected_item) {
		// If player selects item by "unique" or "reference",
		// It should function the same as it only modifies the cursor item stack

		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();


		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_inventory[0].first = item.get();
		player_inventory[0].second = 10;
		
		// Pickup
		{
			// Pick up 5 of item, now selected
			set_clicked_inventory(0, 1);

			// Check if item was incremented
			EXPECT_EQ(increment_selected_item(), true);

			EXPECT_EQ(player_inventory[0].first, item.get());
			EXPECT_EQ(player_inventory[0].second, 5);

			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 6);  // This incremented by 1
		}

		// Drop item down at inv slot 1
		{
			set_clicked_inventory(1, 0);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_inventory[1].first, item.get());
			EXPECT_EQ(player_inventory[1].second, 6);
			
			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST(player_manager, increment_selected_item_exceed_item_stack) {
		// Attempting to increment an item exceeding item stack returns false and fails the increment

		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();


		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_inventory[0].first = item.get();
		player_inventory[0].second = 50;

		// Pickup
		set_clicked_inventory(0, 0);

		// Failed to add item: Item stack already full
		EXPECT_EQ(increment_selected_item(), false);

		const auto cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);  // This unchanged
	}


	TEST(player_manager, decrement_selected_item_unique) {
		// If player selects item by "unique"
		// If decremented to 0, deselect the cursor item

		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();


		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_inventory[0].first = item.get();
		player_inventory[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			set_clicked_inventory(0, 1);

			// Check if item was incremented
			EXPECT_EQ(decrement_selected_item(), true);

			EXPECT_EQ(player_inventory[0].first, item.get());
			EXPECT_EQ(player_inventory[0].second, 5);

			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);  // This decremented by 1
		}

		// Drop item down at inv slot 1
		{
			set_clicked_inventory(1, 0);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_inventory[1].first, item.get());
			EXPECT_EQ(player_inventory[1].second, 4);

			const auto cursor_item = get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST(player_manager, decrement_selected_item_reach_zero_reference) {
		// Selected by reference
		// If decremented to 0, deselect the cursor item
		// If the selected item is empty after decrementing, return false

		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		// Setup
		auto data_manager_guard = jactorio::core::Resource_guard(jactorio::data::data_manager::clear_data);

		// Create the cursor prototype
		auto* cursor = new jactorio::data::Item();
		jactorio::data::data_manager::data_raw_add(
			jactorio::data::data_category::item, inventory_selected_cursor_iname, cursor);

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_inventory[0].first = item.get();
		player_inventory[0].second = 1;

		// Pickup
		set_clicked_inventory(0, 0);

		EXPECT_EQ(decrement_selected_item(), false);

		// Cursor is nullptr: no item selected
		const auto cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Should remove the selection cursor on the item
		EXPECT_EQ(player_inventory[0].first, nullptr);
		EXPECT_EQ(player_inventory[0].second, 0);
	}

	TEST(player_maanger, player_inventory_sort) {
		using namespace jactorio::game::player_manager;

		clear_player_inventory();
		reset_inventory_variables();

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		const auto item2 = std::make_unique<jactorio::data::Item>();
		item2->stack_size = 10;

		// Item 1
		player_inventory[0].first = item.get();
		player_inventory[0].second = 10;

		player_inventory[10].first = item.get();
		player_inventory[10].second = 25;

		player_inventory[20].first = item.get();
		player_inventory[20].second = 25;

		player_inventory[13].first = item.get();
		player_inventory[13].second = 20;

		player_inventory[14].first = item.get();
		player_inventory[14].second = 30;

		// Item 2
		player_inventory[31].first = item2.get();
		player_inventory[31].second = 4;

		player_inventory[32].first = item2.get();
		player_inventory[32].second = 6;
		
		player_inventory[22].first = item2.get();
		player_inventory[22].second = 1;

		
		// Sorted inventory should be as follows
		// Item(count)
		// 1(50), 1(50), 1(10), 2(10), 2(1)
		player_inventory_sort();

		EXPECT_EQ(player_inventory[0].first, item.get());
		EXPECT_EQ(player_inventory[0].second, 50);
		EXPECT_EQ(player_inventory[1].first, item.get());
		EXPECT_EQ(player_inventory[1].second, 50);
		EXPECT_EQ(player_inventory[2].first, item.get());
		EXPECT_EQ(player_inventory[2].second, 10);

		EXPECT_EQ(player_inventory[3].first, item2.get());
		EXPECT_EQ(player_inventory[3].second, 10);
		EXPECT_EQ(player_inventory[4].first, item2.get());
		EXPECT_EQ(player_inventory[4].second, 1);
	}

	TEST(player_maanger, player_inventory_sort2) {
		// Sorting will not move the item with inventory_selected_cursor_iname (to prevent breaking the inventory logic)
		using namespace jactorio::game::player_manager;
		namespace data_manager = jactorio::data::data_manager;
		
		clear_player_inventory();
		reset_inventory_variables();

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		
		// Create the cursor prototype
		auto* cursor = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, inventory_selected_cursor_iname, cursor);

		player_inventory[10].first = cursor;
		player_inventory[10].second = 0;


		player_inventory_sort();

		EXPECT_EQ(player_inventory[10].first, cursor);
		EXPECT_EQ(player_inventory[10].second, 0);

		// There should have been no new cursors created anywhere
		for (int i = 0; i < player_inventory_size; ++i) {
			if (i == 10)
				continue;
			EXPECT_NE(player_inventory[i].first, cursor);
		}
	}

	TEST(player_maanger, player_inventory_sort_item_exceding_stack) {
		// If there is an item which exceeds its stack size, do not attempt to stack into it
		using namespace jactorio::game::player_manager;
		namespace data_manager = jactorio::data::data_manager;

		clear_player_inventory();
		reset_inventory_variables();

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		player_inventory[10].first = item.get();
		player_inventory[10].second = 100;

		player_inventory[11].first = item.get();
		player_inventory[11].second = 100;

		player_inventory[12].first = item.get();
		player_inventory[12].second = 10;

		player_inventory_sort();

		EXPECT_EQ(player_inventory[0].first, item.get());
		EXPECT_EQ(player_inventory[0].second, 100);

		EXPECT_EQ(player_inventory[1].first, item.get());
		EXPECT_EQ(player_inventory[1].second, 100);

		EXPECT_EQ(player_inventory[2].first, item.get());
		EXPECT_EQ(player_inventory[2].second, 10);
	}
	
	//
	//
	//
	//
	// RECIPE MENU
	//
	//
	//
	//
	
	TEST(player_manager, recipe_select_recipe_group) {
		using namespace jactorio::game::player_manager;
		select_recipe_group(1);

		EXPECT_EQ(get_selected_recipe_group(), 1);
	}
}
