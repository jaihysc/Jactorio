// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "data/data_manager.h"
#include "data/prototype/entity/assembly_machine.h"

namespace jactorio::data
{
	class AssemblyMachineTest : public testing::Test
	{
	protected:
		game::WorldData worldData_;
		game::LogicData logicData_;

		PrototypeManager dataManager_;

		AssemblyMachineData data_{};
		AssemblyMachine proto_;

		Recipe recipe_{};
		Item* item1_       = nullptr;
		Item* item2_       = nullptr;
		Item* itemProduct_ = nullptr;

		void SetupRecipe() {
			recipe_.craftingTime = 1.f;
			recipe_.ingredients  = {{"@1", 1}, {"@2", 1}};
			recipe_.product      = {"@3", 1};

			auto i_1 = std::make_unique<Item>();
			item1_   = i_1.get();

			auto i_2 = std::make_unique<Item>();
			item2_   = i_2.get();

			auto item_3  = std::make_unique<Item>();
			itemProduct_ = item_3.get();

			dataManager_.DataRawAdd("@1", i_1.release());
			dataManager_.DataRawAdd("@2", i_2.release());
			dataManager_.DataRawAdd("@3", item_3.release());
		}

		///
		/// \brief Sets inventory contents of assembly machine to allow crafting
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
		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);
		EXPECT_TRUE(data_.HasRecipe());
		EXPECT_EQ(data_.GetRecipe(), &recipe_);
	}

	TEST_F(AssemblyMachineTest, ChangeRecipeSelectRecipe) {

		EXPECT_EQ(data_.deferralEntry.second, 0);

		SetupRecipe();

		// Recipe crafted in 60 ticks
		logicData_.deferralTimer.DeferralUpdate(worldData_, 900);
		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);

		EXPECT_EQ(data_.deferralEntry.first, 0);

		ASSERT_EQ(data_.ingredientInv.size(), 2);
		ASSERT_EQ(data_.productInv.size(), 1);

		EXPECT_EQ(data_.ingredientInv[0].filter, item1_);
		EXPECT_EQ(data_.ingredientInv[1].filter, item2_);
		EXPECT_EQ(data_.productInv[0].filter, itemProduct_);

		// crafting
		SetupMachineCraftingInv();
		EXPECT_TRUE(proto_.TryBeginCrafting(logicData_, data_));
		EXPECT_EQ(data_.deferralEntry.first, 960);
	}

	TEST_F(AssemblyMachineTest, ChangeRecipeRemoveRecipe) {
		SetupRecipe();

		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);

		SetupMachineCraftingInv();
		ASSERT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

		ASSERT_NE(data_.deferralEntry.second, 0);

		// Remove recipe
		data_.ChangeRecipe(logicData_, dataManager_, nullptr);
		EXPECT_EQ(data_.deferralEntry.second, 0);

		EXPECT_EQ(data_.ingredientInv.size(), 0);
		EXPECT_EQ(data_.productInv.size(), 0);
	}

	TEST_F(AssemblyMachineTest, CanBeginCrafting) {
		// No recipe
		EXPECT_FALSE(data_.CanBeginCrafting());

		SetupRecipe();

		// Ingredients not yet present
		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);
		EXPECT_FALSE(data_.CanBeginCrafting());

		// Ingredients does not match 
		data_.ingredientInv[0] = {item1_, 1};
		data_.ingredientInv[1] = {item1_, 10};

		EXPECT_FALSE(data_.CanBeginCrafting());

		// Ingredients exist and matches
		data_.ingredientInv[1] = {item2_, 1};
		EXPECT_TRUE(data_.CanBeginCrafting());
	}

	TEST_F(AssemblyMachineTest, CraftDeductIngredients) {
		SetupRecipe();
		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);

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
		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);

		data_.ingredientInv[0] = {item1_, 5};
		data_.ingredientInv[1] = {item1_, 10};

		data_.CraftAddProduct();

		ASSERT_EQ(data_.productInv[0].item, itemProduct_);
		ASSERT_EQ(data_.productInv[0].count, 1);
		ASSERT_EQ(data_.productInv[0].filter, itemProduct_);
	}

	// ======================================================================

	TEST_F(AssemblyMachineTest, AssemblySpeed) {
		// AssemblySpeed > 1 reduces crafting time

		SetupRecipe();

		proto_.assemblySpeed = 2;

		recipe_.craftingTime = 1.f;

		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);

		SetupMachineCraftingInv();
		ASSERT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

		EXPECT_EQ(data_.deferralEntry.first, 30);
	}

	TEST_F(AssemblyMachineTest, Build) {
		// Creates unique data on build
		auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::ChunkTile::ChunkLayer::entity);

		proto_.OnBuild(worldData_,
		               logicData_,
		               {0, 0},
		               layer, Orientation::up);

		EXPECT_NE(layer.GetUniqueData(), nullptr);
	}

	TEST_F(AssemblyMachineTest, OnRemoveRemoveDeferralEntry) {
		auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::ChunkTile::ChunkLayer::entity);
		proto_.OnBuild(worldData_, logicData_, {0, 0}, layer, Orientation::up);

		const auto* assembly_proto = layer.GetPrototypeData<AssemblyMachine>();
		auto* assembly_data        = layer.GetUniqueData<AssemblyMachineData>();

		SetupRecipe();
		assembly_data->ChangeRecipe(logicData_, dataManager_, &recipe_);

		assembly_proto->OnRemove(worldData_, logicData_, {0, 0}, layer);
		EXPECT_EQ(assembly_data->deferralEntry.second, 0);
	}

	TEST_F(AssemblyMachineTest, TryBeginCrafting) {
		// No items
		EXPECT_FALSE(proto_.TryBeginCrafting(logicData_, data_));

		// Ok, has items
		SetupRecipe();
		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);
		SetupMachineCraftingInv(10);

		EXPECT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

		// Already has deferral
		EXPECT_NE(data_.deferralEntry.second, 0);
		EXPECT_FALSE(proto_.TryBeginCrafting(logicData_, data_));
	}

	TEST_F(AssemblyMachineTest, CraftingLoop) {
		SetupRecipe();

		data_.ChangeRecipe(logicData_, dataManager_, &recipe_);
		SetupMachineCraftingInv(3);

		// Craft 1
		EXPECT_TRUE(proto_.TryBeginCrafting(logicData_, data_));

		// -- Items removed
		EXPECT_EQ(data_.ingredientInv[0].count, 2);
		EXPECT_EQ(data_.ingredientInv[1].count, 2);

		// -- Output product
		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);
		EXPECT_EQ(data_.productInv[0].count, 1);


		// Craft 2
		EXPECT_EQ(data_.ingredientInv[0].count, 1);
		EXPECT_EQ(data_.ingredientInv[1].count, 1);
		logicData_.deferralTimer.DeferralUpdate(worldData_, 120);
		EXPECT_EQ(data_.productInv[0].count, 2);


		// Craft 3
		EXPECT_EQ(data_.ingredientInv[0].count, 0);
		EXPECT_EQ(data_.ingredientInv[1].count, 0);
		logicData_.deferralTimer.DeferralUpdate(worldData_, 180);
		EXPECT_EQ(data_.productInv[0].count, 3);


		// No more crafting
		EXPECT_EQ(data_.deferralEntry.second, 0);
	}
}
