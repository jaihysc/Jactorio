// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/04/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/tile/tile.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace game
{
	class PlayerDataPlacementTest : public testing::Test
	{
	protected:
		jactorio::game::Player_data player_data_{};
		jactorio::game::World_data world_data_{};

		void TearDown() override {
			jactorio::data::clear_data();
		}

		class Mock_entity final : public jactorio::data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			mutable bool build_called  = false;
			mutable bool remove_called = false;


			J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(jactorio::data::Orientation,
			                                                                    jactorio::game::World_data&,
			                                                                    const jactorio::game::World_data::world_pair&) const override {
				return {0, 0};
			}

			void on_r_show_gui(jactorio::game::Player_data&,
			                   jactorio::game::Chunk_tile_layer*) const override {
			}

			// ======================================================================

			void on_build(jactorio::game::World_data&,
			              const jactorio::game::World_data::world_pair&,
			              jactorio::game::Chunk_tile_layer&,
			              jactorio::data::Orientation) const override {
				build_called = true;
			}

			void on_remove(jactorio::game::World_data&,
			               const jactorio::game::World_data::world_pair&,
			               jactorio::game::Chunk_tile_layer&) const override {
				remove_called = true;
			}
		};

		// Creates the base tile and entity at world coords
		void set_entity_coords(const int world_x,
		                       const int world_y,
		                       jactorio::data::Tile* tile_proto,
		                       jactorio::data::Entity* entity_proto) const {
			world_data_.get_tile(world_x, world_y)
			           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::base).prototype_data = tile_proto;

			world_data_.get_tile(world_x, world_y)
			           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = entity_proto;
		}
	};

	TEST_F(PlayerDataPlacementTest, RotatePlacementOrientation) {
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::up);

		player_data_.rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::right);

		player_data_.rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::down);

		player_data_.rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::left);

		player_data_.rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::up);
	}

	TEST_F(PlayerDataPlacementTest, CounterRotatePlacementOrientation) {
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::up);

		player_data_.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::left);

		player_data_.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::down);

		player_data_.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::right);

		player_data_.counter_rotate_placement_orientation();
		EXPECT_EQ(player_data_.placement_orientation, jactorio::data::Orientation::up);
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceEntity) {
		// Create entity
		auto item           = jactorio::data::Item();
		auto item_no_entity = jactorio::data::Item();  // Does not hold an entity reference

		auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->set_item(&item);


		auto entity2 = std::make_unique<jactorio::data::Container_entity>();


		auto tile_proto     = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[1].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[1].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity2.get());

		world_data_.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));

		// Edge cases
		player_data_.try_place_entity(world_data_, 0, 0);  // Placing with no items selected

		jactorio::data::item_stack selected_item = {&item_no_entity, 2};
		player_data_.set_selected_item(selected_item);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity.get());

		player_data_.try_place_entity(world_data_, 0, 0);  // Item holds no reference to an entity
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			entity.get());  // Should not delete item at this location


		// Placement tests

		// Place at 0, 0
		selected_item = {&item, 2};
		player_data_.set_selected_item(selected_item);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, nullptr);

		player_data_.try_place_entity(world_data_, 0, 0);  // Place on empty tile 0, 0

		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			entity.get());
		EXPECT_EQ(player_data_.get_selected_item()->second, 1);  // 1 less item 

		// The on_build() method should get called, creating unique data on the tile which holds the inventory
		EXPECT_NE(tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data, nullptr);


		// Do not place at 1, 0 
		player_data_.try_place_entity(world_data_, 1, 0);  // A tile already exists on 1, 0 - Should not override it
		EXPECT_EQ(
			tiles[1].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			entity2.get());
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceEntityActivateLayer) {
		// Create entity
		auto item           = jactorio::data::Item();
		auto item_no_entity = jactorio::data::Item();  // Does not hold an entity reference

		auto entity       = std::make_unique<jactorio::data::Container_entity>();
		entity->placeable = true;
		entity->set_item(&item);


		auto tile_proto     = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity.get());


		world_data_.add_chunk(new jactorio::game::Chunk(0, 0, tiles));

		// If selected item's entity is placeable, do not set activated_layer
		jactorio::data::item_stack selected_item = {&item, 2};
		player_data_.set_selected_item(selected_item);

		player_data_.try_place_entity(world_data_, 0, 0, true);
		EXPECT_EQ(player_data_.get_activated_layer(), nullptr);

		// Clicking on an entity with no placeable items selected will set activated_layer
		selected_item = {&item_no_entity, 2};
		player_data_.set_selected_item(selected_item);

		// However! If mouse_release is not true, do not set activated_layer
		player_data_.try_place_entity(world_data_, 0, 0);
		EXPECT_EQ(player_data_.get_activated_layer(), nullptr);

		player_data_.try_place_entity(world_data_, 0, 0, true);
		EXPECT_EQ(player_data_.get_activated_layer(),
		          &tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity));

		// Clicking again will NOT unset
		player_data_.try_place_entity(world_data_, 0, 0, true);
		EXPECT_EQ(player_data_.get_activated_layer(),
		          &tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity));


		// Activated layer can be set to nullptr to unactivate layers
		player_data_.set_activated_layer(nullptr);
		EXPECT_EQ(player_data_.get_activated_layer(), nullptr);

	}

	TEST_F(PlayerDataPlacementTest, TryPickupEntityDeactivateLayer) {
		// Picking up an entity wil unset activated layer if activated layer was the entity

		// Create entity
		auto item = jactorio::data::Item();

		auto entity       = std::make_unique<jactorio::data::Container_entity>();
		entity->placeable = false;
		entity->set_item(&item);


		auto tile_proto     = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity.get());


		world_data_.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));

		// Entity is non-placeable, therefore when clicking on an entity, it will get activated_layer
		jactorio::data::item_stack selected_item = {&item, 2};
		player_data_.set_selected_item(selected_item);

		// Set
		player_data_.try_place_entity(world_data_, 0, 0, true);
		EXPECT_EQ(player_data_.get_activated_layer(),
		          &tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity));

		// Picking up entity will unset
		player_data_.try_pickup(world_data_, 0, 0, 1000);
		EXPECT_EQ(player_data_.get_activated_layer(), nullptr);

	}

	TEST_F(PlayerDataPlacementTest, try_pickup_entity) {
		// Create entity
		auto item = jactorio::data::Item();

		auto* entity        = new jactorio::data::Container_entity();
		entity->pickup_time = 1.f;
		entity->set_item(&item);
		jactorio::data::data_raw_add("chester", entity);

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity);

		tiles[1].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity);

		// Create unique data by calling build event for prototype with layer
		{
			jactorio::game::World_data world_data{};
			entity->on_build(world_data,
			                 {},
			                 tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity),
			                 jactorio::data::Orientation::up);
		}

		world_data_.add_chunk(new jactorio::game::Chunk(0, 0, tiles));


		// 
		EXPECT_EQ(player_data_.get_pickup_percentage(), 0.f);  // Defaults to 0
		player_data_.try_pickup(world_data_, 0, 2, 990);  // Will not attempt to pickup non entity tiles


		// Test pickup
		player_data_.try_pickup(world_data_, 0, 0, 30);
		EXPECT_EQ(player_data_.get_pickup_percentage(), 0.5f);  // 50% picked up 30 ticks out of 60
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			entity);  // Not picked up yet - 10 more ticks needed to reach 1 second


		player_data_.try_pickup(world_data_, 1, 0, 30);  // Selecting different tile will reset pickup counter
		EXPECT_EQ(
			tiles[1].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			entity);  // Not picked up yet - 50 more to 1 second since counter reset

		player_data_.try_pickup(world_data_, 0, 0, 50);
		player_data_.try_pickup(world_data_, 0, 0, 10);
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			nullptr);  // Picked up, item given to inventory

		EXPECT_EQ(player_data_.inventory_player[0].first, &item);
		EXPECT_EQ(player_data_.inventory_player[0].second, 1);

		// Unique data for layer should have been deleted
		EXPECT_EQ(tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data, nullptr);
	}

	TEST_F(PlayerDataPlacementTest, TryPickupResource) {
		// Create resource entity
		auto item = jactorio::data::Item();

		auto* entity        = new jactorio::data::Resource_entity();
		entity->pickup_time = 3.f;
		entity->set_item(&item);
		jactorio::data::data_raw_add("diamond", entity);

		// Create world with the resource entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		world_data_.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));


		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource, entity);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = new jactorio::data::Resource_entity_data(2);
		tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).unique_data = resource_data;


		//
		player_data_.try_pickup(world_data_, 0, 0, 180);
		// Resource entity should only become nullptr after all the resources are extracted
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource),
			entity);

		EXPECT_EQ(resource_data->resource_amount, 1);

		EXPECT_EQ(player_data_.inventory_player[0].first, &item);  // Gave 1 resource to player
		EXPECT_EQ(player_data_.inventory_player[0].second, 1);


		// All resources extracted from resource entity, should now become nullptr
		player_data_.try_pickup(world_data_, 0, 0, 60);
		player_data_.try_pickup(world_data_, 0, 0, 60);
		player_data_.try_pickup(world_data_, 0, 0, 60);
		EXPECT_EQ(
			tiles[1].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource),
			nullptr);  // Picked up, item given to inventory

		// Resource_data should be deleted

		EXPECT_EQ(player_data_.inventory_player[0].first, &item);
		EXPECT_EQ(player_data_.inventory_player[0].second, 2);  // Player has 2 of resource
	}

	TEST_F(PlayerDataPlacementTest, TryPickupLayered) {
		auto item = jactorio::data::Item();
		// Create world with the resource entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		world_data_.add_chunk(
			new jactorio::game::Chunk(0, 0, tiles));


		// Resource entity
		auto* resource_entity        = new jactorio::data::Resource_entity();
		resource_entity->pickup_time = 3.f;
		resource_entity->set_item(&item);
		jactorio::data::data_raw_add("diamond", resource_entity);


		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource, resource_entity);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = new jactorio::data::Resource_entity_data(2);
		tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).unique_data = resource_data;


		// Other entity (e.g Container_entity)
		auto* container_entity        = new jactorio::data::Container_entity();
		container_entity->pickup_time = 1.f;
		container_entity->set_item(&item);

		jactorio::data::data_raw_add("chester", container_entity);

		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, container_entity);

		//
		player_data_.try_pickup(world_data_, 0, 0, 60);  // Container entity takes priority
		EXPECT_EQ(
			tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			nullptr);  // Picked up, item given to inventory


		// Now that container entity is picked up, resource entity is next
		player_data_.try_pickup(world_data_, 0, 0, 60);
		player_data_.try_pickup(world_data_, 0, 0, 60);
		EXPECT_EQ(resource_data->resource_amount, 2);  // Not picked up, still 60 more ticks required

		player_data_.try_pickup(world_data_, 0, 0, 60);
		EXPECT_EQ(resource_data->resource_amount, 1);  // Picked up
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceCallOnBuild) {
		// The world tile must have a tile prototype
		auto tile_proto     = jactorio::data::Tile();
		tile_proto.is_water = false;

		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		world_data_.add_chunk(new jactorio::game::Chunk{0, 0, tiles});


		// Create entity

		auto item = jactorio::data::Item{};

		auto* entity = new Mock_entity{};
		entity->set_item(&item);
		jactorio::data::data_raw_add("", entity);


		jactorio::data::item_stack selected_item = {&item, 1};
		player_data_.set_selected_item(selected_item);

		player_data_.try_place_entity(world_data_, 0, 0, true);

		ASSERT_TRUE(entity->build_called);
	}

	TEST_F(PlayerDataPlacementTest, TryPickupCallOnRemove) {
		// Create entity
		auto item = jactorio::data::Item{};

		auto* entity        = new Mock_entity{};
		entity->pickup_time = 1.f;
		entity->set_item(&item);
		jactorio::data::data_raw_add("", entity);

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity);

		world_data_.add_chunk(new jactorio::game::Chunk{0, 0, tiles});


		player_data_.try_pickup(world_data_, 0, 0, 60);

		ASSERT_TRUE(entity->remove_called);
	}


	TEST_F(PlayerDataPlacementTest, TryPlaceCallOnCanBuild) {
		class Mock_entity_can_build final : public jactorio::data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(jactorio::data::Orientation,
			                                                                    jactorio::game::World_data&,
			                                                                    const jactorio::game::World_data::world_pair&) const override {
				return {0, 0};
			}

			void on_r_show_gui(jactorio::game::Player_data&, jactorio::game::Chunk_tile_layer*) const override {
			}

			void on_build(jactorio::game::World_data&,
                          const jactorio::game::World_data::world_pair&,
			              jactorio::game::Chunk_tile_layer&,
			              jactorio::data::Orientation) const override {
			}

			void on_remove(jactorio::game::World_data&,
			               const jactorio::game::World_data::world_pair&,
			               jactorio::game::Chunk_tile_layer&) const override {
			}

			// ======================================================================

			J_NODISCARD bool on_can_build(const jactorio::game::World_data&,
							              const jactorio::game::World_data::world_pair&) const override {
				return false;
			}
		};

		// The world tile must have a tile prototype
		auto tile_proto     = jactorio::data::Tile();
		tile_proto.is_water = false;

		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

		world_data_.add_chunk(new jactorio::game::Chunk{0, 0, tiles});


		// Create entity

		auto item = jactorio::data::Item{};

		auto* entity = new Mock_entity_can_build{};
		entity->set_item(&item);
		jactorio::data::data_raw_add("", entity);

		jactorio::data::item_stack selected_item = {&item, 1};
		player_data_.set_selected_item(selected_item);

		player_data_.try_place_entity(world_data_, 0, 0, true);

		// Not placed because on_can_build returned false
		EXPECT_EQ(tiles[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity), nullptr);
	}


	TEST_F(PlayerDataPlacementTest, TryPlaceCallOnNeighborUpdate) {
		// Placing or removing an entity should call on_neighbor_update for the 4 neighbors in all directions

		class Mock_entity final : public jactorio::data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(jactorio::data::Orientation,
			                                                                    jactorio::game::World_data&,
			                                                                    const jactorio::game::World_data::world_pair&) const override {
				return {0, 0};
			}

			void on_r_show_gui(jactorio::game::Player_data&, jactorio::game::Chunk_tile_layer*) const override {
			}

			void on_build(jactorio::game::World_data&,
                          const jactorio::game::World_data::world_pair&,
			              jactorio::game::Chunk_tile_layer&,
			              jactorio::data::Orientation) const override {
			}


			void on_remove(jactorio::game::World_data&,
			               const jactorio::game::World_data::world_pair&,
			               jactorio::game::Chunk_tile_layer&) const override {
			}

			// ======================================================================
			mutable int on_update_called = 0;


			void on_neighbor_update(jactorio::game::World_data&,
			                        const jactorio::game::World_data::world_pair&,
			                        const jactorio::game::World_data::world_pair&,
			                        jactorio::data::Orientation) const override {
				on_update_called++;
			}
		};

		/*
		 *     [ ]
		 * [ ] [x] [ ]
		 *     [ ]
		 */

		// Tile
		auto tile_proto     = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Item

		auto item = jactorio::data::Item{};

		// Entity
		auto* entity_proto = new Mock_entity{};
		entity_proto->set_item(&item);
		jactorio::data::data_raw_add("", entity_proto);

		world_data_.add_chunk(new jactorio::game::Chunk{0, 0});

		world_data_.get_tile(1, 1)
		           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::base).prototype_data = &tile_proto;

		set_entity_coords(1, 0, &tile_proto, entity_proto);
		set_entity_coords(2, 1, &tile_proto, entity_proto);
		set_entity_coords(1, 2, &tile_proto, entity_proto);
		set_entity_coords(0, 1, &tile_proto, entity_proto);

		// ======================================================================

		jactorio::data::item_stack selected_item = {&item, 1};
		player_data_.set_selected_item(selected_item);

		player_data_.try_place_entity(world_data_, 1, 1, true);

		EXPECT_EQ(entity_proto->on_update_called, 4);
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceCallOnNeighborUpdate2x3) {
		// Placing or removing an entity should call on_neighbor_update for 10 adjacent tiles in clockwise order from top left

		class Mock_entity final : public jactorio::data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(jactorio::data::Orientation,
			                                                                    jactorio::game::World_data&,
			                                                                    const jactorio::game::World_data::world_pair&) const override {
				return {0, 0};
			}

			void on_r_show_gui(jactorio::game::Player_data&, jactorio::game::Chunk_tile_layer*) const override {
			}

			void on_build(jactorio::game::World_data&,
                          const jactorio::game::World_data::world_pair&,
			              jactorio::game::Chunk_tile_layer&,
			              jactorio::data::Orientation) const override {
			}


			void on_remove(jactorio::game::World_data&,
			               const jactorio::game::World_data::world_pair&,
			               jactorio::game::Chunk_tile_layer&) const override {
			}

			// ======================================================================
			mutable std::vector<std::pair<jactorio::game::World_data::world_coord, jactorio::game::World_data::world_coord>>
			coords;

			void on_neighbor_update(jactorio::game::World_data&,
			                        const jactorio::game::World_data::world_pair& emit_world_coords,
			                        const jactorio::game::World_data::world_pair& receive_world_coords,
			                        jactorio::data::Orientation) const override {
				EXPECT_EQ(emit_world_coords.first, 1);
				EXPECT_EQ(emit_world_coords.second, 1);
				coords.push_back(receive_world_coords);
			}
		};

		/*
		 *     [1] [2]
		 * [A] [x] [x] [3]
		 * [9] [x] [x] [4]
		 * [8] [x] [x] [5]
		 *     [7] [6]
		 */

		// Tile
		auto tile_proto     = jactorio::data::Tile();
		tile_proto.is_water = false;

		// Item

		auto item = jactorio::data::Item{};

		// Entity
		auto* entity_proto        = new Mock_entity{};
		entity_proto->tile_width  = 2;
		entity_proto->tile_height = 3;
		entity_proto->set_item(&item);
		jactorio::data::data_raw_add("", entity_proto);

		world_data_.add_chunk(new jactorio::game::Chunk{0, 0});

		// Set tiles so entity can be placed on it
		for (int y = 1; y < 4; ++y) {
			for (int x = 1; x < 3; ++x) {
				world_data_.get_tile(x, y)
				           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::base).prototype_data = &tile_proto;
			}
		}

		// Set entity around border
		for (int x = 1; x <= 2; ++x) {
			set_entity_coords(x, 0, &tile_proto, entity_proto);
		}
		for (int y = 1; y <= 3; ++y) {
			set_entity_coords(3, y, &tile_proto, entity_proto);
		}
		for (int x = 2; x >= 1; --x) {
			set_entity_coords(x, 4, &tile_proto, entity_proto);
		}
		for (int y = 3; y >= 1; --y) {
			set_entity_coords(0, y, &tile_proto, entity_proto);
		}

		// ======================================================================

		jactorio::data::item_stack selected_item = {&item, 1};
		player_data_.set_selected_item(selected_item);

		player_data_.try_place_entity(world_data_, 1, 1, true);
		ASSERT_EQ(entity_proto->coords.size(), 10);

#define VALIDATE_COORDS(index, x, y)\
		{\
		auto pair = std::pair<jactorio::game::World_data::world_coord, jactorio::game::World_data::world_coord>{x, y};\
		EXPECT_EQ(entity_proto->coords[index], pair);\
		}

		VALIDATE_COORDS(0, 1, 0);
		VALIDATE_COORDS(1, 2, 0);

		VALIDATE_COORDS(2, 3, 1);
		VALIDATE_COORDS(3, 3, 2);
		VALIDATE_COORDS(4, 3, 3);

		VALIDATE_COORDS(5, 2, 4);
		VALIDATE_COORDS(6, 1, 4);

		VALIDATE_COORDS(7, 0, 3);
		VALIDATE_COORDS(8, 0, 2);
		VALIDATE_COORDS(9, 0, 1);

		// ======================================================================

		player_data_.try_pickup(world_data_, 1, 1, 9999);
		ASSERT_EQ(entity_proto->coords.size(), 20);
	}
}
