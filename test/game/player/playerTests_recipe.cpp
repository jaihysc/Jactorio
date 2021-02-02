// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/player.h"

#include "jactorioTests.h"

namespace jactorio::game
{
    class PlayerRecipeTest : public testing::Test
    {
    protected:
        Player playerData_;
        Player::Inventory& playerInv_  = playerData_.inventory;
        Player::Crafting& playerCraft_ = playerData_.crafting;

        data::PrototypeManager dataManager_;

        proto::Item* itemProduct_ = nullptr;
        proto::Item* item1_       = nullptr;
        proto::Item* item2_       = nullptr;
        proto::Item* itemSub1_    = nullptr;
        proto::Item* itemSub2_    = nullptr;

        proto::Recipe* finalRecipe_ = nullptr;
        proto::Recipe* itemRecipe_  = nullptr;


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
            itemProduct_ = &dataManager_.AddProto<proto::Item>("item-product");

            item1_ = &dataManager_.AddProto<proto::Item>("item-1");
            item2_ = &dataManager_.AddProto<proto::Item>("item-2");

            itemSub1_ = &dataManager_.AddProto<proto::Item>("item-sub-1");
            itemSub2_ = &dataManager_.AddProto<proto::Item>("item-sub-2");

            finalRecipe_ = &dataManager_.AddProto<proto::Recipe>("item-product-recipe");
            finalRecipe_->Set_ingredients({{"item-1", 3}, {"item-2", 1}});
            finalRecipe_->product      = {"item-product", 1};
            finalRecipe_->craftingTime = 1.5;

