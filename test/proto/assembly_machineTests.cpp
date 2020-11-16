// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

namespace jactorio::proto
{
    class AssemblyMachineTest : public testing::Test
    {
    protected:
        game::WorldData worldData_;
        game::LogicData logicData_;

        data::PrototypeManager dataManager_;

        AssemblyMachineData data_{};
        AssemblyMachine proto_;

        Recipe* recipe_    = nullptr;
        Item* item1_       = nullptr;
        Item* item2_       = nullptr;
        Item* itemProduct_ = nullptr;

        void SetupRecipe() {
            const auto recipe_data = TestSetupRecipe(dataManager_);

            recipe_      = recipe_data.recipe;
            item1_       = recipe_data.item1;
            item2_       = recipe_data.item2;
            itemProduct_ = recipe_data.itemProduct;
        }

        ///
        /// Sets inventory contents of assembly machine to allow crafting
        void SetupMachineCraftingInv(const Item::StackCount amount = 1) {
            data_.ingredientInv[0] = {item1_, amount};
            data_.ingredientInv[1] = {item2_, amount};
        }

        void SetUp() override {
            worldData_.EmplaceChunk(0, 0);
        }
    };

    TEST_F(AssemblyMachineTest, RecipeGet) {
        SetupRecipe();

        // No recipe
        EXPECT_FALSE(data_.HasRecipe());
        EXPECT_EQ(data_.GetRecipe(), nullptr);

        // Has recipe
        data_.ChangeRecipe(logicData_, dataManager_, recipe_);
        EXPECT_TRUE(data_.HasRecipe());
        EXPECT_EQ(data_.GetRecipe(), recipe_);
    }

    TEST_F(AssemblyMachineTest, ChangeRecipeSelectRecipe) {

        EXPECT_EQ(data_.deferralEntry.callbackIndex, 0);

        SetupRecipe();

        // Recipe crafted in 60 ticks
        logicData_.DeferralUpdate(worldData_, 900);
        data_.ChangeRecipe(logicData_, dataManager_, recipe_);

        EXPECT_EQ(data_.deferralEntry.dueTick, 0);

        ASSERT_EQ(data_.ingredientInv.size(), 2);
        ASSERT_EQ(data_.productInv.size(), 1);

        EXPECT_EQ(data_.ingredientInv[0].filter, item1_);
        EXPECT_EQ(data_.ingredientInv[1].filter, item2_);
        EXPECT_EQ(data_.productInv[0].filter, itemProduct_);

        // crafting
        SetupMachineCraftingInv();
        EXPECT_TRUE(proto_.TryBeginCrafting(logicData_, data_));
        EXPECT_EQ(data_.deferralEntry.dueTick, 960);
    }

    TEST_F(AssemblyMachineTest, ChangeRecipeRemoveRecipe) {
        SetupRecipe();

        data_.ChangeRecipe(logicData_, dataManager_, recipe_);

        SetupMachineCraftingInv();
        ASSERT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

        ASSERT_NE(data_.deferralEntry.callbackIndex, 0);

        // Remove recipe
        data_.ChangeRecipe(logicData_, dataManager_, nullptr);
        EXPECT_EQ(data_.deferralEntry.callbackIndex, 0);

        EXPECT_EQ(data_.ingredientInv.size(), 0);
        EXPECT_EQ(data_.productInv.size(), 0);
    }

    TEST_F(AssemblyMachineTest, CanBeginCrafting) {
        // No recipe
        EXPECT_FALSE(data_.CanBeginCrafting());

        SetupRecipe();

        // Ingredients not yet present
        data_.ChangeRecipe(logicData_, dataManager_, recipe_);
        EXPECT_FALSE(data_.CanBeginCrafting());

        // Ingredients does not match
        data_.ingredientInv[0] = {item1_, 1};
        data_.ingredientInv[1] = {item1_, 10};

        EXPECT_FALSE(data_.CanBeginCrafting());

        // Ingredients exist and matches
        data_.ingredientInv[1] = {item2_, 1};
        EXPECT_TRUE(data_.CanBeginCrafting());
    }

    TEST_F(AssemblyMachineTest, CanBeginCraftingStackLimit) {
        SetupRecipe();
        itemProduct_->stackSize = 50;
        recipe_->product.second = 2; // 2 per crafting

        data_.ChangeRecipe(logicData_, dataManager_, recipe_);
        SetupMachineCraftingInv();
        data_.productInv[0] = {itemProduct_, 49, itemProduct_};


        // Cannot craft, next craft will exceed stack
        EXPECT_FALSE(data_.CanBeginCrafting());
    }

    TEST_F(AssemblyMachineTest, CraftDeductIngredients) {
        SetupRecipe();
        data_.ChangeRecipe(logicData_, dataManager_, recipe_);

        data_.ingredientInv[0] = {item1_, 5};
        data_.ingredientInv[1] = {item1_, 10};

        data_.CraftRemoveIngredients();

        EXPECT_EQ(data_.ingredientInv[0].count, 4);
        EXPECT_EQ(data_.ingredientInv[1].count, 9);


        // Final items
        data_.ingredientInv[0] = {item1_, 1};
        data_.ingredientInv[1] = {item1_, 1};

        data_.CraftRemoveIngredients();

        EXPECT_EQ(data_.ingredientInv[0].item, nullptr);
        EXPECT_EQ(data_.ingredientInv[0].count, 0);
        EXPECT_EQ(data_.ingredientInv[1].item, nullptr);
        EXPECT_EQ(data_.ingredientInv[1].count, 0);
    }

