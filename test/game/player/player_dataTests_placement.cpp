// 
// player_dataTests_placement.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/04/2020
// Last modified: 04/04/2020
// 

#include <gtest/gtest.h>

#include "core/resource_guard.h"
#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/tile/tile.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace game
{
	TEST(player_data, rotate_placement_orientation) {
		jactorio::game::Player_data player_data{};

		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::up);

		player_data.rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::right);

		player_data.rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::down);

		player_data.rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::left);

		player_data.rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::up);
	}

	TEST(player_data, counter_rotate_placement_orientation) {
		jactorio::game::Player_data player_data{};

		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::up);

		player_data.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::left);

		player_data.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::down);

		player_data.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::right);

		player_data.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data.placement_orientation, jactorio::data::placementOrientation::up);
	}

	TEST(player_data, try_place_entity) {
		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};

		// Create entity
		auto item = jactorio::data::Item();
		auto item_no_entity = jactorio::data::Item();  // Does not hold an entity reference

		auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->set_item(&item);


		auto entity2 = std::make_unique<jactorio::data::Container_entity>();


		auto tile_proto = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[1].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[1].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity2.get());

		world_data.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));

		// Edge cases
		player_data.try_place_entity(world_data, 0, 0);  // Placing with no items selected

		jactorio::data::item_stack selected_item = {&item_no_entity, 2};
		player_data.set_selected_item(selected_item);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity.get());

		player_data.try_place_entity(world_data, 0, 0);  // Item holds no reference to an entity
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			entity.get());  // Should not delete item at this location


		// Placement tests

		// Place at 0, 0
		selected_item = {&item, 2};
		player_data.set_selected_item(selected_item);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, nullptr);

		player_data.try_place_entity(world_data, 0, 0);  // Place on empty tile 0, 0

		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			entity.get());
		EXPECT_EQ(player_data.get_selected_item()->second, 1);  // 1 less item 

		// The on_build() method should get called, creating unique data on the tile which holds the inventory
		EXPECT_NE(tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data, nullptr);


		// Do not place at 1, 0 
		player_data.try_place_entity(world_data, 1, 0);  // A tile already exists on 1, 0 - Should not override it
		EXPECT_EQ(
			tiles[1].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			entity2.get());
	}

	TEST(player_data, try_place_entity_activate_layer) {
		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};


		// Create entity
		auto item = jactorio::data::Item();
		auto item_no_entity = jactorio::data::Item();  // Does not hold an entity reference

		auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->placeable = true;
		entity->set_item(&item);


		auto tile_proto = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity.get());


		world_data.add_chunk(new jactorio::game::Chunk(0, 0, tiles));

		// If selected item's entity is placeable, do not set activated_layer
		jactorio::data::item_stack selected_item = {&item, 2};
		player_data.set_selected_item(selected_item);

		player_data.try_place_entity(world_data, 0, 0, true);
		EXPECT_EQ(player_data.get_activated_layer(), nullptr);

		// Clicking on an entity with no placeable items selected will set activated_layer
		selected_item = {&item_no_entity, 2};
		player_data.set_selected_item(selected_item);

		// However! If mouse_release is not true, do not set activated_layer
		player_data.try_place_entity(world_data, 0, 0);
		EXPECT_EQ(player_data.get_activated_layer(), nullptr);

		player_data.try_place_entity(world_data, 0, 0, true);
		EXPECT_EQ(player_data.get_activated_layer(),
		          &tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity));

		// Clicking again will NOT unset
		player_data.try_place_entity(world_data, 0, 0, true);
		EXPECT_EQ(player_data.get_activated_layer(),
		          &tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity));


		// Activated layer can be set to nullptr to unactivate layers
		player_data.set_activated_layer(nullptr);
		EXPECT_EQ(player_data.get_activated_layer(), nullptr);

	}

	TEST(player_data, try_pickup_entity_deactivate_layer) {
		// Picking up an entity wil unset activated layer if activated layer was the entity

		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};


		// Create entity
		auto item = jactorio::data::Item();

		auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->placeable = false;
		entity->set_item(&item);


		auto tile_proto = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity.get());


		world_data.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));

		// Entity is non-placeable, therefore when clicking on an entity, it will get activated_layer
		jactorio::data::item_stack selected_item = {&item, 2};
		player_data.set_selected_item(selected_item);

		// Set
		player_data.try_place_entity(world_data, 0, 0, true);
		EXPECT_EQ(player_data.get_activated_layer(),
		          &tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity));

		// Picking up entity will unset
		player_data.try_pickup(world_data, 0, 0, 1000);
		EXPECT_EQ(player_data.get_activated_layer(), nullptr);

	}

	TEST(player_data, try_pickup_entity) {
		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};

		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		// Create entity
		auto item = jactorio::data::Item();

		auto* entity = new jactorio::data::Container_entity();
		entity->pickup_time = 1.f;
		entity->set_item(&item);
		jactorio::data::data_manager::data_raw_add(jactorio::data::dataCategory::container_entity, "chester", entity);

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity);

		tiles[1].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity);

		// Create unique data by calling build event for prototype with layer
		{
			jactorio::game::World_data world_data{};
			entity->on_build(world_data, {},
			                 tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity), 0,
			                 jactorio::data::placementOrientation::up);
		}

		world_data.add_chunk(new jactorio::game::Chunk(0, 0, tiles));


		// 
		EXPECT_EQ(player_data.get_pickup_percentage(), 0.f);  // Defaults to 0
		player_data.try_pickup(world_data, 0, 2, 990);  // Will not attempt to pickup non entity tiles


		// Test pickup
		player_data.try_pickup(world_data, 0, 0, 30);
		EXPECT_EQ(player_data.get_pickup_percentage(), 0.5f);  // 50% picked up 30 ticks out of 60
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			entity);  // Not picked up yet - 10 more ticks needed to reach 1 second


		player_data.try_pickup(world_data, 1, 0, 30);  // Selecting different tile will reset pickup counter
		EXPECT_EQ(
			tiles[1].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			entity);  // Not picked up yet - 50 more to 1 second since counter reset

		player_data.try_pickup(world_data, 0, 0, 50);
		player_data.try_pickup(world_data, 0, 0, 10);
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			nullptr);  // Picked up, item given to inventory

		EXPECT_EQ(player_data.inventory_player[0].first, &item);
		EXPECT_EQ(player_data.inventory_player[0].second, 1);

		// Unique data for layer should have been deleted
		EXPECT_EQ(tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data, nullptr);
	}

	TEST(player_data, try_pickup_resource) {
		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};

		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		// Create resource entity
		auto item = jactorio::data::Item();

		auto* entity = new jactorio::data::Resource_entity();
		entity->pickup_time = 3.f;
		entity->set_item(&item);
		jactorio::data::data_manager::data_raw_add(jactorio::data::dataCategory::resource_entity, "diamond", entity);

		// Create world with the resource entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		world_data.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));


		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource, entity);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = new jactorio::data::Resource_entity_data(2);
		tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).unique_data = resource_data;


		//
		player_data.try_pickup(world_data, 0, 0, 180);
		// Resource entity should only become nullptr after all the resources are extracted
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource)
			,
			entity);

		EXPECT_EQ(resource_data->resource_amount, 1);

		EXPECT_EQ(player_data.inventory_player[0].first, &item);  // Gave 1 resource to player
		EXPECT_EQ(player_data.inventory_player[0].second, 1);


		// All resources extracted from resource entity, should now become nullptr
		player_data.try_pickup(world_data, 0, 0, 60);
		player_data.try_pickup(world_data, 0, 0, 60);
		player_data.try_pickup(world_data, 0, 0, 60);
		EXPECT_EQ(
			tiles[1].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource)
			,
			nullptr);  // Picked up, item given to inventory

		// Resource_data should be deleted

		EXPECT_EQ(player_data.inventory_player[0].first, &item);
		EXPECT_EQ(player_data.inventory_player[0].second, 2);  // Player has 2 of resource
	}

	TEST(player_data, try_pickup_layered) {
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};

		auto item = jactorio::data::Item();
		// Create world with the resource entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		world_data.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));


		// Resource entity
		auto* resource_entity = new jactorio::data::Resource_entity();
		resource_entity->pickup_time = 3.f;
		resource_entity->set_item(&item);
		jactorio::data::data_manager::data_raw_add(
			jactorio::data::dataCategory::resource_entity, "diamond", resource_entity);


		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource, resource_entity);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = new jactorio::data::Resource_entity_data(2);
		tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).unique_data = resource_data;


		// Other entity (e.g Container_entity)
		auto* container_entity = new jactorio::data::Container_entity();
		container_entity->pickup_time = 1.f;
		container_entity->set_item(&item);

		jactorio::data::data_manager::data_raw_add(jactorio::data::dataCategory::container_entity, "chester",
		                                           container_entity);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, container_entity);

		//
		player_data.try_pickup(world_data, 0, 0, 60);  // Container entity takes priority
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			nullptr);  // Picked up, item given to inventory


		// Now that container entity is picked up, resource entity is next
		player_data.try_pickup(world_data, 0, 0, 60);
		player_data.try_pickup(world_data, 0, 0, 60);
		EXPECT_EQ(resource_data->resource_amount, 2);  // Not picked up, still 60 more ticks required

		player_data.try_pickup(world_data, 0, 0, 60);
		EXPECT_EQ(resource_data->resource_amount, 1);  // Picked up
	}


	class Mock_entity final : public jactorio::data::Entity
	{
	public:
		mutable bool build_called = false;
		mutable bool remove_called = false;

		void on_build(jactorio::game::World_data& world_data, std::pair<int, int> world_coords,
		              jactorio::game::Chunk_tile_layer& tile_layer, uint16_t frame,
		              jactorio::data::placementOrientation orientation) const override {
			build_called = true;
		}

		void on_remove(jactorio::game::World_data& world_data, std::pair<int, int> world_coords,
		               jactorio::game::Chunk_tile_layer& tile_layer) const override {
			remove_called = true;
		}

		void on_r_show_gui(jactorio::game::Player_data& player_data,
		                   jactorio::game::Chunk_tile_layer* tile_layer) const override {
		}
	};

	TEST(player_data, try_place_call_on_build) {
		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};

		// The world tile must have a tile prototype
		auto tile_proto = jactorio::data::Tile();
		tile_proto.is_water = false;

		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		world_data.add_chunk(new jactorio::game::Chunk{0, 0, tiles});


		// Create entity
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);
		auto item = jactorio::data::Item{};

		auto* entity = new Mock_entity{};
		entity->set_item(&item);
		jactorio::data::data_manager::data_raw_add(jactorio::data::dataCategory::container_entity,
		                                           "", entity);


		jactorio::data::item_stack selected_item = {&item, 1};
		player_data.set_selected_item(selected_item);

		player_data.try_place_entity(world_data, 0, 0, true);

		ASSERT_TRUE(entity->build_called);
	}

	TEST(player_data, try_pickup_call_on_remove) {
		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};

		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		// Create entity
		auto item = jactorio::data::Item{};

		auto* entity = new Mock_entity{};
		entity->pickup_time = 1.f;
		entity->set_item(&item);
		jactorio::data::data_manager::data_raw_add(jactorio::data::dataCategory::container_entity,
		                                           "", entity);

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity);

		world_data.add_chunk(new jactorio::game::Chunk{0, 0, tiles});


		player_data.try_pickup(world_data, 0, 0, 60);

		ASSERT_TRUE(entity->remove_called);
	}


	class Mock_entity_can_build final : public jactorio::data::Entity
	{
	public:
		void on_r_show_gui(jactorio::game::Player_data& player_data, jactorio::game::Chunk_tile_layer* tile_layer) const
		override {
		}

		void on_build(jactorio::game::World_data& world_data,
		              std::pair<jactorio::game::World_data::world_coord, jactorio::game::World_data::world_coord> world_coords,
		              jactorio::game::Chunk_tile_layer& tile_layer, uint16_t frame,
		              jactorio::data::placementOrientation orientation) const override {
		}

		bool on_can_build(jactorio::game::World_data& world_data,
		                  std::pair<jactorio::game::World_data::world_coord, jactorio::game::World_data::world_coord>
		                  world_coords) override {
			return false;
		}
	};


	TEST(player_data, try_place_call_on_can_build) {
		jactorio::game::Player_data player_data{};
		jactorio::game::World_data world_data{};

		// The world tile must have a tile prototype
		auto tile_proto = jactorio::data::Tile();
		tile_proto.is_water = false;

		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		world_data.add_chunk(new jactorio::game::Chunk{0, 0, tiles});


		// Create entity
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);
		auto item = jactorio::data::Item{};

		auto* entity = new Mock_entity_can_build{};
		entity->set_item(&item);
		jactorio::data::data_manager::data_raw_add(jactorio::data::dataCategory::container_entity, "", entity);

		jactorio::data::item_stack selected_item = {&item, 1};
		player_data.set_selected_item(selected_item);

		player_data.try_place_entity(world_data, 0, 0, true);

		// Not placed because on_can_build returned false
		EXPECT_EQ(tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity), nullptr);
	}
}
