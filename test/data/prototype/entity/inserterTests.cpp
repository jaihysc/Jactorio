// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"

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

		game::ChunkTileLayer& BuildInserter(const game::WorldData::WorldPair& coords,
		                                    const Orientation orientation) {
			return TestSetupInserter(worldData_, logicData_, coords, inserterProto_, orientation);
		}
	};

	TEST_F(InserterTest, OnBuildCreateDataInvalid) {
		BuildInserter({1, 1}, Orientation::right);

		auto& layer         = worldData_.GetTile({1, 1})->GetLayer(game::ChunkTile::ChunkLayer::entity);
		auto* inserter_data = layer.GetUniqueData<InserterData>();
		ASSERT_TRUE(inserter_data);

		EXPECT_FLOAT_EQ(inserter_data->rotationDegree.getAsDouble(), 180.f);

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
		worldData_.updateDispatcher.Dispatch({1, 1}, UpdateType::place);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);


		// Pickup
		TestSetupContainer(worldData_, {3, 1}, container_entity);
		worldData_.updateDispatcher.Dispatch({3, 1}, UpdateType::place);

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
		worldData_.updateDispatcher.Dispatch({2, 0}, UpdateType::place);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);


		// Pickup
		TestSetupContainer(worldData_, {2, 4}, container_entity);
		worldData_.updateDispatcher.Dispatch({2, 4}, UpdateType::place);

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
		worldData_.updateDispatcher.Dispatch({1, 1}, UpdateType::place);
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);


		// Removed chest

		worldData_.GetTile({3, 1})->GetLayer(game::ChunkTile::ChunkLayer::entity).Clear();
		worldData_.updateDispatcher.Dispatch({3, 1}, UpdateType::place);

		worldData_.GetTile({1, 1})->GetLayer(game::ChunkTile::ChunkLayer::entity).Clear();
		worldData_.updateDispatcher.Dispatch({1, 1}, UpdateType::place);

		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);
	}
}
