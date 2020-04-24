// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/22/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "game/player/player_data.h"

namespace game
{
	class PlayerDataRecipeTest : public testing::Test
	{
		bool setup_cursor_ = false;

	protected:
		jactorio::game::Player_data player_data_{};

		void TearDown() override {
			jactorio::data::clear_data();
		}

		jactorio::data::Item* item_product_ = nullptr;
		jactorio::data::Item* item1_        = nullptr;
		jactorio::data::Item* item2_        = nullptr;
		jactorio::data::Item* item_sub1_    = nullptr;
		jactorio::data::Item* item_sub2_    = nullptr;

		jactorio::data::Recipe* final_recipe_ = nullptr;
		jactorio::data::Recipe* item_recipe_  = nullptr;


		void setup_test_recipe() {
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
			item_product_ = new jactorio::data::Item();
			data_raw_add("item-product", item_product_);

			item1_ = new jactorio::data::Item();
			data_raw_add("item-1", item1_);

			item2_ = new jactorio::data::Item();
			data_raw_add("item-2", item2_);

			item_sub1_ = new jactorio::data::Item();
			data_raw_add("item-sub-1", item_sub1_);

			item_sub2_ = new jactorio::data::Item();
			data_raw_add("item-sub-2", item_sub2_);

			final_recipe_ = new jactorio::data::Recipe();
			final_recipe_->set_ingredients({{"item-1", 3}, {"item-2", 1}});
			final_recipe_->set_product({"item-product", 1});

			data_raw_add("item-product-recipe", final_recipe_);

			item_recipe_ = new jactorio::data::Recipe();
			item_recipe_->set_ingredients({{"item-sub-1", 5}, {"item-sub-2", 10}});
			item_recipe_->set_product({"item-1", 2});

			data_raw_add("item-1-recipe", item_recipe_);
		}
	};


	TEST_F(PlayerDataRecipeTest, RecipeSelectRecipeGroup) {

		player_data_.recipe_group_select(1);

		EXPECT_EQ(player_data_.recipe_group_get_selected(), 1);
	}


	TEST_F(PlayerDataRecipeTest, RecipeQueue) {
		// Queueing 2 recipes will remove the ingredients from the player inventory, but will not return any products
		// since recipe_craft_tick() is not called

		auto* item         = new jactorio::data::Item();
		auto* item_product = new jactorio::data::Item();


		// Register items
		data_raw_add("item-1", item);

		data_raw_add("item-product", item_product);

		// Register recipes
		auto recipe = jactorio::data::Recipe();
		recipe.set_ingredients({{"item-1", 2}});
		recipe.set_product({"item-product", 1});
		recipe.set_crafting_time(1);

		// 10 of item in player inventory
		player_data_.inventory_player[0] = {item, 10};

		// Queue 2 crafts
		player_data_.recipe_queue(&recipe);
		player_data_.recipe_queue(&recipe);

		// Used up 2 * 2 (4) items
		EXPECT_EQ(player_data_.inventory_player[0].first, item);
		EXPECT_EQ(player_data_.inventory_player[0].second, 6);

		EXPECT_EQ(player_data_.inventory_player[1].first, nullptr);


		//


		// Output items should be in slot index 1 after 60 ticks (1 second) for each item
		player_data_.recipe_craft_tick(30);  // Not done yet

		EXPECT_EQ(player_data_.inventory_player[1].first, nullptr);

		player_data_.recipe_craft_tick(90);

		EXPECT_EQ(player_data_.inventory_player[1].first, item_product);
		EXPECT_EQ(player_data_.inventory_player[1].second, 2);
	}


