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
		Item* item1_ = nullptr;
		Item* item2_ = nullptr;
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

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}
	};

	TEST_F(AssemblyMachineTest, RecipeGet) {
		// No recipe

		EXPECT_FALSE(data_.HasRecipe());
		EXPECT_EQ(data_.GetRecipe(), nullptr);

		// Has recipe
		Recipe recipe{};

		data_.ChangeRecipe(logicData_, dataManager_, &recipe);
		EXPECT_TRUE(data_.HasRecipe());
		EXPECT_EQ(data_.GetRecipe(), &recipe);
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
		data_.ingredientInv[0] = {item1_, 1};
		data_.ingredientInv[1] = {item2_, 1};

		EXPECT_TRUE(proto_.TryBeginCrafting(logicData_, data_));
		EXPECT_EQ(data_.deferralEntry.first, 960);
	}

	TEST_F(AssemblyMachineTest, ChangeRecipeRemoveRecipe) {

		Recipe recipe{};
		SetupRecipe();
		data_.ChangeRecipe(logicData_, dataManager_, &recipe);
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

	// ======================================================================

	TEST_F(AssemblyMachineTest, AssemblySpeed) {
		// AssemblySpeed > 1 reduces crafting time

		proto_.assemblySpeed = 2;

		Recipe recipe{};
		recipe.craftingTime = 1.f;

		SetupRecipe();
		data_.ChangeRecipe(logicData_, dataManager_, &recipe);

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

		Recipe recipe{};
		assembly_data->ChangeRecipe(logicData_, dataManager_, &recipe);

		assembly_proto->OnRemove(worldData_, logicData_, {0, 0}, layer);
		EXPECT_EQ(assembly_data->deferralEntry.second, 0);
	}
}
