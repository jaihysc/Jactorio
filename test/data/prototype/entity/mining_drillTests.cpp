// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/06/2020

#include <gtest/gtest.h>


#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/resource_entity.h"

namespace data::prototype
{
	class MiningDrillTest : public testing::Test
	{
	protected:
		jactorio::game::WorldData worldData_{};

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}


		///
		/// \brief Creates a chest in the world
		static void SetupChest(jactorio::game::WorldData& world_data,
		                       jactorio::data::ContainerEntity& container,
		                       const int world_x = 4, const int world_y = 2) {
			jactorio::game::ChunkTileLayer& container_layer =
				world_data.GetTile(world_x, world_y)
				          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			container_layer.prototypeData = &container;
			container_layer.uniqueData    = new jactorio::data::ContainerEntityData(20);
		}

		///
		/// \brief Creates a chest in the world, calling on_build
		static void SetupDrill(jactorio::game::WorldData& world_data,
		                       jactorio::data::ResourceEntity& resource,
		                       jactorio::data::MiningDrill& drill) {

			jactorio::game::ChunkTile* tile                                               = world_data.GetTile(1, 1);
			tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

			drill.OnBuild(world_data, {1, 1},
			              tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity), jactorio::data::Orientation::right);
		}
	};

	TEST_F(MiningDrillTest, OnCanBuild) {
		/*
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [X] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 */

		jactorio::data::MiningDrill drill{};
		drill.tileWidth    = 4;
		drill.tileHeight   = 3;
		drill.miningRadius = 2;

		// Has no resource tiles
		EXPECT_FALSE(drill.OnCanBuild(worldData_, {2, 2}));

		// Has resource tiles
		jactorio::data::ResourceEntity resource{};
		worldData_.GetTile(0, 0)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

		EXPECT_TRUE(drill.OnCanBuild(worldData_, {2, 2}));
	}

	TEST_F(MiningDrillTest, OnCanBuild2) {
		/*
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [X] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 */

		jactorio::data::MiningDrill drill{};
		drill.tileWidth    = 4;
		drill.tileHeight   = 3;
		drill.miningRadius = 2;

		jactorio::data::ResourceEntity resource{};
		worldData_.GetTile(7, 6)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

		EXPECT_TRUE(drill.OnCanBuild(worldData_, {2, 2}));
	}

	// Creates a world and a 3 x 3 radius 1 mining drill
#define MINING_DRILL_DRILL\
		jactorio::data::MiningDrill drill{};\
		drill.tileWidth = 3;\
		drill.tileHeight = 3;\
		drill.miningRadius = 1;