    TEST_F(AssemblyMachineTest, CraftAddProduct) {
        SetupRecipe();
        data_.ChangeRecipe(logicData_, dataManager_, recipe_);

        data_.ingredientInv[0] = {item1_, 5};
        data_.ingredientInv[1] = {item1_, 10};

        data_.CraftAddProduct();

        ASSERT_EQ(data_.productInv[0].item, itemProduct_);
        ASSERT_EQ(data_.productInv[0].count, 1);
        ASSERT_EQ(data_.productInv[0].filter, itemProduct_);
    }

    TEST_F(AssemblyMachineTest, Serialize) {
        SetupRecipe();
        data_.ChangeRecipe(logicData_, dataManager_, recipe_);

        data_.ingredientInv[0] = {item1_, 5};
        data_.ingredientInv[1] = {item2_, 10};

        data_.productInv[0] = {itemProduct_, 6};

        data_.deferralEntry.dueTick       = 7;
        data_.deferralEntry.callbackIndex = 6;

        data_.health = 4321;


        data::active_prototype_manager = &dataManager_;
        dataManager_.GenerateRelocationTable();
        const auto result = TestSerializeDeserialize(data_);

        ASSERT_EQ(result.ingredientInv.size(), 2);

        EXPECT_EQ(result.ingredientInv[0].item, item1_);
        EXPECT_EQ(result.ingredientInv[1].item, item2_);

        EXPECT_EQ(result.ingredientInv[0].count, 5);
        EXPECT_EQ(result.ingredientInv[1].count, 10);


        ASSERT_EQ(result.productInv.size(), 1);

        EXPECT_EQ(result.productInv[0].item, itemProduct_);
        EXPECT_EQ(result.productInv[0].count, 6);


        EXPECT_EQ(result.GetRecipe(), recipe_);


        EXPECT_EQ(result.deferralEntry.dueTick, 7);
        EXPECT_EQ(result.deferralEntry.callbackIndex, 6);


        EXPECT_EQ(result.health, 4321);
    }

    // ======================================================================

    TEST_F(AssemblyMachineTest, AssemblySpeed) {
        // AssemblySpeed > 1 reduces crafting time

        SetupRecipe();

        proto_.assemblySpeed = 2;

        recipe_->craftingTime = 1.f;

        data_.ChangeRecipe(logicData_, dataManager_, recipe_);

        SetupMachineCraftingInv();
        ASSERT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

        EXPECT_EQ(data_.deferralEntry.dueTick, 30);
    }

    TEST_F(AssemblyMachineTest, Build) {
        // Creates unique data on build
        auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::TileLayer::entity);

        proto_.OnBuild(worldData_, logicData_, {0, 0}, layer, Orientation::up);

        EXPECT_NE(layer.GetUniqueData(), nullptr);
    }

    TEST_F(AssemblyMachineTest, OnRemoveRemoveDeferralEntry) {
        auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::TileLayer::entity);
        proto_.OnBuild(worldData_, logicData_, {0, 0}, layer, Orientation::up);

        const auto* assembly_proto = layer.GetPrototypeData<AssemblyMachine>();
        auto* assembly_data        = layer.GetUniqueData<AssemblyMachineData>();

        SetupRecipe();
        assembly_data->ChangeRecipe(logicData_, dataManager_, recipe_);

        assembly_proto->OnRemove(worldData_, logicData_, {0, 0}, layer);
        EXPECT_EQ(assembly_data->deferralEntry.callbackIndex, 0);
    }

    TEST_F(AssemblyMachineTest, TryBeginCrafting) {
        // No items
        EXPECT_FALSE(proto_.TryBeginCrafting(logicData_, data_));

        // Ok, has items
        SetupRecipe();
        data_.ChangeRecipe(logicData_, dataManager_, recipe_);
        SetupMachineCraftingInv(10);

        EXPECT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

        // Already has deferral
        EXPECT_NE(data_.deferralEntry.callbackIndex, 0);
        EXPECT_FALSE(proto_.TryBeginCrafting(logicData_, data_));
    }

    TEST_F(AssemblyMachineTest, CraftingLoop) {
        SetupRecipe();

        data_.ChangeRecipe(logicData_, dataManager_, recipe_);
        SetupMachineCraftingInv(3);

        // Craft 1
        EXPECT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

        // -- Items removed
        EXPECT_EQ(data_.ingredientInv[0].count, 2);
        EXPECT_EQ(data_.ingredientInv[1].count, 2);

        // -- Output product
        logicData_.DeferralUpdate(worldData_, 60);
        EXPECT_EQ(data_.productInv[0].count, 1);


        // Craft 2
        EXPECT_EQ(data_.ingredientInv[0].count, 1);
        EXPECT_EQ(data_.ingredientInv[1].count, 1);
        logicData_.DeferralUpdate(worldData_, 120);
        EXPECT_EQ(data_.productInv[0].count, 2);


        // Craft 3
        EXPECT_EQ(data_.ingredientInv[0].count, 0);
        EXPECT_EQ(data_.ingredientInv[1].count, 0);
        logicData_.DeferralUpdate(worldData_, 180);
        EXPECT_EQ(data_.productInv[0].count, 3);


        // No more crafting
        EXPECT_EQ(data_.deferralEntry.callbackIndex, 0);
    }
} // namespace jactorio::proto