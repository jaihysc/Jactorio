#include <gtest/gtest.h>

#include "data/prototype/entity/inserter.h"
#include "game/logic/inserter_controller.h"

namespace game
{
	class InserterControllerTest : public testing::Test
	{
	protected:
		jactorio::game::WorldData worldData_{};

		jactorio::data::Inserter inserterProto_{};
		
		void SetUp() override {
			auto* chunk = worldData_.EmplaceChunk(0, 0);
			worldData_.LogicAddChunk(chunk);
		}

		///
		/// \param orientation Orientation to drop off
		void BuildInserter(const jactorio::game::WorldData::WorldPair& coords,
		                   const jactorio::data::Orientation orientation) {
			auto& layer = worldData_.GetTile(coords)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			layer.prototypeData = &inserterProto_;
			inserterProto_.OnBuild(worldData_, coords, layer, orientation);
		}
	};

	TEST_F(InserterControllerTest, GetInserterArmOffset) {
		EXPECT_FLOAT_EQ(jactorio::game::GetInserterArmOffset(160, 1), 0.436764281);
	}

	TEST_F(InserterControllerTest, RotateToDropoff) {
		inserterProto_.rotationSpeed = 2.1;

		BuildInserter({1, 2}, jactorio::data::Orientation::left);


		// Pickup item
		InserterLogicUpdate(worldData_);

		// Reach 0 degrees after 86 updates
		// TODO
	}
}