#define MINING_DRILL_RESOURCE\
		jactorio::data::Item resource_item{};\
		jactorio::data::ResourceEntity resource{};\
		resource.pickupTime = 1.f;\
		resource.SetItem(&resource_item);


	TEST_F(MiningDrillTest, FindOutputItem) {
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		EXPECT_EQ(drill.FindOutputItem(worldData_, {2, 2}), nullptr);  // No resources 


		worldData_.GetTile(0, 0)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

		EXPECT_EQ(drill.FindOutputItem(worldData_, {2, 2}), nullptr);  // No resources in range


		worldData_.GetTile(6, 5)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

		EXPECT_EQ(drill.FindOutputItem(worldData_, {2, 2}), nullptr);  // No resources in range

		// ======================================================================

		worldData_.GetTile(5, 5)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;
		EXPECT_EQ(drill.FindOutputItem(worldData_, {2, 2}), &resource_item);

		// Closer to the top left
		{
			jactorio::data::Item item2{};
			jactorio::data::ResourceEntity resource2{};
			resource2.SetItem(&item2);

			worldData_.GetTile(1, 1)
			          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &resource2;
			EXPECT_EQ(drill.FindOutputItem(worldData_, {2, 2}), &item2);
		}
	}


	TEST_F(MiningDrillTest, BuildAndExtractResource) {
		// Mining drill is built with an item output chest
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resourceOutput.right = {3, 1};
		jactorio::data::ContainerEntity container{};


		SetupChest(worldData_, container);

		SetupDrill(worldData_, resource, drill);


		// ======================================================================
		// Unique data created by on_build()
		jactorio::game::ChunkTile* tile = worldData_.GetTile(1, 1);

		jactorio::game::ChunkTileLayer& layer = tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
		auto* data                            = static_cast<jactorio::data::MiningDrillData*>(layer.uniqueData);

		// Ensure it inserts into the correct entity
		jactorio::data::Item item{};
		data->outputTile.DropOff({&item, 1});

		jactorio::game::ChunkTileLayer& container_layer = worldData_.GetTile(4, 2)
		                                                            ->GetLayer(
			                                                            jactorio::game::ChunkTile::ChunkLayer::entity);

		EXPECT_EQ(static_cast<jactorio::data::ContainerEntityData*>(container_layer.uniqueData)->inventory[0].second
		          ,
		          1);

		// ======================================================================
		worldData_.deferralTimer.DeferralUpdate(60);  // Takes 60 ticks to mine

		EXPECT_EQ(static_cast<jactorio::data::ContainerEntityData*>(container_layer.uniqueData)->inventory[1].second
		          ,
		          1);
	}

	TEST_F(MiningDrillTest, BuildNoOutput) {
		// Mining drill is built without anywhere to output items
		// Should do nothing until an output is built
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resourceOutput.right = {3, 1};
		jactorio::data::ContainerEntity container{};


		SetupDrill(worldData_, resource, drill);

		SetupChest(worldData_, container);

		drill.OnNeighborUpdate(worldData_,
		                       {4, 2}, {1, 1},
		                       jactorio::data::Orientation::right);

		// ======================================================================
		// Should now insert as it has an entity to output to
		jactorio::game::ChunkTile* tile = worldData_.GetTile(1, 1);

		auto* data =
			static_cast<jactorio::data::MiningDrillData*>(
				tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).uniqueData);

		// Ensure it inserts into the correct entity
		jactorio::data::Item item{};
		data->outputTile.DropOff({&item, 1});

		jactorio::game::ChunkTileLayer& container_layer = worldData_.GetTile(4, 2)
		                                                            ->GetLayer(
			                                                            jactorio::game::ChunkTile::ChunkLayer::entity);

		EXPECT_EQ(static_cast<jactorio::data::ContainerEntityData*>(container_layer.uniqueData)->inventory[0].second
		          ,
		          1);
	}

	TEST_F(MiningDrillTest, RemoveDrill) {
		// When the mining drill is removed, it needs to unregister the defer update
		// callback to the unique_data which now no longer exists
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resourceOutput.right = {3, 1};
		jactorio::data::ContainerEntity container{};

		SetupChest(worldData_, container);
		SetupDrill(worldData_, resource, drill);

		// Remove
		jactorio::game::ChunkTile* tile = worldData_.GetTile(1, 1);
		drill.OnRemove(worldData_, {1, 1}, tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity));

		tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).Clear();  // Deletes drill data

		// Should no longer be valid
		worldData_.deferralTimer.DeferralUpdate(60);
	}

	TEST_F(MiningDrillTest, RemoveOutputEntity) {
		// When the mining drill's output entity is removed, it needs to unregister the defer update
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resourceOutput.right = {3, 1};
		jactorio::data::ContainerEntity container{};

		SetupChest(worldData_, container);
		SetupDrill(worldData_, resource, drill);

		// Remove chest
		jactorio::game::ChunkTile* tile = worldData_.GetTile(4, 2);
		tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).Clear();  // Remove container 

		// Should only remove the callback once
		drill.OnNeighborUpdate(worldData_,
		                       {4, 2}, {1, 1},
		                       jactorio::data::Orientation::right);
		drill.OnNeighborUpdate(worldData_,
		                       {4, 2}, {1, 1},
		                       jactorio::data::Orientation::right);
		drill.OnNeighborUpdate(worldData_,
		                       {4, 2}, {1, 1},
		                       jactorio::data::Orientation::right);

		// Should no longer be valid
		worldData_.deferralTimer.DeferralUpdate(60);
	}

	TEST_F(MiningDrillTest, UpdateNonOutput) {
		// Mining drill should ignore on_neighbor_update from tiles other than the item output tile
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resourceOutput.up    = {1, -1};
		drill.resourceOutput.right = {3, 1};
		SetupDrill(worldData_, resource, drill);

		// ======================================================================
		jactorio::data::ContainerEntity container{};
		SetupChest(worldData_, container, 2, 0);
		SetupChest(worldData_, container, 4, 1);

		drill.OnNeighborUpdate(worldData_,
		                       {2, 0}, {1, 1},
		                       jactorio::data::Orientation::up);
		drill.OnNeighborUpdate(worldData_,
		                       {4, 1}, {1, 1},
		                       jactorio::data::Orientation::right);

		worldData_.deferralTimer.DeferralUpdate(60);

		// If the on_neighbor_update event was ignored, no items will be added
		{
			jactorio::game::ChunkTileLayer& container_layer =
				worldData_.GetTile(2, 0)
				          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(static_cast<jactorio::data::ContainerEntityData*>(container_layer.uniqueData)->inventory[0].second
			          ,
			          0);
		}
		{
			jactorio::game::ChunkTileLayer& container_layer =
				worldData_.GetTile(4, 1)
				          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(static_cast<jactorio::data::ContainerEntityData*>(container_layer.uniqueData)->inventory[0].second
			          ,
			          0);
		}
	}
}
