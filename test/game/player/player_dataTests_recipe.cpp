// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/24/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "game/player/player_data.h"

namespace jactorio::game
{
	class PlayerDataRecipeTest : public testing::Test
	{
	protected:
		PlayerData playerData_{};
		data::DataManager dataManager_{};

		data::Item* itemProduct_ = nullptr;
		data::Item* item1_       = nullptr;
		data::Item* item2_       = nullptr;
		data::Item* itemSub1_    = nullptr;
		data::Item* itemSub2_    = nullptr;

		data::Recipe* finalRecipe_ = nullptr;
		data::Recipe* itemRecipe_  = nullptr;


		void SetupTestRecipe() {
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
			itemProduct_ = new data::Item();
			dataManager_.DataRawAdd("item-product", itemProduct_);

			item1_ = new data::Item();
			dataManager_.DataRawAdd("item-1", item1_);

			item2_ = new data::Item();
			dataManager_.DataRawAdd("item-2", item2_);

			itemSub1_ = new data::Item();
			dataManager_.DataRawAdd("item-sub-1", itemSub1_);

			itemSub2_ = new data::Item();
			dataManager_.DataRawAdd("item-sub-2", itemSub2_);

			finalRecipe_ = new data::Recipe();
			finalRecipe_->Set_ingredients({{"item-1", 3}, {"item-2", 1}});
			finalRecipe_->SetProduct({"item-product", 1});

			dataManager_.DataRawAdd("item-product-recipe", finalRecipe_);

			itemRecipe_ = new data::Recipe();
			itemRecipe_->Set_ingredients({{"item-sub-1", 5}, {"item-sub-2", 10}});
			itemRecipe_->SetProduct({"item-1", 2});

			dataManager_.DataRawAdd("item-1-recipe", itemRecipe_);
		}
	};


	TEST_F(PlayerDataRecipeTest, RecipeSelectRecipeGroup) {

		playerData_.RecipeGroupSelect(1);

		EXPECT_EQ(playerData_.RecipeGroupGetSelected(), 1);
	}


	TEST_F(PlayerDataRecipeTest, RecipeQueue) {
		// Queueing 2 recipes will remove the ingredients from the player inventory, but will not return any products
		// since recipe_craft_tick() is not called

		auto* item         = new data::Item();
		auto* item_product = new data::Item();


		// Register items
		dataManager_.DataRawAdd("item-1", item);

		dataManager_.DataRawAdd("item-product", item_product);

		// Register recipes
		auto recipe = data::Recipe();
		recipe.Set_ingredients({{"item-1", 2}});
		recipe.SetProduct({"item-product", 1});
		recipe.Set_craftingTime(1);

		// 10 of item in player inventory
		playerData_.inventoryPlayer[0] = {item, 10};

		// Queue 2 crafts
		playerData_.RecipeQueue(dataManager_, recipe);
		playerData_.RecipeQueue(dataManager_, recipe);

		// Used up 2 * 2 (4) items
		EXPECT_EQ(playerData_.inventoryPlayer[0].first, item);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 6);

		EXPECT_EQ(playerData_.inventoryPlayer[1].first, nullptr);


		//


		// Output items should be in slot index 1 after 60 ticks (1 second) for each item
		playerData_.RecipeCraftTick(dataManager_, 30);  // Not done yet

		EXPECT_EQ(playerData_.inventoryPlayer[1].first, nullptr);

		playerData_.RecipeCraftTick(dataManager_, 90);