            itemRecipe_ = &dataManager_.AddProto<proto::Recipe>("item-1-recipe");
            itemRecipe_->Set_ingredients({{"item-sub-1", 5}, {"item-sub-2", 10}});
            itemRecipe_->product       = {"item-1", 2};
            finalRecipe_->craftingTime = 3.1;
        }
    };


    TEST_F(PlayerRecipeTest, RecipeSelectRecipeGroup) {
        playerCraft_.RecipeGroupSelect(1);
        EXPECT_EQ(playerCraft_.RecipeGroupGetSelected(), 1);
    }


    TEST_F(PlayerRecipeTest, RecipeQueue) {
        // Queueing 2 recipes will remove the ingredients from the player inventory, but will not return any products
        // since recipe_craft_tick() is not called

        // Register items
        auto& item         = dataManager_.AddProto<proto::Item>("item-1");
        auto& item_product = dataManager_.AddProto<proto::Item>("item-product");

        // Register recipes
        auto recipe = proto::Recipe();
        recipe.Set_ingredients({{"item-1", 2}});
        recipe.product = {"item-product", 1};
        recipe.Set_craftingTime(1);

        // 10 of item in player inventory
        playerInv_.inventory[0] = {&item, 10};

        // Queue 2 crafts
        playerCraft_.QueueRecipe(dataManager_, recipe);
        playerCraft_.QueueRecipe(dataManager_, recipe);

        // Used up 2 * 2 (4) items
        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 6);

        EXPECT_EQ(playerInv_.inventory[1].item, nullptr);


        // Output items should be in slot index 1 after 60 ticks (1 second) for each item
        playerCraft_.RecipeCraftTick(dataManager_, 30); // Not done yet

        EXPECT_EQ(playerInv_.inventory[1].item, nullptr);

        playerCraft_.RecipeCraftTick(dataManager_, 90);

        EXPECT_EQ(playerInv_.inventory[1].item, &item_product);
        EXPECT_EQ(playerInv_.inventory[1].count, 2);
    }


    TEST_F(PlayerRecipeTest, RecipeCraftResurse) {
        // Should recursively craft the product, crafting intermediate products as necessary
        SetupTestRecipe();

        playerInv_.inventory[0] = {item2_, 1};

        playerInv_.inventory[1] = {itemSub1_, 10};
        playerInv_.inventory[2] = {itemSub2_, 20};

        playerCraft_.RecipeCraftR(dataManager_, *finalRecipe_);

        playerCraft_.RecipeCraftTick(dataManager_, (2 * 3.1 + 1.5) * kGameHertz);

        EXPECT_EQ(playerInv_.inventory[0].item, item1_); // 1 extra item 1 from crafting
        EXPECT_EQ(playerInv_.inventory[0].count, 1);

        EXPECT_EQ(playerInv_.inventory[1].item, itemProduct_);
        EXPECT_EQ(playerInv_.inventory[1].count, 1);

        EXPECT_EQ(playerCraft_.GetCraftingItemDeductions().size(), 0);
        EXPECT_EQ(playerCraft_.GetCraftingItemExtras().size(), 0);
    }

    TEST_F(PlayerRecipeTest, RecipeCraftResurse2) {
        // Calculations for recursive crafting should also factor in the excess left by previous recipes
        SetupTestRecipe();

        // All ingredients should be completely used up
        playerInv_.inventory[0] = {item2_, 4};
        playerInv_.inventory[1] = {itemSub1_, 30};
        playerInv_.inventory[2] = {itemSub2_, 60};

        playerCraft_.RecipeCraftR(dataManager_, *finalRecipe_);
        playerCraft_.RecipeCraftR(dataManager_, *finalRecipe_);
        playerCraft_.RecipeCraftR(dataManager_, *finalRecipe_);

        // This should not craft item1 since there will be 3 in excess from the previous 3 crafting
        playerCraft_.RecipeCraftR(dataManager_, *finalRecipe_);

        playerCraft_.RecipeCraftTick(dataManager_, 9999); // Should be enough ticks to finish crafting

        EXPECT_EQ(playerInv_.inventory[0].item, itemProduct_);
        EXPECT_EQ(playerInv_.inventory[0].count, 4);

        EXPECT_EQ(playerCraft_.GetCraftingItemDeductions().size(), 0);
        EXPECT_EQ(playerCraft_.GetCraftingItemExtras().size(), 0);

        // Ensure there were no excess items
        for (std::size_t i = 1; i < playerInv_.inventory.size(); ++i) {
            EXPECT_EQ(playerInv_.inventory[i].item, nullptr);
        }
    }

    TEST_F(PlayerRecipeTest, RecipeCanCraft) {
        // Should recursively step through a recipe and determine that it can be crafted
        SetupTestRecipe();

        // 3-B + 1-C -> 1-A
        // 5-B1 + 10-B2 -> 2-B
        playerInv_.inventory[0] = {item1_, 1};
        playerInv_.inventory[1] = {item2_, 2};

        playerInv_.inventory[2] = {itemSub1_, 5};
        playerInv_.inventory[3] = {itemSub2_, 10};

        // Crafting item-product:
        // Item 1 requires one batch to be crafted
        // Crafting requires for item 2 met
        EXPECT_EQ(playerCraft_.RecipeCanCraft(dataManager_, *finalRecipe_, 1), true);
    }

    TEST_F(PlayerRecipeTest, RecipeCanCraftInvalid) {
        // Should recursively step through a recipe and determine that it canNOT in fact be crafted
        SetupTestRecipe();

        // 3-B + 1-C -> 1-A
        // 5-B1 + 10-B2 -> 2-B
        playerInv_.inventory[0] = {item1_, 1};
        playerInv_.inventory[1] = {item2_, 2};

        playerInv_.inventory[2] = {itemSub1_, 5};
        // player_inventory[3] = {item_sub2, 10};  // Without this, the recipe cannot be crafted

        // Crafting item-product:
        // Item 1 requires one batch to be crafted
        // Crafting requires for item 2 met
        EXPECT_EQ(playerCraft_.RecipeCanCraft(dataManager_, *finalRecipe_, 1), false);
    }

    TEST_F(PlayerRecipeTest, RecipeCanCraftInvalid2) {
        // When encountering the same items, it must account for the fact it has already been used earlier
        // Should recursively step through a recipe and determine that it canNOT in fact be crafted
        SetupTestRecipe();

        // Final product: item-1 + item-sub-2
        // item-2: item-sub-1 + item-sub-2

        playerInv_.inventory[1] = {itemSub1_, 10};
        playerInv_.inventory[2] = {itemSub2_, 5};

        EXPECT_EQ(playerCraft_.RecipeCanCraft(dataManager_, *finalRecipe_, 1), false);
    }

    TEST_F(PlayerRecipeTest, RecipeCraftFullInventory) {
        // If the inventory is full, the item will be held,
        // preventing any further crafting until a slot in the inventory if freed for the item to be returned

        SetupTestRecipe();
        playerInv_.inventory[0] = {item1_, 3};
        playerInv_.inventory[1] = {item2_, 1};

        EXPECT_TRUE(playerCraft_.RecipeCanCraft(dataManager_, *finalRecipe_, 1));
        playerCraft_.RecipeCraftR(dataManager_, *finalRecipe_);

        // Fill inventory so crafted item cannot be returned
        proto::Item filler_item;
        for (auto& slot : playerInv_.inventory) {
            slot.item  = &filler_item;
            slot.count = 1;
        }

        // Will not return item until slot is freed
        playerCraft_.RecipeCraftTick(dataManager_, 9999);
        playerCraft_.RecipeCraftTick(dataManager_, 9999);
        playerCraft_.RecipeCraftTick(dataManager_, 9999);

        EXPECT_EQ(playerInv_.inventory[0].item, &filler_item);
        EXPECT_EQ(playerInv_.inventory[0].count, 1);

        playerInv_.inventory[0] = {nullptr, 0};
        playerCraft_.RecipeCraftTick(dataManager_, 1);

        EXPECT_EQ(playerInv_.inventory[0].item, itemProduct_);
        EXPECT_EQ(playerInv_.inventory[0].count, 1);
    }

    TEST_F(PlayerRecipeTest, Serialize) {
        SetupTestRecipe();

        playerInv_.inventory[0] = {item2_, 1};

        playerInv_.inventory[1] = {itemSub1_, 10};
        playerInv_.inventory[2] = {itemSub2_, 20};

        playerCraft_.RecipeCraftR(dataManager_, *finalRecipe_);


        dataManager_.GenerateRelocationTable();
        data::active_prototype_manager = &dataManager_;

        auto result = TestSerializeDeserialize(playerData_);


        result.crafting.RecipeCraftTick(dataManager_, (2 * 3.1 + 1.5) * kGameHertz);

        auto& result_inv = result.inventory;
        EXPECT_EQ(result_inv.inventory[0].item, item1_);
        EXPECT_EQ(result_inv.inventory[0].count, 1);

        EXPECT_EQ(result_inv.inventory[1].item, itemProduct_);
        EXPECT_EQ(result_inv.inventory[1].count, 1);

        EXPECT_EQ(result.crafting.GetCraftingItemDeductions().size(), 0);
        EXPECT_EQ(result.crafting.GetCraftingItemExtras().size(), 0);
    }
} // namespace jactorio::game
