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

		AssemblyMachine proto_;

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}
	};

	TEST_F(AssemblyMachineTest, RecipeGet) {
		// No recipe
		AssemblyMachineData data{};

		EXPECT_FALSE(data.HasRecipe());
		EXPECT_EQ(data.GetRecipe(), nullptr);

		// Has recipe
		Recipe recipe{};

		data.ChangeRecipe(logicData_, dataManager_, proto_, &recipe);
		EXPECT_TRUE(data.HasRecipe());
		EXPECT_EQ(data.GetRecipe(), &recipe);
	}

	TEST_F(AssemblyMachineTest, ChangeRecipeSelectRecipe) {
		AssemblyMachineData data{};

		EXPECT_EQ(data.deferralEntry.second, 0);

		Recipe recipe{};
		recipe.craftingTime = 1.f;
		recipe.ingredients  = {{"@1", 1}, {"@2", 1}};
		recipe.product      = {"@3", 1};

		auto item_1            = std::make_unique<Item>();
		const auto* item_1_ptr = item_1.get();

		auto item_2            = std::make_unique<Item>();
		const auto* item_2_ptr = item_2.get();

		auto item_3            = std::make_unique<Item>();
		const auto* item_3_ptr = item_3.get();

		dataManager_.DataRawAdd("@1", item_1.release());
		dataManager_.DataRawAdd("@2", item_2.release());
		dataManager_.DataRawAdd("@3", item_3.release());

		// Recipe crafted in 60 ticks
		logicData_.deferralTimer.DeferralUpdate(worldData_, 900);
		data.ChangeRecipe(logicData_, dataManager_, proto_, &recipe);

		EXPECT_EQ(data.deferralEntry.first, 960);

		ASSERT_EQ(data.ingredientInv.size(), 2);
		ASSERT_EQ(data.productInv.size(), 1);

		EXPECT_EQ(data.ingredientInv[0].filter, item_1_ptr);
		EXPECT_EQ(data.ingredientInv[1].filter, item_2_ptr);
		EXPECT_EQ(data.productInv[0].filter, item_3_ptr);
	}

	TEST_F(AssemblyMachineTest, ChangeRecipeRemoveRecipe) {
		AssemblyMachineData data{};

		Recipe recipe{};
		data.ChangeRecipe(logicData_, dataManager_, proto_, &recipe);

		EXPECT_NE(data.deferralEntry.second, 0);

		// Remove recipe
		data.ChangeRecipe(logicData_, dataManager_, proto_, nullptr);
		EXPECT_EQ(data.deferralEntry.second, 0);

		EXPECT_EQ(data.ingredientInv.size(), 0);
		EXPECT_EQ(data.productInv.size(), 0);
	}

	TEST_F(AssemblyMachineTest, AssemblySpeed) {
		// AssemblySpeed > 1 reduces crafting time

		proto_.assemblySpeed = 2;
		AssemblyMachineData data{};

		Recipe recipe{};
		recipe.craftingTime = 1.f;

		data.ChangeRecipe(logicData_, dataManager_, proto_, &recipe);
		EXPECT_EQ(data.deferralEntry.first, 30);
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
		assembly_data->ChangeRecipe(logicData_, dataManager_, proto_, &recipe);

		assembly_proto->OnRemove(worldData_, logicData_, {0, 0}, layer);
		EXPECT_EQ(assembly_data->deferralEntry.second, 0);
	}
}