		EXPECT_EQ(playerData_.inventoryPlayer[1].first, item_product);
		EXPECT_EQ(playerData_.inventoryPlayer[1].second, 2);
	}


	TEST_F(PlayerDataRecipeTest, RecipeCraftResurse) {
		// Should recursively craft the product, crafting intermediate products as necessary
		SetupTestRecipe();

		playerData_.inventoryPlayer[0] = {item2_, 1};

		playerData_.inventoryPlayer[1] = {itemSub1_, 10};
		playerData_.inventoryPlayer[2] = {itemSub2_, 20};

		playerData_.RecipeCraftR(dataManager_, *finalRecipe_);

		playerData_.RecipeCraftTick(dataManager_, 9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, item1_);  // 1 extra item 1 from crafting
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 1);

		EXPECT_EQ(playerData_.inventoryPlayer[1].first, itemProduct_);
		EXPECT_EQ(playerData_.inventoryPlayer[1].second, 1);

		EXPECT_EQ(playerData_.GetCraftingItemDeductions().size(), 0);
		EXPECT_EQ(playerData_.GetCraftingItemExtras().size(), 0);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCraftResurse2) {
		// Calculations for recursive crafting should also factor in the excess left by previous recipes
		SetupTestRecipe();

		// All ingredients should be completely used up
		playerData_.inventoryPlayer[0] = {item2_, 4};
		playerData_.inventoryPlayer[1] = {itemSub1_, 30};
		playerData_.inventoryPlayer[2] = {itemSub2_, 60};

		playerData_.RecipeCraftR(dataManager_, *finalRecipe_);
		playerData_.RecipeCraftR(dataManager_, *finalRecipe_);
		playerData_.RecipeCraftR(dataManager_, *finalRecipe_);

		// This should not craft item1 since there will be 3 in excess from the previous 3 crafting
		playerData_.RecipeCraftR(dataManager_, *finalRecipe_);

		playerData_.RecipeCraftTick(dataManager_, 9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, itemProduct_);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 4);

		EXPECT_EQ(playerData_.GetCraftingItemDeductions().size(), 0);
		EXPECT_EQ(playerData_.GetCraftingItemExtras().size(), 0);

		// Ensure there were no excess items
		for (int i = 1; i < PlayerData::kInventorySize; ++i) {
			EXPECT_EQ(playerData_.inventoryPlayer[i].first, nullptr);
		}
	}

	TEST_F(PlayerDataRecipeTest, RecipeCanCraft) {
		// Should recursively step through a recipe and determine that it can be crafted
		SetupTestRecipe();

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		playerData_.inventoryPlayer[0] = {item1_, 1};
		playerData_.inventoryPlayer[1] = {item2_, 2};

		playerData_.inventoryPlayer[2] = {itemSub1_, 5};
		playerData_.inventoryPlayer[3] = {itemSub2_, 10};

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(playerData_.RecipeCanCraft(dataManager_,*finalRecipe_, 1), true);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCanCraftInvalid) {
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		SetupTestRecipe();

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		playerData_.inventoryPlayer[0] = {item1_, 1};
		playerData_.inventoryPlayer[1] = {item2_, 2};

		playerData_.inventoryPlayer[2] = {itemSub1_, 5};
		// player_inventory[3] = {item_sub2, 10};  // Without this, the recipe cannot be crafted

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(playerData_.RecipeCanCraft(dataManager_,*finalRecipe_, 1), false);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCanCraftInvalid2) {
		// When encountering the same items, it must account for the fact it has already been used earlier
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		SetupTestRecipe();

		// Final product: item-1 + item-sub-2
		// item-2: item-sub-1 + item-sub-2

		playerData_.inventoryPlayer[1] = {itemSub1_, 10};
		playerData_.inventoryPlayer[2] = {itemSub2_, 5};

		EXPECT_EQ(playerData_.RecipeCanCraft(dataManager_,*finalRecipe_, 1), false);
	}

	TEST_F(PlayerDataRecipeTest, RecipeCraftFullInventory) {
		// If the inventory is full, the item will be held,
		// preventing any further crafting until a slot in the inventory if freed for the item to be returned

		// Should be able to craft
		SetupTestRecipe();
		playerData_.inventoryPlayer[0] = {item1_, 3};
		playerData_.inventoryPlayer[1] = {item2_, 1};

		EXPECT_TRUE(playerData_.RecipeCanCraft(dataManager_,*finalRecipe_, 1));
		playerData_.RecipeCraftR(dataManager_, *finalRecipe_);

		// Fill inventory so crafted item cannot be returned
		data::Item filler_item{};
		for (auto& slot : playerData_.inventoryPlayer) {
			slot.first  = &filler_item;
			slot.second = 1;
		}

		// Will not return item until slot is freed
		playerData_.RecipeCraftTick(dataManager_, 9999);
		playerData_.RecipeCraftTick(dataManager_, 9999);
		playerData_.RecipeCraftTick(dataManager_, 9999);

		playerData_.inventoryPlayer[0] = {nullptr, 0};
		playerData_.RecipeCraftTick(dataManager_, 9999);

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, itemProduct_);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 1);
	}
}
