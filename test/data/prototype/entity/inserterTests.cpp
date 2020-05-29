// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

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

		Inserter inserterProto_{};

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}

		game::ChunkTileLayer& BuildInserter(const game::WorldData::WorldPair& coords,
		                                    const Orientation orientation) {
			auto& layer = worldData_.GetTile(coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);

			layer.prototypeData = &inserterProto_;
			inserterProto_.OnBuild(worldData_, coords, layer, orientation);

			return layer;
		}
	};

	TEST_F(InserterTest, OnBuildCreateData) {
		BuildInserter({1, 1}, Orientation::right);

		auto& layer = worldData_.GetTile({1, 1})->GetLayer(game::ChunkTile::ChunkLayer::entity);
		auto* inserter_data = layer.GetUniqueData<InserterData>();
		ASSERT_TRUE(inserter_data);

		EXPECT_FLOAT_EQ(inserter_data->rotationDegree.getAsDouble(), 180.f);

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
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

		TestSetupContainer(worldData_, {1, 1}, container_entity);
		inserterProto_.OnNeighborUpdate(worldData_, {1, 1}, {2, 1}, Orientation::left);
		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());

		TestSetupContainer(worldData_, {3, 1}, container_entity);
		inserterProto_.OnNeighborUpdate(worldData_, {3, 1}, {2, 1}, Orientation::right);
		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
	}

	TEST_F(InserterTest, RemovePickupDropoff) {
		// Finding pickup and dropoff tiles

		const ContainerEntity container_entity{};

		TestSetupContainer(worldData_, {3, 1}, container_entity);

		auto& layer = BuildInserter({2, 1}, Orientation::left);

		EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());

		// Removed chest
		
		worldData_.GetTile({3, 1})->GetLayer(game::ChunkTile::ChunkLayer::entity).Clear();
		inserterProto_.OnNeighborUpdate(worldData_, {3, 1}, {2, 1}, Orientation::right);

		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
		EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
	}
}
