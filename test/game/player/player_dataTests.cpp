// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "core/resource_guard.h"
#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/tile/tile.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace game
{
	//
	//
	//
	//
	// PLAYER INVENTORY MENU
	//
	//
	//
	//

	// Create the cursor prototype hardcoded when an item is selected
#define INVENTORY_CURSOR\
	auto* cursor = new jactorio::data::Item();\
		jactorio::data::data_manager::data_raw_add(jactorio::game::Player_data::inventory_selected_cursor_iname, \
			cursor);
	TEST(player_data, inventory_lclick_select_item_by_reference) {
		// Left click on a slot picks up items by reference
		// The inventory slot becomes the cursor
		// The cursor holds the item
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();


		// Position 3 should have the 50 items + item prototype after moving
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 50;

		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		EXPECT_EQ(player_data.inventory_player[0].first, cursor);
		EXPECT_EQ(player_data.inventory_player[0].second, 0);

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);
	}

	TEST(player_data, inventory_deselect_referenced_item) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot deselects the item
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select
			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Deselect


			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
		player_data.clear_player_inventory();
		// Right click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select
			player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Deselect


			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}

	TEST(player_data, inventory_deselect_referenced_item_2_inventories) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot in another inventory however will not deselect the item
		jactorio::game::Player_data player_data{};

		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();

		player_data.clear_player_inventory();
		// Left click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select

			// Deselect into inv_2
			jactorio::data::item_stack inv_2[10];
			player_data.inventory_click(0, 0, true, inv_2);  // Deselect

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);

			// Selected cursor should no longer exist in inventory_player
			EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
		}
		player_data.clear_player_inventory();
		// Right click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select

			// Deselect into inv_2
			jactorio::data::item_stack inv_2[10];
			player_data.inventory_click(0, 1, true, inv_2);  // Will NOT Deselect since in another inventory

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 1);

			// Cursor still holds 49
			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 49);

			// Selected cursor should STILL exist in inventory_player since not deselected
			EXPECT_NE(player_data.inventory_player[0].first, nullptr);
		}
	}

	TEST(player_data, inventory_move_referenced_item) {
		// Left click on a slot picks up items by reference
		// Left click on index 3 to drop off the item at index

		// Index 0 (origin) should be empty
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		// Create the cursor prototype
		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select item
			player_data.inventory_click(3, 0, true, player_data.inventory_player);  // Drop item off


			EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
			EXPECT_EQ(player_data.inventory_player[0].second, 0);

			EXPECT_EQ(player_data.inventory_player[3].first, item.get());
			EXPECT_EQ(player_data.inventory_player[3].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}


	TEST(player_data, inventory_Rclick_select_item_by_unique) {
		// Right click on a slot creates a new inventory slot in the cursor and places half from the inventory into it
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();


		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 40;

		player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half

		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 20;

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 20);
	}

	TEST(player_data, inventory_drop_single_unique_item) {
		// Right click on item to pick up half into cursor
		// Right click on empty slot to drop 1 off from the cursor
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();

		// Drop one on another location
		{
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 10;

			player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half

			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 5);


			player_data.inventory_click(3, 1, true, player_data.inventory_player);  // Drop 1 at index 3

			// Should remain unchanged
			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			// Loses 1
			cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Gain 1
			EXPECT_EQ(player_data.inventory_player[3].first, item.get());
			EXPECT_EQ(player_data.inventory_player[3].second, 1);
		}

		player_data.clear_player_inventory();
		player_data.reset_inventory_variables();
		// Drop 1 on the original item stack where half was taken from
		{
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 10;

			player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half
			player_data.
				inventory_click(0, 1, true, player_data.inventory_player);  // Drop 1 one the stack it picked up from

			// Loses 1
			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Half (5) plus 1 = 6
			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 6);
		}
	}

	TEST(player_data, inventory_drop_stack_unique_item) {
		// Right click on item to pick up half into cursor
		// Left click on empty slot to drop entire stack off from the cursor
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();


		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;


		player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half

		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 5);

		auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 5);


		player_data.inventory_click(3, 0, true, player_data.inventory_player);  // Drop stack at index 3

		// Should remain unchanged
		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 5);

		// Empty
		cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Gain 10
		EXPECT_EQ(player_data.inventory_player[3].first, item.get());
		EXPECT_EQ(player_data.inventory_player[3].second, 5);
	}

	TEST(player_data, inventory_click_empty_slot) {
		// Left click on empty slot
		// Should remain unchanged
		jactorio::game::Player_data player_data{};

		player_data.inventory_player[0].first = nullptr;
		player_data.inventory_player[0].second = 0;

		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
		EXPECT_EQ(player_data.inventory_player[0].second, 0);

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);
	}


	// Increment / decrement selected item
	TEST(player_data, increment_selected_item) {
		// If player selects item by "unique" or "reference",
		// It should function the same as it only modifies the cursor item stack
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			player_data.inventory_click(0, 1, true, player_data.inventory_player);

			// Check if item was incremented
			EXPECT_EQ(player_data.increment_selected_item(), true);

			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 6);  // This incremented by 1
		}

		// Drop item down at inv slot 1
		{
			player_data.inventory_click(1, 0, true, player_data.inventory_player);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_data.inventory_player[1].first, item.get());
			EXPECT_EQ(player_data.inventory_player[1].second, 6);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST(player_data, increment_selected_item_exceed_item_stack) {
		// Attempting to increment an item exceeding item stack returns false and fails the increment
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 50;

		// Pickup
		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		// Failed to add item: Item stack already full
		EXPECT_EQ(player_data.increment_selected_item(), false);

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);  // This unchanged
	}


	TEST(player_data, decrement_selected_item_unique) {
		// If player selects item by "unique"
		// If decremented to 0, deselect the cursor item
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			player_data.inventory_click(0, 1, true, player_data.inventory_player);

			// Check if item was incremented
			EXPECT_EQ(player_data.decrement_selected_item(), true);

			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);  // This decremented by 1
		}

		// Drop item down at inv slot 1
		{
			player_data.inventory_click(1, 0, true, player_data.inventory_player);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_data.inventory_player[1].first, item.get());
			EXPECT_EQ(player_data.inventory_player[1].second, 4);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST(player_data, decrement_selected_item_reach_zero_reference) {
		// Selected by reference
		// If decremented to 0, deselect the cursor item
		// If the selected item is empty after decrementing, return false
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 1;

		// Pickup
		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		EXPECT_EQ(player_data.decrement_selected_item(), false);

		// Cursor is nullptr: no item selected
		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Should remove the selection cursor on the item
		EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
		EXPECT_EQ(player_data.inventory_player[0].second, 0);
	}

	TEST(player_data, player_inventory_sort) {
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		const auto item2 = std::make_unique<jactorio::data::Item>();
		item2->stack_size = 10;

		// Item 1
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;

		player_data.inventory_player[10].first = item.get();
		player_data.inventory_player[10].second = 25;

		player_data.inventory_player[20].first = item.get();
		player_data.inventory_player[20].second = 25;

		player_data.inventory_player[13].first = item.get();
		player_data.inventory_player[13].second = 20;

		player_data.inventory_player[14].first = item.get();
		player_data.inventory_player[14].second = 30;

		// Item 2
		player_data.inventory_player[31].first = item2.get();
		player_data.inventory_player[31].second = 4;

		player_data.inventory_player[32].first = item2.get();
		player_data.inventory_player[32].second = 6;

		player_data.inventory_player[22].first = item2.get();
		player_data.inventory_player[22].second = 1;


		// Sorted inventory should be as follows
		// Item(count)
		// 1(50), 1(50), 1(10), 2(10), 2(1)
		player_data.inventory_sort();

		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 50);
		EXPECT_EQ(player_data.inventory_player[1].first, item.get());
		EXPECT_EQ(player_data.inventory_player[1].second, 50);
		EXPECT_EQ(player_data.inventory_player[2].first, item.get());
		EXPECT_EQ(player_data.inventory_player[2].second, 10);

		EXPECT_EQ(player_data.inventory_player[3].first, item2.get());
		EXPECT_EQ(player_data.inventory_player[3].second, 10);
		EXPECT_EQ(player_data.inventory_player[4].first, item2.get());
		EXPECT_EQ(player_data.inventory_player[4].second, 1);
	}

	TEST(player_data, player_inventory_sort2) {
		// Sorting will not move the item with inventory_selected_cursor_iname (to prevent breaking the inventory logic)
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		player_data.inventory_player[10].first = cursor;
		player_data.inventory_player[10].second = 0;


		player_data.inventory_sort();

		EXPECT_EQ(player_data.inventory_player[10].first, cursor);
		EXPECT_EQ(player_data.inventory_player[10].second, 0);

		// There should have been no new cursors created anywhere
		for (int i = 0; i < jactorio::game::Player_data::inventory_size; ++i) {
			if (i == 10)
				continue;
			EXPECT_NE(player_data.inventory_player[i].first, cursor);
		}
	}

	TEST(player_data, player_inventory_sort_full) {
		// Sorting the inventory when it is full should also work
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		for (auto& i : player_data.inventory_player) {
			i.first = item.get();
			i.second = 50;
		}

		player_data.inventory_sort();


		// There should have been no new cursors created anywhere
		for (auto& i : player_data.inventory_player) {
			EXPECT_EQ(i.first, item.get());
			EXPECT_EQ(i.second, 50);
		}
	}

	TEST(player_data, player_inventory_sort_item_exceding_stack) {
		// If there is an item which exceeds its stack size, do not attempt to stack into it
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		player_data.inventory_player[10].first = item.get();
		player_data.inventory_player[10].second = 100;

		player_data.inventory_player[11].first = item.get();
		player_data.inventory_player[11].second = 100;

		player_data.inventory_player[12].first = item.get();
		player_data.inventory_player[12].second = 10;

		player_data.inventory_sort();

		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 100);

		EXPECT_EQ(player_data.inventory_player[1].first, item.get());
		EXPECT_EQ(player_data.inventory_player[1].second, 100);

		EXPECT_EQ(player_data.inventory_player[2].first, item.get());
		EXPECT_EQ(player_data.inventory_player[2].second, 10);
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

	TEST(player_data, recipe_select_recipe_group) {
		jactorio::game::Player_data player_data{};
		player_data.recipe_group_select(1);

		EXPECT_EQ(player_data.recipe_group_get_selected(), 1);
	}


	TEST(player_data, recipe_queue) {
		// Queueing 2 recipes will remove the ingredients from the player inventory, but will not return any products
		// since recipe_craft_tick() is not called

		jactorio::game::Player_data player_data{};

		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		const auto item = new jactorio::data::Item();
		const auto item_product = new jactorio::data::Item();


		// Register items
		jactorio::data::data_manager::data_raw_add("item-1", item);

		jactorio::data::data_manager::data_raw_add("item-product", item_product);

		// Register recipes
		auto recipe = jactorio::data::Recipe();
		recipe.set_ingredients({{"item-1", 2}});
		recipe.set_product({"item-product", 1});
		recipe.set_crafting_time(1);

		// 10 of item in player inventory
		player_data.inventory_player[0] = {item, 10};

		// Queue 2 crafts
		player_data.recipe_queue(&recipe);
		player_data.recipe_queue(&recipe);

		// Used up 2 * 2 (4) items
		EXPECT_EQ(player_data.inventory_player[0].first, item);
		EXPECT_EQ(player_data.inventory_player[0].second, 6);

		EXPECT_EQ(player_data.inventory_player[1].first, nullptr);


		//


		// Output items should be in slot index 1 after 60 ticks (1 second) for each item
		player_data.recipe_craft_tick(30);  // Not done yet

		EXPECT_EQ(player_data.inventory_player[1].first, nullptr);

		player_data.recipe_craft_tick(90);

		EXPECT_EQ(player_data.inventory_player[1].first, item_product);
		EXPECT_EQ(player_data.inventory_player[1].second, 2);
	}


	// Creates a crafting recipes with the following crafting hierarchy
	/*
	 *        ---
	 *        |1|product
	 *   --3--- ---1--
	 *   |2|item1  | |item2
	 * -5- -10     ---
	 * | | | |
	 * --- ---
	 * sub1  sub2
	 */
	// 3 item1 + 1 item2 -> 1 product
	// 5 sub1 + 10 sub2 -> 2 item1
