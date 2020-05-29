// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
#include "game/logic/inserter_controller.h"

namespace jactorio::game
{
	class InserterControllerTest : public testing::Test
	{
	protected:
		WorldData worldData_{};

		data::Inserter inserterProto_{};

		data::ContainerEntity containerProto_{};
		data::Item containerItemProto_{};

		void SetUp() override {
			auto* chunk = worldData_.EmplaceChunk(0, 0);
			worldData_.LogicAddChunk(chunk);
		}

		///
		/// \param orientation Orientation to drop off
		void BuildInserter(const WorldData::WorldPair& coords,
		                   const data::Orientation orientation) {
			auto& layer = worldData_.GetTile(coords)->GetLayer(ChunkTile::ChunkLayer::entity);

			layer.prototypeData = &inserterProto_;
			inserterProto_.OnBuild(worldData_, coords, layer, orientation);
		}

		///
		/// \brief Creates chest with 10 items, emits OnNeighborUpdate for 1, 2
		/// \param orientation Orientation to chest from inserter
		data::ContainerEntityData* BuildChest(const WorldData::WorldPair& coords,
		                                      const data::Orientation orientation,
		                                      const uint16_t inv_size = 10) {
			auto& layer = worldData_.GetTile(coords)->GetLayer(ChunkTile::ChunkLayer::entity);

			layer.prototypeData = &containerProto_;

			auto* unique_data = layer.MakeUniqueData<data::ContainerEntityData>(inv_size);

			{
				auto& neighbor_layer       = worldData_.GetTile({1, 2})->GetLayer(ChunkTile::ChunkLayer::entity);
				const auto* neighbor_proto =
					static_cast<const data::ContainerEntity*>(neighbor_layer.prototypeData);
				neighbor_proto->OnNeighborUpdate(worldData_, coords, {1, 2}, orientation);
			}

			unique_data->inventory[0] = {&containerItemProto_, 10};

			return unique_data;
		}
	};

	TEST_F(InserterControllerTest, GetInserterArmOffset) {
		EXPECT_FLOAT_EQ(jactorio::game::GetInserterArmOffset(160, 1), 0.436764281);
	}

	/*
	TEST_F(InserterControllerTest, RotateToDropoff) {
		inserterProto_.rotationSpeed = 2.1;

		BuildInserter({1, 2}, data::Orientation::left);
		auto* dropoff = BuildChest({0, 2}, data::Orientation::left);  // Dropoff
		auto* pickup = BuildChest({2, 2}, data::Orientation::right);  // Pickup

		// Pickup item
		InserterLogicUpdate(worldData_);

		// Reach 0 degrees after 86 updates
		worldData_.deferralTimer.DeferralUpdate(86);
		EXPECT_EQ(dropoff->inventory[0].second, 1);
		EXPECT_EQ(pickup->inventory[0].second, 9);
	}
	*/
}