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

	TEST_F(AssemblyMachineTest, Build) {
		// Creates unique data on build
		auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::ChunkTile::ChunkLayer::entity);

		proto_.OnBuild(worldData_,
					   {0, 0},
					   layer,
					   Orientation::up);

		EXPECT_NE(layer.GetUniqueData(), nullptr);
	}
}
