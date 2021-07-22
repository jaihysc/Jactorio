// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

namespace jactorio::proto
{
    class AssemblyMachineTest : public testing::Test
    {
    protected:
        game::World world_;
        game::Logic logic_;

        data::PrototypeManager proto_;

        AssemblyMachineData data_;
        AssemblyMachine asmMachine_;

        Recipe* recipe_    = nullptr;
        Item* item1_       = nullptr;
        Item* item2_       = nullptr;
        Item* itemProduct_ = nullptr;

        void SetupRecipe() {
            const auto recipe_data = TestSetupRecipe(proto_);

            recipe_      = recipe_data.recipe;
            item1_       = recipe_data.item1;
            item2_       = recipe_data.item2;
            itemProduct_ = recipe_data.itemProduct;
        }

        /// Sets inventory contents of assembly machine to allow crafting
        void SetupMachineCraftingInv(const Item::StackCount amount = 1) {
            data_.ingredientInv[0] = {item1_, amount};
            data_.ingredientInv[1] = {item2_, amount};
        }

        void SetUp() override {
            world_.EmplaceChunk({0, 0});
        }
    };

    TEST_F(AssemblyMachineTest, RecipeGet) {
        SetupRecipe();

        // No recipe
        EXPECT_FALSE(data_.HasRecipe());
        EXPECT_EQ(data_.GetRecipe(), nullptr);

        // Has recipe
        data_.ChangeRecipe(logic_, proto_, recipe_);
        EXPECT_TRUE(data_.HasRecipe());
        EXPECT_EQ(data_.GetRecipe(), recipe_);
    }

    TEST_F(AssemblyMachineTest, ChangeRecipeSelectRecipe) {

        EXPECT_EQ(data_.deferralEntry.callbackIndex, 0);

        SetupRecipe();

        // Recipe crafted in 60 ticks
        logic_.DeferralUpdate(world_, 900);
        data_.ChangeRecipe(logic_, proto_, recipe_);

        EXPECT_EQ(data_.deferralEntry.dueTick, 0);

        ASSERT_EQ(data_.ingredientInv.Size(), 2);
        ASSERT_EQ(data_.productInv.Size(), 1);

        EXPECT_EQ(data_.ingredientInv[0].filter, item1_);
        EXPECT_EQ(data_.ingredientInv[1].filter, item2_);
        EXPECT_EQ(data_.productInv[0].filter, itemProduct_);

        // crafting
        SetupMachineCraftingInv();
        EXPECT_TRUE(asmMachine_.TryBeginCrafting(logic_, data_));
        EXPECT_EQ(data_.deferralEntry.dueTick, 960);
    }

    TEST_F(AssemblyMachineTest, ChangeRecipeRemoveRecipe) {
        SetupRecipe();

        data_.ChangeRecipe(logic_, proto_, recipe_);

        SetupMachineCraftingInv();
        ASSERT_TRUE(asmMachine_.TryBeginCrafting(logic_, data_));

        ASSERT_NE(data_.deferralEntry.callbackIndex, 0);

        // Remove recipe
        data_.ChangeRecipe(logic_, proto_, nullptr);
        EXPECT_EQ(data_.deferralEntry.callbackIndex, 0);

        EXPECT_EQ(data_.ingredientInv.Size(), 0);
        EXPECT_EQ(data_.productInv.Size(), 0);
    }

    TEST_F(AssemblyMachineTest, CanBeginCrafting) {
        // No recipe
        EXPECT_FALSE(data_.CanBeginCrafting());

        SetupRecipe();

        // Ingredients not yet present
        data_.ChangeRecipe(logic_, proto_, recipe_);
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

        data_.ChangeRecipe(logic_, proto_, recipe_);
        SetupMachineCraftingInv();
        data_.productInv[0] = {itemProduct_, 49, itemProduct_};


        // Cannot craft, next craft will exceed stack
        EXPECT_FALSE(data_.CanBeginCrafting());
    }

    TEST_F(AssemblyMachineTest, CraftDeductIngredients) {
        SetupRecipe();
        data_.ChangeRecipe(logic_, proto_, recipe_);

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
        data_.ChangeRecipe(logic_, proto_, recipe_);

        data_.ingredientInv[0] = {item1_, 5};
        data_.ingredientInv[1] = {item1_, 10};

        data_.CraftAddProduct();

        ASSERT_EQ(data_.productInv[0].item, itemProduct_);
        ASSERT_EQ(data_.productInv[0].count, 1);
        ASSERT_EQ(data_.productInv[0].filter, itemProduct_);
    }