#define RECIPE_TEST_RECIPE\
	auto* item_product = new jactorio::data::Item();\
	data_manager::data_raw_add("item-product", item_product);\
	\
	auto* item1 = new jactorio::data::Item();\
	data_manager::data_raw_add("item-1", item1);\
	\
	auto* item2 = new jactorio::data::Item();\
	data_manager::data_raw_add("item-2", item2);\
	\
	auto* item_sub1 = new jactorio::data::Item();\
	data_manager::data_raw_add("item-sub-1", item_sub1);\
	\
	auto* item_sub2 = new jactorio::data::Item();\
	data_manager::data_raw_add("item-sub-2", item_sub2);\
	\
	auto* final_recipe = new jactorio::data::Recipe();\
	final_recipe->set_ingredients({{"item-1", 3}, {"item-2", 1}});\
	final_recipe->set_product({"item-product", 1});\
	\
	data_manager::data_raw_add("item-product-recipe", final_recipe);\
	\
	auto* item_recipe = new jactorio::data::Recipe();\
	item_recipe->set_ingredients({{"item-sub-1", 5}, {"item-sub-2", 10}});\
	item_recipe->set_product({"item-1", 2});\
	\
	data_manager::data_raw_add("item-1-recipe", item_recipe);

	TEST(player_data, recipe_craft_resurse) {
		// Should recursively craft the product, crafting intermediate products as necessary
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		RECIPE_TEST_RECIPE

		player_data.inventory_player[0] = {item2, 1};

		player_data.inventory_player[1] = {item_sub1, 10};
		player_data.inventory_player[2] = {item_sub2, 20};

		player_data.recipe_craft_r(final_recipe);

		player_data.recipe_craft_tick(9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(player_data.inventory_player[0].first, item1);  // 1 extra item 1 from crafting
		EXPECT_EQ(player_data.inventory_player[0].second, 1);

		EXPECT_EQ(player_data.inventory_player[1].first, item_product);
		EXPECT_EQ(player_data.inventory_player[1].second, 1);

		EXPECT_EQ(player_data.get_crafting_item_deductions().size(), 0);
		EXPECT_EQ(player_data.get_crafting_item_extras().size(), 0);
	}

	TEST(player_data, recipe_craft_resurse2) {
		// Calculations for recursive crafting should also factor in the excess left by previous recipes

		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		RECIPE_TEST_RECIPE

		// All ingredients should be completely used up
		player_data.inventory_player[0] = {item2, 4};
		player_data.inventory_player[1] = {item_sub1, 30};
		player_data.inventory_player[2] = {item_sub2, 60};

		player_data.recipe_craft_r(final_recipe);
		player_data.recipe_craft_r(final_recipe);
		player_data.recipe_craft_r(final_recipe);

		// This should not craft item1 since there will be 3 in excess from the previous 3 crafting
		player_data.recipe_craft_r(final_recipe);

		player_data.recipe_craft_tick(9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(player_data.inventory_player[0].first, item_product);
		EXPECT_EQ(player_data.inventory_player[0].second, 4);

		EXPECT_EQ(player_data.get_crafting_item_deductions().size(), 0);
		EXPECT_EQ(player_data.get_crafting_item_extras().size(), 0);

		// Ensure there were no excess items
		for (int i = 1; i < jactorio::game::Player_data::inventory_size; ++i) {
			EXPECT_EQ(player_data.inventory_player[i].first, nullptr);
		}
	}

	TEST(player_data, recipe_can_craft) {
		// Should recursively step through a recipe and determine that it can be crafted
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		RECIPE_TEST_RECIPE

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		player_data.inventory_player[0] = {item1, 1};
		player_data.inventory_player[1] = {item2, 2};

		player_data.inventory_player[2] = {item_sub1, 5};
		player_data.inventory_player[3] = {item_sub2, 10};

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(player_data.recipe_can_craft(final_recipe, 1), true);
	}

	TEST(player_data, recipe_can_craft_invalid) {
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		RECIPE_TEST_RECIPE

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		player_data.inventory_player[0] = {item1, 1};
		player_data.inventory_player[1] = {item2, 2};

		player_data.inventory_player[2] = {item_sub1, 5};
		// player_inventory[3] = {item_sub2, 10};  // Without this, the recipe cannot be crafted

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(player_data.recipe_can_craft(final_recipe, 1), false);
	}

	TEST(player_data, recipe_can_craft_invalid2) {
		// When encountering the same items, it must account for the fact it has already been used earlier
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		RECIPE_TEST_RECIPE

		// Final product: item-1 + item-sub-2
		// item-2: item-sub-1 + item-sub-2

		player_data.inventory_player[1] = {item_sub1, 10};
		player_data.inventory_player[2] = {item_sub2, 5};

		EXPECT_EQ(player_data.recipe_can_craft(final_recipe, 1), false);
	}

#undef RECIPE_TEST_RECIPE
}
