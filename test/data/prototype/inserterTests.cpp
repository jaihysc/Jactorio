// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "data/prototype/container_entity.h"
#include "data/prototype/inserter.h"

namespace jactorio::data
{
	class InserterTest : public testing::Test
	{
	protected:
		game::WorldData worldData_{};
		game::LogicData logicData_{};

		Inserter inserterProto_{};

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}

		game::ChunkTileLayer& BuildInserter(const WorldCoord& coords,
		                                    const Orientation orientation) {
			return TestSetupInserter(worldData_, logicData_, coords, inserterProto_, orientation);
		}
	};

	TEST_F(InserterTest, OnBuildCreateDataInvalid) {
		BuildInserter({1, 1}, Orientation::right);

		auto& layer         = worldData_.GetTile({1, 1})->GetLayer(game::TileLayer::entity);
		auto* inserter_data = layer.GetUniqueData<InserterData>();
		ASSERT_TRUE(inserter_data);

		EXPECT_DOUBLE_EQ(inserter_data->rotationDegree.getAsDouble(), 180.);

		// Does not have both pickup + dropoff, not added
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);
	}

	TEST_F(InserterTest, FindPickupDropoffOnBuild) {
		// Finding pickup and dropoff tiles

		const ContainerEntity container_entity{};

		TestSetupContainer(worldData_, {1, 1}, container_entity);
		TestSetupContainer(worldData_, {3, 1}, container_entity);

		auto& layer = BuildInserter({2, 1}, Orientation::left);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
	}

	TEST_F(InserterTest, FindPickupDropoff) {
		// Finding pickup and dropoff tiles

		const ContainerEntity container_entity{};

		auto& layer = BuildInserter({2, 1}, Orientation::left);

		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());


		// Dropoff
		TestSetupContainer(worldData_, {1, 1}, container_entity);
		worldData_.UpdateDispatch({1, 1}, UpdateType::place);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);


		// Pickup
		TestSetupContainer(worldData_, {3, 1}, container_entity);
		worldData_.UpdateDispatch({3, 1}, UpdateType::place);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);  // Added since both are now valid
	}

	TEST_F(InserterTest, FindPickupDropoffFar) {
		// Finding pickup and dropoff tiles when tileReach > 1

		inserterProto_.tileReach = 2;

		const ContainerEntity container_entity{};

		auto& layer = BuildInserter({2, 2}, Orientation::up);

		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());


		// Dropoff
		AssemblyMachine asm_machine{};

		TestSetupAssemblyMachine(worldData_, {1, 0}, asm_machine);
		worldData_.UpdateDispatch({2, 0}, UpdateType::place);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);


		// Pickup
		TestSetupContainer(worldData_, {2, 4}, container_entity);
		worldData_.UpdateDispatch({2, 4}, UpdateType::place);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);  // Added since both are now valid
	}

	TEST_F(InserterTest, RemovePickupDropoff) {
		// Finding pickup and dropoff tiles

		const ContainerEntity container_entity{};

		TestSetupContainer(worldData_, {3, 1}, container_entity);

		auto& layer = BuildInserter({2, 1}, Orientation::left);
		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());


		TestSetupContainer(worldData_, {1, 1}, container_entity);
		worldData_.UpdateDispatch({1, 1}, UpdateType::place);
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);


		// Removed chest

		worldData_.GetTile({3, 1})->GetLayer(game::TileLayer::entity).Clear();
		worldData_.UpdateDispatch({3, 1}, UpdateType::place);

		worldData_.GetTile({1, 1})->GetLayer(game::TileLayer::entity).Clear();
		worldData_.UpdateDispatch({1, 1}, UpdateType::place);

		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);
	}

	TEST_F(InserterTest, Serialize) {
		auto inserter_data = std::make_unique<InserterData>(Orientation::left);

		inserter_data->rotationDegree = 145.234;
		inserter_data->status         = InserterData::Status::dropoff;
		inserter_data->heldItem       = {nullptr, 32};

		// ======================================================================

		const std::unique_ptr<UniqueDataBase> base_data = std::move(inserter_data);

		const auto result_base  = TestSerializeDeserialize(base_data);
		const auto* result_data = static_cast<InserterData*>(result_base.get());

		EXPECT_EQ(result_data->orientation, Orientation::left);
		EXPECT_DOUBLE_EQ(result_data->rotationDegree.getAsDouble(), 145.234);
		EXPECT_EQ(result_data->status, InserterData::Status::dropoff);
		EXPECT_EQ(result_data->heldItem.count, 32);
	}
}