    TEST_F(AssemblyMachineTest, Serialize) {
        SetupRecipe();
        data_.ChangeRecipe(logic_, proto_, recipe_);

        data_.ingredientInv[0] = {item1_, 5};
        data_.ingredientInv[1] = {item2_, 10};

        data_.productInv[0] = {itemProduct_, 6};

        data_.deferralEntry.dueTick       = 7;
        data_.deferralEntry.callbackIndex = 6;

        data_.health = 4321;


        data::active_prototype_manager = &proto_;
        proto_.GenerateRelocationTable();
        const auto result = TestSerializeDeserialize(data_);

        ASSERT_EQ(result.ingredientInv.Size(), 2);

        EXPECT_EQ(result.ingredientInv[0].item, item1_);
        EXPECT_EQ(result.ingredientInv[1].item, item2_);

        EXPECT_EQ(result.ingredientInv[0].count, 5);
        EXPECT_EQ(result.ingredientInv[1].count, 10);


        ASSERT_EQ(result.productInv.Size(), 1);

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

        asmMachine_.assemblySpeed = 2;

        recipe_->craftingTime = 1.f;

        data_.ChangeRecipe(logic_, proto_, recipe_);

        SetupMachineCraftingInv();
        ASSERT_TRUE(asmMachine_.TryBeginCrafting(logic_, data_));

        EXPECT_EQ(data_.deferralEntry.dueTick, 30);
    }

    TEST_F(AssemblyMachineTest, Build) {
        // Creates unique data on build
        auto* tile = world_.GetTile({0, 0}, game::TileLayer::entity);

        asmMachine_.OnBuild(world_, logic_, {0, 0}, Orientation::up);

        EXPECT_NE(tile->GetUniqueData(), nullptr);
    }

    TEST_F(AssemblyMachineTest, OnRemoveRemoveDeferralEntry) {
        asmMachine_.OnBuild(world_, logic_, {0, 0}, Orientation::up);

        auto* tile                 = world_.GetTile({0, 0}, game::TileLayer::entity);
        const auto* assembly_proto = tile->GetPrototype<AssemblyMachine>();
        auto* assembly_data        = tile->GetUniqueData<AssemblyMachineData>();

        SetupRecipe();
        assembly_data->ChangeRecipe(logic_, proto_, recipe_);

        assembly_proto->OnRemove(world_, logic_, {0, 0});
        EXPECT_EQ(assembly_data->deferralEntry.callbackIndex, 0);
    }

    TEST_F(AssemblyMachineTest, TryBeginCrafting) {
        // No items
        EXPECT_FALSE(asmMachine_.TryBeginCrafting(logic_, data_));

        // Ok, has items
        SetupRecipe();
        data_.ChangeRecipe(logic_, proto_, recipe_);
        SetupMachineCraftingInv(10);

        EXPECT_TRUE(asmMachine_.TryBeginCrafting(logic_, data_));

        // Already has deferral
        EXPECT_NE(data_.deferralEntry.callbackIndex, 0);
        EXPECT_FALSE(asmMachine_.TryBeginCrafting(logic_, data_));
    }

    TEST_F(AssemblyMachineTest, CraftingLoop) {
        SetupRecipe();

        data_.ChangeRecipe(logic_, proto_, recipe_);
        SetupMachineCraftingInv(3);

        // Craft 1
        EXPECT_TRUE(asmMachine_.TryBeginCrafting(logic_, data_));

        // -- Items removed
        EXPECT_EQ(data_.ingredientInv[0].count, 2);
        EXPECT_EQ(data_.ingredientInv[1].count, 2);

        // -- Output product
        logic_.DeferralUpdate(world_, 60);
        EXPECT_EQ(data_.productInv[0].count, 1);


        // Craft 2
        EXPECT_EQ(data_.ingredientInv[0].count, 1);
        EXPECT_EQ(data_.ingredientInv[1].count, 1);
        logic_.DeferralUpdate(world_, 120);
        EXPECT_EQ(data_.productInv[0].count, 2);


        // Craft 3
        EXPECT_EQ(data_.ingredientInv[0].count, 0);
        EXPECT_EQ(data_.ingredientInv[1].count, 0);
        logic_.DeferralUpdate(world_, 180);
        EXPECT_EQ(data_.productInv[0].count, 3);


        // No more crafting
        EXPECT_EQ(data_.deferralEntry.callbackIndex, 0);
    }
} // namespace jactorio::proto
