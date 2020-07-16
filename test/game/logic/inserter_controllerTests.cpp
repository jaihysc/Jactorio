// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
#include "data/prototype/entity/transport_belt.h"
#include "game/logic/inserter_controller.h"

namespace jactorio::game
{
	class InserterControllerTest : public testing::Test
	{
	protected:
		WorldData worldData_{};
		LogicData logicData_{};

		data::Inserter inserterProto_{};

		data::ContainerEntity containerProto_{};
		data::Item containerItemProto_{};

		void SetUp() override {
			auto* chunk = worldData_.EmplaceChunk(0, 0);
			worldData_.LogicAddChunk(chunk);
		}

		ChunkTileLayer& BuildInserter(const WorldData::WorldPair& coords,
		                              const data::Orientation orientation) {
			return TestSetupInserter(worldData_, logicData_, coords, inserterProto_, orientation);
		}

		///
		/// \brief Creates chest with 10 items, emits OnNeighborUpdate for 1, 2
		/// \param orientation Orientation to chest from inserter
		data::ContainerEntityData* BuildChest(const WorldData::WorldPair& coords,
		                                      const data::Orientation orientation) {
			auto& layer = TestSetupContainer(worldData_, coords, containerProto_);

			auto* unique_data = layer.GetUniqueData<data::ContainerEntityData>();

			{
				auto& neighbor_layer       = worldData_.GetTile({1, 2})->GetLayer(ChunkTile::ChunkLayer::entity);
				const auto* neighbor_proto =
					static_cast<const data::ContainerEntity*>(neighbor_layer.prototypeData);

				if (neighbor_proto)
					neighbor_proto->OnNeighborUpdate(worldData_, logicData_,
					                                 coords, {1, 2}, orientation);
			}

			unique_data->inventory[0] = {&containerItemProto_, 10};

			return unique_data;
		}
	};

	TEST_F(InserterControllerTest, GetInserterArmOffset) {
		EXPECT_FLOAT_EQ(GetInserterArmOffset(20, 1), 0.063235718);
		EXPECT_FLOAT_EQ(GetInserterArmOffset(160, 1), 0.936764281);

		EXPECT_FLOAT_EQ(GetInserterArmOffset(160, 2), 1.300734515);

		EXPECT_FLOAT_EQ(GetInserterArmOffset(180, 1), 0.5);
		EXPECT_FLOAT_EQ(GetInserterArmOffset(180, 2), 0.5);
	}

	TEST_F(InserterControllerTest, GetInserterArmLength) {
		EXPECT_FLOAT_EQ(GetInserterArmLength(20, 1), -1.277013327);
		EXPECT_FLOAT_EQ(GetInserterArmLength(160, 1), 1.277013327);

		EXPECT_FLOAT_EQ(GetInserterArmLength(160, 2), 2.341191099);

		EXPECT_FLOAT_EQ(GetInserterArmLength(180, 1), 1.2);
		EXPECT_FLOAT_EQ(GetInserterArmLength(180, 2), 2.2);
	}

	TEST_F(InserterControllerTest, RotateToDropoffAndBack) {
		inserterProto_.rotationSpeed = 2.1;
		const int updates_to_target  = 86;

		auto* dropoff = BuildChest({0, 2}, data::Orientation::left);  // Dropoff
		auto* pickup  = BuildChest({2, 2}, data::Orientation::right);  // Pickup

		auto& inserter_layer = BuildInserter({1, 2}, data::Orientation::left);
		auto* inserter_data  = inserter_layer.GetUniqueData<data::InserterData>();

		// Pickup item
		InserterLogicUpdate(worldData_, logicData_);
		EXPECT_EQ(pickup->inventory[0].count, 9);
		EXPECT_EQ(inserter_data->status, data::InserterData::Status::dropoff);

		// Reach 0 degrees after 86 updates
		for (int i = 0; i < updates_to_target; ++i) {
			InserterLogicUpdate(worldData_, logicData_);
		}
		EXPECT_EQ(dropoff->inventory[0].count, 11);
		EXPECT_EQ(inserter_data->status, data::InserterData::Status::pickup);
		EXPECT_FLOAT_EQ(inserter_data->rotationDegree.getAsDouble(), 0);


		// Return to pickup location after 86 updates, pick up item, set status to dropoff
		for (int i = 0; i < updates_to_target; ++i) {
			InserterLogicUpdate(worldData_, logicData_);
		}
		EXPECT_EQ(pickup->inventory[0].count, 8);
		EXPECT_EQ(inserter_data->status, data::InserterData::Status::dropoff);
	}

	TEST_F(InserterControllerTest, PickupTransportSegment) {
		inserterProto_.rotationSpeed = 2.1;
		inserterProto_.tileReach     = 1;

		// Setup transport segment
		data::Item item{};
		data::TransportBelt segment_proto{};

		auto dropoff = std::make_shared<TransportSegment>(data::Orientation::left,
		                                                  TransportSegment::TerminationType::straight,
		                                                  2);
		TestRegisterTransportSegment(worldData_, {1, 0}, dropoff, segment_proto);


		//
		auto pickup = std::make_shared<TransportSegment>(data::Orientation::left,
		                                                 TransportSegment::TerminationType::straight,
		                                                 2);
		TestRegisterTransportSegment(worldData_, {1, 2}, pickup, segment_proto);

		for (int i = 0; i < 1000; ++i) {
			pickup->AppendItem(false, 0, &item);
		}


		auto& inserter_layer = BuildInserter({1, 1}, data::Orientation::up);
		auto* inserter_data  = inserter_layer.GetUniqueData<data::InserterData>();

		// Logic chunk will be unregistered if setup was invalid
		ASSERT_EQ(worldData_.LogicGetChunks().size(), 1);


		// Will not pickup unless over 90 degrees
		inserter_data->rotationDegree = 87.9;
		inserter_data->status         = data::InserterData::Status::pickup;

		InserterLogicUpdate(worldData_, logicData_);
		ASSERT_EQ(inserter_data->status, data::InserterData::Status::pickup);


		// Pickup when within arm length
		for (int i = 0; i < 42; ++i) {
			InserterLogicUpdate(worldData_, logicData_);

			if (inserter_data->status != data::InserterData::Status::pickup) {
				printf("Failed on iteration %d\n", i);
				ASSERT_EQ(inserter_data->status, data::InserterData::Status::pickup);
				FAIL();
			}
		}


		InserterLogicUpdate(worldData_, logicData_);
		EXPECT_EQ(inserter_data->status, data::InserterData::Status::dropoff);
	}
}
