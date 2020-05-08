// 
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
		jactorio::game::World_data world_data{};

		void SetUp() override {
			world_data.add_chunk(new jactorio::game::Chunk{0, 0});
		}


		///
		/// \brief Creates a chest in the world
		static void setup_chest(jactorio::game::World_data& world_data,
											 jactorio::data::Container_entity& container,
		                                     const int world_x = 4, const int world_y = 2) {
			jactorio::game::Chunk_tile_layer& container_layer =
				world_data.get_tile(world_x, world_y)
				          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			container_layer.prototype_data = &container;
			container_layer.unique_data = new jactorio::data::Container_entity_data(20);
		}

		///
		/// \brief Creates a chest in the world, calling on_build
		static void setup_drill(jactorio::game::World_data& world_data,
		                                     jactorio::data::Resource_entity& resource,
		                                     jactorio::data::Mining_drill& drill) {

			jactorio::game::Chunk_tile* tile = world_data.get_tile(1, 1);
			tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;

			drill.on_build(world_data, {1, 1},
			               tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity), jactorio::data::Orientation::right);
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

		jactorio::data::Mining_drill drill{};
		drill.tile_width = 4;
		drill.tile_height = 3;
		drill.mining_radius = 2;

		// Has no resource tiles
		EXPECT_FALSE(drill.on_can_build(world_data, {2, 2}));

		// Has resource tiles
		jactorio::data::Resource_entity resource{};
		world_data.get_tile(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;

		EXPECT_TRUE(drill.on_can_build(world_data, {2, 2}));
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

		jactorio::data::Mining_drill drill{};
		drill.tile_width = 4;
		drill.tile_height = 3;
		drill.mining_radius = 2;

		jactorio::data::Resource_entity resource{};
		world_data.get_tile(7, 6)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;

		EXPECT_TRUE(drill.on_can_build(world_data, {2, 2}));
	}

	// Creates a world and a 3 x 3 radius 1 mining drill
#define MINING_DRILL_DRILL\
		jactorio::data::Mining_drill drill{};\
		drill.tile_width = 3;\
		drill.tile_height = 3;\
		drill.mining_radius = 1;

