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
			jactorio::data::data_category::item, "__core__/inventory-selected-cursor", cursor);
		
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
			jactorio::data::data_category::item, "__core__/inventory-selected-cursor", cursor);

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
			jactorio::data::data_category::item, "__core__/inventory-selected-cursor", cursor);

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

		set_clicked_inventory(0, 0);  // Pick up half

		EXPECT_EQ(player_inventory[0].first, nullptr);
		EXPECT_EQ(player_inventory[0].second, 0);

		const auto cursor_item = get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);
	}
}