// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#include <gtest/gtest.h>

#include "jactorioTests.h"
#include "data/prototype/entity/assembly_machine.h"

namespace jactorio::data
{
	class AssemblyMachineTest : public testing::Test
	{
	protected:
		game::WorldData worldData_;
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

		data.ChangeRecipe(worldData_, proto_, &recipe);
		EXPECT_TRUE(data.HasRecipe());
		EXPECT_EQ(data.GetRecipe(), &recipe);
	}

	TEST_F(AssemblyMachineTest, ChangeRecipeSelectRecipe) {
		AssemblyMachineData data{};

		EXPECT_EQ(data.deferralEntry.second, 0);

		Recipe recipe{};
		recipe.craftingTime = 1.f;

		// Recipe crafted in 60 ticks
		worldData_.deferralTimer.DeferralUpdate(900);
		data.ChangeRecipe(worldData_, proto_, &recipe);

		EXPECT_EQ(data.deferralEntry.first, 960);
	}

	TEST_F(AssemblyMachineTest, ChangeRecipeRemoveRecipe) {
		AssemblyMachineData data{};

		Recipe recipe{};
		data.ChangeRecipe(worldData_, proto_, &recipe);

		EXPECT_NE(data.deferralEntry.second, 0);

		// Remove recipe
		data.ChangeRecipe(worldData_, proto_, nullptr);
		EXPECT_EQ(data.deferralEntry.second, 0);
	}

	TEST_F(AssemblyMachineTest, AssemblySpeed) {
		// AssemblySpeed > 1 reduces crafting time

		proto_.assemblySpeed = 2;
		AssemblyMachineData data{};

		Recipe recipe{};
		recipe.craftingTime = 1.f;

		data.ChangeRecipe(worldData_, proto_, &recipe);
		EXPECT_EQ(data.deferralEntry.first, 30);
	}

	TEST_F(AssemblyMachineTest, Build) {
		// Creates unique data on build
		auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::ChunkTile::ChunkLayer::entity);

		proto_.OnBuild(worldData_,
		               {0, 0},
		               layer,
		               Orientation::up);

		EXPECT_NE(layer.GetUniqueData(), nullptr);
	}

	TEST_F(AssemblyMachineTest, OnRemoveRemoveDeferralEntry) {
		auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::ChunkTile::ChunkLayer::entity);
		proto_.OnBuild(worldData_, {0, 0}, layer, Orientation::up);

		const auto* assembly_proto = layer.GetPrototypeData<AssemblyMachine>();
		auto* assembly_data        = layer.GetUniqueData<AssemblyMachineData>();

		Recipe recipe{};
		assembly_data->ChangeRecipe(worldData_, proto_, &recipe);

		assembly_proto->OnRemove(worldData_, {0, 0}, layer);
		EXPECT_EQ(assembly_data->deferralEntry.second, 0);
	}
}