#define MINING_DRILL_RESOURCE\
		jactorio::data::Item resource_item{};\
		jactorio::data::Resource_entity resource{};\
		resource.pickup_time = 1.f;\
		resource.set_item(&resource_item);


	TEST_F(MiningDrillTest, FindOutputItem) {
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		EXPECT_EQ(drill.find_output_item(world_data, {2, 2}), nullptr);  // No resources 


		world_data.get_tile(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;

		EXPECT_EQ(drill.find_output_item(world_data, {2, 2}), nullptr);  // No resources in range


		world_data.get_tile(6, 5)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;

		EXPECT_EQ(drill.find_output_item(world_data, {2, 2}), nullptr);  // No resources in range

		// ======================================================================

		world_data.get_tile(5, 5)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;
		EXPECT_EQ(drill.find_output_item(world_data, {2, 2}), &resource_item);

		// Closer to the top left
		{
			jactorio::data::Item item2{};
			jactorio::data::Resource_entity resource2{};
			resource2.set_item(&item2);

			world_data.get_tile(1, 1)
			          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource2;
			EXPECT_EQ(drill.find_output_item(world_data, {2, 2}), &item2);
		}
	}


	TEST_F(MiningDrillTest, BuildAndExtractResource) {
		// Mining drill is built with an item output chest
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resource_output.right = {3, 1};
		jactorio::data::Container_entity container{};


		setup_chest(world_data, container);

		setup_drill(world_data, resource, drill);


		// ======================================================================
		// Unique data created by on_build()
		jactorio::game::Chunk_tile* tile = world_data.get_tile(1, 1);

		jactorio::game::Chunk_tile_layer& layer = tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		auto* data = static_cast<jactorio::data::Mining_drill_data*>(layer.unique_data);

		ASSERT_TRUE(data->output_tile.has_value());

		// Ensure it inserts into the correct entity
		jactorio::data::Item item{};
		data->output_tile->insert({&item, 1});

		jactorio::game::Chunk_tile_layer& container_layer = world_data.get_tile(4, 2)
		                                                              ->get_layer(
			                                                              jactorio::game::Chunk_tile::chunkLayer::entity);

		EXPECT_EQ(static_cast<jactorio::data::Container_entity_data*>(container_layer.unique_data)->inventory[0].second
		          ,
		          1);

		// ======================================================================
		world_data.deferral_timer.deferral_update(60);  // Takes 60 ticks to mine

		EXPECT_EQ(static_cast<jactorio::data::Container_entity_data*>(container_layer.unique_data)->inventory[1].second
		          ,
		          1);
	}

	TEST_F(MiningDrillTest, BuildNoOutput) {
		// Mining drill is built without anywhere to output items
		// Should do nothing until an output is built
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resource_output.right = {3, 1};
		jactorio::data::Container_entity container{};


		setup_drill(world_data, resource, drill);

		setup_chest(world_data, container);

		drill.on_neighbor_update(world_data,
		                         {4, 2}, {1, 1},
		                         jactorio::data::Orientation::right);

		// ======================================================================
		// Should now insert as it has an entity to output to
		jactorio::game::Chunk_tile* tile = world_data.get_tile(1, 1);

		auto* data =
			static_cast<jactorio::data::Mining_drill_data*>(
				tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data);

		ASSERT_TRUE(data->output_tile.has_value());

		// Ensure it inserts into the correct entity
		jactorio::data::Item item{};
		data->output_tile->insert({&item, 1});

		jactorio::game::Chunk_tile_layer& container_layer = world_data.get_tile(4, 2)
		                                                              ->get_layer(
			                                                              jactorio::game::Chunk_tile::chunkLayer::entity);

		EXPECT_EQ(static_cast<jactorio::data::Container_entity_data*>(container_layer.unique_data)->inventory[0].second
		          ,
		          1);
	}

	TEST_F(MiningDrillTest, RemoveDrill) {
		// When the mining drill is removed, it needs to unregister the defer update
		// callback to the unique_data which now no longer exists
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resource_output.right = {3, 1};
		jactorio::data::Container_entity container{};

		setup_chest(world_data, container);
		setup_drill(world_data, resource, drill);

		// Remove
		jactorio::game::Chunk_tile* tile = world_data.get_tile(1, 1);
		drill.on_remove(world_data, {1, 1}, tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity));

		tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).clear();  // Deletes drill data

		// Should no longer be valid
		world_data.deferral_timer.deferral_update(60);
	}

	TEST_F(MiningDrillTest, RemoveOutputEntity) {
		// When the mining drill's output entity is removed, it needs to unregister the defer update
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resource_output.right = {3, 1};
		jactorio::data::Container_entity container{};

		setup_chest(world_data, container);
		setup_drill(world_data, resource, drill);

		// Remove chest
		jactorio::game::Chunk_tile* tile = world_data.get_tile(4, 2);
		tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).clear();  // Remove container 

		// Should only remove the callback once
		drill.on_neighbor_update(world_data,
		                         {4, 2}, {1, 1},
		                         jactorio::data::Orientation::right);
		drill.on_neighbor_update(world_data,
		                         {4, 2}, {1, 1},
		                         jactorio::data::Orientation::right);
		drill.on_neighbor_update(world_data,
		                         {4, 2}, {1, 1},
		                         jactorio::data::Orientation::right);

		// Should no longer be valid
		world_data.deferral_timer.deferral_update(60);
	}

	TEST_F(MiningDrillTest, UpdateNonOutput) {
		// Mining drill should ignore on_neighbor_update from tiles other than the item output tile
		MINING_DRILL_DRILL
		MINING_DRILL_RESOURCE

		drill.resource_output.up = {1, -1};
		drill.resource_output.right = {3, 1};
		setup_drill(world_data, resource, drill);

		// ======================================================================
		jactorio::data::Container_entity container{};
		setup_chest(world_data, container, 2, 0);
		setup_chest(world_data, container, 4, 1);

		drill.on_neighbor_update(world_data,
		                         {2, 0}, {1, 1},
		                         jactorio::data::Orientation::up);
		drill.on_neighbor_update(world_data,
		                         {4, 1}, {1, 1},
		                         jactorio::data::Orientation::right);

		world_data.deferral_timer.deferral_update(60);

		// If the on_neighbor_update event was ignored, no items will be added
		{
			jactorio::game::Chunk_tile_layer& container_layer =
				world_data.get_tile(2, 0)
				          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(static_cast<jactorio::data::Container_entity_data*>(container_layer.unique_data)->inventory[0].second
			          ,
			          0);
		}
		{
			jactorio::game::Chunk_tile_layer& container_layer =
				world_data.get_tile(4, 1)
				          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(static_cast<jactorio::data::Container_entity_data*>(container_layer.unique_data)->inventory[0].second
			          ,
			          0);
		}
	}
}
