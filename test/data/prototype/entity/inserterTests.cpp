#include <gtest/gtest.h>

#include "data/prototype/entity/inserter.h"

namespace data::prototype
{
	class InserterTest : public testing::Test
	{
	protected:
		jactorio::game::WorldData worldData_{};

		jactorio::data::Inserter inserterProto_{};
		
		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}

		void BuildInserter(const jactorio::game::WorldData::WorldPair& coords,
		                   const jactorio::data::Orientation orientation) {
			auto& layer = worldData_.GetTile(coords)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			layer.prototypeData = &inserterProto_;
			inserterProto_.OnBuild(worldData_, coords, layer, orientation);
		}
	};

	TEST_F(InserterTest, OnBuildCreateData) {
		BuildInserter({0, 0}, jactorio::data::Orientation::right);
		
		auto& layer = worldData_.GetTile({0, 0})->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

		ASSERT_TRUE(layer.uniqueData);

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
	}
}