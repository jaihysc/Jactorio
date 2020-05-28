// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/inserter.h"

namespace jactorio::data
{
	class InserterTest : public testing::Test
	{
	protected:
		game::WorldData worldData_{};

		Inserter inserterProto_{};

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}

		void BuildInserter(const game::WorldData::WorldPair& coords,
		                   const Orientation orientation) {
			auto& layer = worldData_.GetTile(coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);

			layer.prototypeData = &inserterProto_;
			inserterProto_.OnBuild(worldData_, coords, layer, orientation);
		}
	};

	TEST_F(InserterTest, OnBuildCreateData) {
		BuildInserter({0, 0}, Orientation::right);

		auto& layer = worldData_.GetTile({0, 0})->GetLayer(game::ChunkTile::ChunkLayer::entity);

		ASSERT_TRUE(layer.GetUniqueData());

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
	}
}