	TEST_F(PlayerDataRecipeTest, RecipeCraftResurse) {
		// Should recursively craft the product, crafting intermediate products as necessary
		setup_test_recipe();

		player_data_.inventory_player[0] = {item2_, 1};

		player_data_.inventory_player[1] = {item_sub1_, 10};
		player_data_.inventory_player[2] = {item_sub2_, 20};

		player_data_.recipe_craft_r(final_recipe_);

		player_data_.recipe_craft_tick(9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(player_data_.inventory_player[0].first, item1_);  // 1 extra item 1 from crafting
		EXPECT_EQ(player_data_.inventory_player[0].second, 1);

		EXPECT_EQ(player_data_.inventory_player[1].first, item_product_);
		EXPECT_EQ(player_data_.inventory_player[1].second, 1);

		EXPECT_EQ(player_data_.get_crafting_item_deductions().size(), 0);
		EXPECT_EQ(player_data_.get_crafting_item_extras().size(), 0);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCraftResurse2) {
		// Calculations for recursive crafting should also factor in the excess left by previous recipes
		setup_test_recipe();

		// All ingredients should be completely used up
		player_data_.inventory_player[0] = {item2_, 4};
		player_data_.inventory_player[1] = {item_sub1_, 30};
		player_data_.inventory_player[2] = {item_sub2_, 60};

		player_data_.recipe_craft_r(final_recipe_);
		player_data_.recipe_craft_r(final_recipe_);
		player_data_.recipe_craft_r(final_recipe_);

		// This should not craft item1 since there will be 3 in excess from the previous 3 crafting
		player_data_.recipe_craft_r(final_recipe_);

		player_data_.recipe_craft_tick(9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(player_data_.inventory_player[0].first, item_product_);
		EXPECT_EQ(player_data_.inventory_player[0].second, 4);

		EXPECT_EQ(player_data_.get_crafting_item_deductions().size(), 0);
		EXPECT_EQ(player_data_.get_crafting_item_extras().size(), 0);

		// Ensure there were no excess items
		for (int i = 1; i < jactorio::game::Player_data::inventory_size; ++i) {
			EXPECT_EQ(player_data_.inventory_player[i].first, nullptr);
		}
	}

	TEST_F(PlayerDataRecipeTest, RecipeCanCraft) {
		// Should recursively step through a recipe and determine that it can be crafted
		setup_test_recipe();

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		player_data_.inventory_player[0] = {item1_, 1};
		player_data_.inventory_player[1] = {item2_, 2};

		player_data_.inventory_player[2] = {item_sub1_, 5};
		player_data_.inventory_player[3] = {item_sub2_, 10};

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(player_data_.recipe_can_craft(final_recipe_, 1), true);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCanCraftInvalid) {
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		setup_test_recipe();

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		player_data_.inventory_player[0] = {item1_, 1};
		player_data_.inventory_player[1] = {item2_, 2};

		player_data_.inventory_player[2] = {item_sub1_, 5};
		// player_inventory[3] = {item_sub2, 10};  // Without this, the recipe cannot be crafted

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(player_data_.recipe_can_craft(final_recipe_, 1), false);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCanCraftInvalid2) {
		// When encountering the same items, it must account for the fact it has already been used earlier
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		setup_test_recipe();

		// Final product: item-1 + item-sub-2
		// item-2: item-sub-1 + item-sub-2

		player_data_.inventory_player[1] = {item_sub1_, 10};
		player_data_.inventory_player[2] = {item_sub2_, 5};

		EXPECT_EQ(player_data_.recipe_can_craft(final_recipe_, 1), false);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCraftFullInventory) {
		// If the inventory is full, the item will be held,
		// preventing any further crafting until a slot in the inventory if freed for the item to be returned

		// Should be able to craft
		setup_test_recipe();
		player_data_.inventory_player[0] = {item1_, 3};
		player_data_.inventory_player[1] = {item2_, 1};

		EXPECT_TRUE(player_data_.recipe_can_craft(final_recipe_, 1));
		player_data_.recipe_craft_r(final_recipe_);

		// Fill inventory so crafted item cannot be returned
		jactorio::data::Item filler_item{};
		for (auto& slot : player_data_.inventory_player) {
			slot.first = &filler_item;
			slot.second = 1;
		}	

		// Will not return item until slot is freed
		player_data_.recipe_craft_tick(9999);
		player_data_.recipe_craft_tick(9999);
		player_data_.recipe_craft_tick(9999);

		player_data_.inventory_player[0] = {nullptr, 0};
		player_data_.recipe_craft_tick(9999);

		EXPECT_EQ(player_data_.inventory_player[0].first, item_product_);
		EXPECT_EQ(player_data_.inventory_player[0].second, 1);
	}
}
