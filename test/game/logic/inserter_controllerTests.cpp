// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "data/prototype/container_entity.h"
#include "data/prototype/inserter.h"
#include "data/prototype/transport_belt.h"
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
			auto& chunk = worldData_.EmplaceChunk(0, 0);
			worldData_.LogicAddChunk(chunk);
		}

		ChunkTileLayer& BuildInserter(const WorldCoord& coords,
		                              const data::Orientation orientation) {
			return TestSetupInserter(worldData_, logicData_, coords, inserterProto_, orientation);
		}

		///
		/// \brief Creates chest with, emits OnNeighborUpdate
		/// \param orientation Orientation to chest from inserter
		/// \param stack_count Amount of items chest starts with
		data::ContainerEntityData* BuildChest(const WorldCoord& coords,
		                                      const data::Orientation orientation,
		                                      const data::Item::StackCount stack_count,
		                                      const WorldCoord& neighbor_update_coord = {1, 2}) {
			auto& layer = TestSetupContainer(worldData_, coords, containerProto_);

			auto* unique_data = layer.GetUniqueData<data::ContainerEntityData>();

			// Emit neighbor update
			{
				auto& neighbor_layer       = worldData_.GetTile(neighbor_update_coord)->GetLayer(TileLayer::entity);
				const auto* neighbor_proto =
					static_cast<const data::ContainerEntity*>(neighbor_layer.prototypeData.Get());

				if (neighbor_proto)
					neighbor_proto->OnNeighborUpdate(worldData_, logicData_,
					                                 coords, neighbor_update_coord, orientation);
			}

			if (stack_count != 0)
				unique_data->inventory[0] = {&containerItemProto_, stack_count};

			return unique_data;
		}
	};

	TEST_F(InserterControllerTest, GetInserterArmOffset) {
		EXPECT_NEAR(GetInserterArmOffset(20, 1), 0.063235718, kFloatingAbsErr);
		EXPECT_NEAR(GetInserterArmOffset(160, 1), 0.936764281, kFloatingAbsErr);

		EXPECT_NEAR(GetInserterArmOffset(160, 2), 1.300734515, kFloatingAbsErr);

		EXPECT_NEAR(GetInserterArmOffset(180, 1), 0.5, kFloatingAbsErr);
		EXPECT_NEAR(GetInserterArmOffset(180, 2), 0.5, kFloatingAbsErr);
	}

	TEST_F(InserterControllerTest, GetInserterArmLength) {
		EXPECT_NEAR(GetInserterArmLength(20, 1), -1.277013327, kFloatingAbsErr);
		EXPECT_NEAR(GetInserterArmLength(160, 1), 1.277013327, kFloatingAbsErr);

		EXPECT_NEAR(GetInserterArmLength(160, 2), 2.341191099, kFloatingAbsErr);

		EXPECT_NEAR(GetInserterArmLength(180, 1), 1.2, kFloatingAbsErr);
		EXPECT_NEAR(GetInserterArmLength(180, 2), 2.2, kFloatingAbsErr);
	}

	TEST_F(InserterControllerTest, RotateToDropoffAndBack) {
		inserterProto_.rotationSpeed = 2.1;
		const int updates_to_target  = 86;

		auto* dropoff = BuildChest({0, 2}, data::Orientation::left, 10);
		auto* pickup  = BuildChest({2, 2}, data::Orientation::right, 10);

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
		EXPECT_DOUBLE_EQ(inserter_data->rotationDegree.getAsDouble(), 0);


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
			pickup->AppendItem(false, 0, item);
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

	TEST_F(InserterControllerTest, PickupIfCanDropOff) {
		// Inserter will not pick up items that it can never drop off

		// Cannot drop into assembly machine since it has no recipe
		data::AssemblyMachine asm_machine{};
		TestSetupAssemblyMachine(worldData_, {0, 1}, asm_machine);

		auto* pickup = BuildChest({3, 2}, data::Orientation::right, 10);


		inserterProto_.rotationSpeed = 2.1f;
		auto& inserter_layer         = BuildInserter({2, 2}, data::Orientation::left);
		auto* inserter_data          = inserter_layer.GetUniqueData<data::InserterData>();


		// 
		InserterLogicUpdate(worldData_, logicData_);

		EXPECT_EQ(inserter_data->status, data::InserterData::Status::pickup);
		EXPECT_EQ(pickup->inventory[0].count, 10);
	}

	TEST_F(InserterControllerTest, PickupDroppedoffItem) {
		// Can pickup an item dropped off by another inserter on the same game tick

		inserterProto_.rotationSpeed = 180.f;

		auto* left_chest  = BuildChest({0, 2}, data::Orientation::up, 1);
		auto* mid_chest   = BuildChest({2, 2}, data::Orientation::up, 0);
		auto* right_chest = BuildChest({4, 2}, data::Orientation::up, 0);

		// Order which inserters are updated should not matter
		BuildInserter({3, 2}, data::Orientation::right);
		BuildInserter({1, 2}, data::Orientation::right);


		// Pickup
		InserterLogicUpdate(worldData_, logicData_);
		EXPECT_EQ(left_chest->inventory[0].count, 0);

		// Dropoff, picked up by inserter2
		InserterLogicUpdate(worldData_, logicData_);
		EXPECT_EQ(mid_chest->inventory[0].count, 0);

		// Dropoff by inserter2
		InserterLogicUpdate(worldData_, logicData_);
		EXPECT_EQ(right_chest->inventory[0].count, 1);
	}
}
