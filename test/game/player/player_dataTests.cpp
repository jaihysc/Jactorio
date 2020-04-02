// 
// player_dataTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/31/2020
// Last modified: 04/02/2020
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
		jactorio::data::data_manager::data_raw_add(jactorio::data::data_category::container_entity, "chester", entity);

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
		jactorio::data::data_manager::data_raw_add(jactorio::data::data_category::resource_entity, "diamond", entity);

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
			jactorio::data::data_category::resource_entity, "diamond", resource_entity);


		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::resource, resource_entity);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = new jactorio::data::Resource_entity_data(2);
		tiles[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).unique_data = resource_data;


		// Other entity (e.g Container_entity)
		auto* container_entity = new jactorio::data::Container_entity();
		container_entity->pickup_time = 1.f;
		container_entity->set_item(&item);

		jactorio::data::data_manager::data_raw_add(jactorio::data::data_category::container_entity, "chester",
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
		jactorio::data::data_manager::data_raw_add(jactorio::data::data_category::container_entity,
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
		jactorio::data::data_manager::data_raw_add(jactorio::data::data_category::container_entity,
		                                           "", entity);

		// Create world with entity at 0, 0
		auto* tiles = new jactorio::game::Chunk_tile[1024];
		tiles[0].set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, entity);

		world_data.add_chunk(new jactorio::game::Chunk{0, 0, tiles});


		player_data.try_pickup(world_data, 0, 0, 60);

		ASSERT_TRUE(entity->remove_called);
	}

	//
	//
	//
	//
	// PLAYER INVENTORY MENU
	//
	//
	//
	//

	// Create the cursor prototype hardcoded when an item is selected
#define INVENTORY_CURSOR\
	auto* cursor = new jactorio::data::Item();\
		jactorio::data::data_manager::data_raw_add(\
			jactorio::data::data_category::item, \
			jactorio::game::Player_data::inventory_selected_cursor_iname, \
			cursor);
	TEST(player_data, inventory_lclick_select_item_by_reference) {
		// Left click on a slot picks up items by reference
		// The inventory slot becomes the cursor
		// The cursor holds the item
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();


		// Position 3 should have the 50 items + item prototype after moving
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 50;

		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		EXPECT_EQ(player_data.inventory_player[0].first, cursor);
		EXPECT_EQ(player_data.inventory_player[0].second, 0);

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);
	}

	TEST(player_data, inventory_deselect_referenced_item) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot deselects the item
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select
			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Deselect


			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
		player_data.clear_player_inventory();
		// Right click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select
			player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Deselect


			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}

	TEST(player_data, inventory_deselect_referenced_item_2_inventories) {
		// Left click on a slot picks up items by reference
		// Left / right clicking again on the same slot in another inventory however will not deselect the item
		jactorio::game::Player_data player_data{};

		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();

		player_data.clear_player_inventory();
		// Left click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select

			// Deselect into inv_2
			jactorio::data::item_stack inv_2[10];
			player_data.inventory_click(0, 0, true, inv_2);  // Deselect

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);

			// Selected cursor should no longer exist in inventory_player
			EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
		}
		player_data.clear_player_inventory();
		// Right click into another inventory
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select

			// Deselect into inv_2
			jactorio::data::item_stack inv_2[10];
			player_data.inventory_click(0, 1, true, inv_2);  // Will NOT Deselect since in another inventory

			EXPECT_EQ(inv_2[0].first, item.get());
			EXPECT_EQ(inv_2[0].second, 1);

			// Cursor still holds 49
			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 49);

			// Selected cursor should STILL exist in inventory_player since not deselected
			EXPECT_NE(player_data.inventory_player[0].first, nullptr);
		}
	}

	TEST(player_data, inventory_move_referenced_item) {
		// Left click on a slot picks up items by reference
		// Left click on index 3 to drop off the item at index

		// Index 0 (origin) should be empty
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		// Create the cursor prototype
		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();

		// Left click
		{
			// Position 3 should have the 50 items + item prototype after moving
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 50;

			player_data.inventory_click(0, 0, true, player_data.inventory_player);  // Select item
			player_data.inventory_click(3, 0, true, player_data.inventory_player);  // Drop item off


			EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
			EXPECT_EQ(player_data.inventory_player[0].second, 0);

			EXPECT_EQ(player_data.inventory_player[3].first, item.get());
			EXPECT_EQ(player_data.inventory_player[3].second, 50);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}
	}


	TEST(player_data, inventory_Rclick_select_item_by_unique) {
		// Right click on a slot creates a new inventory slot in the cursor and places half from the inventory into it
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();


		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 40;

		player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half

		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 20;

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 20);
	}

	TEST(player_data, inventory_drop_single_unique_item) {
		// Right click on item to pick up half into cursor
		// Right click on empty slot to drop 1 off from the cursor
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();

		// Drop one on another location
		{
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 10;

			player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half

			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 5);


			player_data.inventory_click(3, 1, true, player_data.inventory_player);  // Drop 1 at index 3

			// Should remain unchanged
			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			// Loses 1
			cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Gain 1
			EXPECT_EQ(player_data.inventory_player[3].first, item.get());
			EXPECT_EQ(player_data.inventory_player[3].second, 1);
		}

		player_data.clear_player_inventory();
		player_data.reset_inventory_variables();
		// Drop 1 on the original item stack where half was taken from
		{
			player_data.inventory_player[0].first = item.get();
			player_data.inventory_player[0].second = 10;

			player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half
			player_data.
				inventory_click(0, 1, true, player_data.inventory_player);  // Drop 1 one the stack it picked up from

			// Loses 1
			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);

			// Half (5) plus 1 = 6
			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 6);
		}
	}

	TEST(player_data, inventory_drop_stack_unique_item) {
		// Right click on item to pick up half into cursor
		// Left click on empty slot to drop entire stack off from the cursor
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();


		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;


		player_data.inventory_click(0, 1, true, player_data.inventory_player);  // Pick up half

		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 5);

		auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 5);


		player_data.inventory_click(3, 0, true, player_data.inventory_player);  // Drop stack at index 3

		// Should remain unchanged
		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 5);

		// Empty
		cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Gain 10
		EXPECT_EQ(player_data.inventory_player[3].first, item.get());
		EXPECT_EQ(player_data.inventory_player[3].second, 5);
	}

	TEST(player_data, inventory_click_empty_slot) {
		// Left click on empty slot
		// Should remain unchanged
		jactorio::game::Player_data player_data{};

		player_data.inventory_player[0].first = nullptr;
		player_data.inventory_player[0].second = 0;

		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
		EXPECT_EQ(player_data.inventory_player[0].second, 0);

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);
	}


	// Increment / decrement selected item
	TEST(player_data, increment_selected_item) {
		// If player selects item by "unique" or "reference",
		// It should function the same as it only modifies the cursor item stack
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			player_data.inventory_click(0, 1, true, player_data.inventory_player);

			// Check if item was incremented
			EXPECT_EQ(player_data.increment_selected_item(), true);

			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 6);  // This incremented by 1
		}

		// Drop item down at inv slot 1
		{
			player_data.inventory_click(1, 0, true, player_data.inventory_player);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_data.inventory_player[1].first, item.get());
			EXPECT_EQ(player_data.inventory_player[1].second, 6);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST(player_data, increment_selected_item_exceed_item_stack) {
		// Attempting to increment an item exceeding item stack returns false and fails the increment
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 50;

		// Pickup
		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		// Failed to add item: Item stack already full
		EXPECT_EQ(player_data.increment_selected_item(), false);

		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item->first, item.get());
		EXPECT_EQ(cursor_item->second, 50);  // This unchanged
	}


	TEST(player_data, decrement_selected_item_unique) {
		// If player selects item by "unique"
		// If decremented to 0, deselect the cursor item
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;

		// Pickup
		{
			// Pick up 5 of item, now selected
			player_data.inventory_click(0, 1, true, player_data.inventory_player);

			// Check if item was incremented
			EXPECT_EQ(player_data.decrement_selected_item(), true);

			EXPECT_EQ(player_data.inventory_player[0].first, item.get());
			EXPECT_EQ(player_data.inventory_player[0].second, 5);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item->first, item.get());
			EXPECT_EQ(cursor_item->second, 4);  // This decremented by 1
		}

		// Drop item down at inv slot 1
		{
			player_data.inventory_click(1, 0, true, player_data.inventory_player);

			// Inv now empty, contents in inv slot 1
			EXPECT_EQ(player_data.inventory_player[1].first, item.get());
			EXPECT_EQ(player_data.inventory_player[1].second, 4);

			const auto cursor_item = player_data.get_selected_item();
			EXPECT_EQ(cursor_item, nullptr);
		}

	}

	TEST(player_data, decrement_selected_item_reach_zero_reference) {
		// Selected by reference
		// If decremented to 0, deselect the cursor item
		// If the selected item is empty after decrementing, return false
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 1;

		// Pickup
		player_data.inventory_click(0, 0, true, player_data.inventory_player);

		EXPECT_EQ(player_data.decrement_selected_item(), false);

		// Cursor is nullptr: no item selected
		const auto cursor_item = player_data.get_selected_item();
		EXPECT_EQ(cursor_item, nullptr);

		// Should remove the selection cursor on the item
		EXPECT_EQ(player_data.inventory_player[0].first, nullptr);
		EXPECT_EQ(player_data.inventory_player[0].second, 0);
	}

	TEST(player_maanger, player_inventory_sort) {
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		const auto item2 = std::make_unique<jactorio::data::Item>();
		item2->stack_size = 10;

		// Item 1
		player_data.inventory_player[0].first = item.get();
		player_data.inventory_player[0].second = 10;

		player_data.inventory_player[10].first = item.get();
		player_data.inventory_player[10].second = 25;

		player_data.inventory_player[20].first = item.get();
		player_data.inventory_player[20].second = 25;

		player_data.inventory_player[13].first = item.get();
		player_data.inventory_player[13].second = 20;

		player_data.inventory_player[14].first = item.get();
		player_data.inventory_player[14].second = 30;

		// Item 2
		player_data.inventory_player[31].first = item2.get();
		player_data.inventory_player[31].second = 4;

		player_data.inventory_player[32].first = item2.get();
		player_data.inventory_player[32].second = 6;

		player_data.inventory_player[22].first = item2.get();
		player_data.inventory_player[22].second = 1;


		// Sorted inventory should be as follows
		// Item(count)
		// 1(50), 1(50), 1(10), 2(10), 2(1)
		player_data.inventory_sort();

		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 50);
		EXPECT_EQ(player_data.inventory_player[1].first, item.get());
		EXPECT_EQ(player_data.inventory_player[1].second, 50);
		EXPECT_EQ(player_data.inventory_player[2].first, item.get());
		EXPECT_EQ(player_data.inventory_player[2].second, 10);

		EXPECT_EQ(player_data.inventory_player[3].first, item2.get());
		EXPECT_EQ(player_data.inventory_player[3].second, 10);
		EXPECT_EQ(player_data.inventory_player[4].first, item2.get());
		EXPECT_EQ(player_data.inventory_player[4].second, 1);
	}

	TEST(player_maanger, player_inventory_sort2) {
		// Sorting will not move the item with inventory_selected_cursor_iname (to prevent breaking the inventory logic)
		jactorio::game::Player_data player_data{};
		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		INVENTORY_CURSOR

		player_data.inventory_player[10].first = cursor;
		player_data.inventory_player[10].second = 0;


		player_data.inventory_sort();

		EXPECT_EQ(player_data.inventory_player[10].first, cursor);
		EXPECT_EQ(player_data.inventory_player[10].second, 0);

		// There should have been no new cursors created anywhere
		for (int i = 0; i < jactorio::game::Player_data::inventory_size; ++i) {
			if (i == 10)
				continue;
			EXPECT_NE(player_data.inventory_player[i].first, cursor);
		}
	}

	TEST(player_maanger, player_inventory_sort_full) {
		// Sorting the inventory when it is full should also work
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		for (auto& i : player_data.inventory_player) {
			i.first = item.get();
			i.second = 50;
		}

		player_data.inventory_sort();


		// There should have been no new cursors created anywhere
		for (auto& i : player_data.inventory_player) {
			EXPECT_EQ(i.first, item.get());
			EXPECT_EQ(i.second, 50);
		}
	}

	TEST(player_maanger, player_inventory_sort_item_exceding_stack) {
		// If there is an item which exceeds its stack size, do not attempt to stack into it
		jactorio::game::Player_data player_data{};

		const auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		player_data.inventory_player[10].first = item.get();
		player_data.inventory_player[10].second = 100;

		player_data.inventory_player[11].first = item.get();
		player_data.inventory_player[11].second = 100;

		player_data.inventory_player[12].first = item.get();
		player_data.inventory_player[12].second = 10;

		player_data.inventory_sort();

		EXPECT_EQ(player_data.inventory_player[0].first, item.get());
		EXPECT_EQ(player_data.inventory_player[0].second, 100);

		EXPECT_EQ(player_data.inventory_player[1].first, item.get());
		EXPECT_EQ(player_data.inventory_player[1].second, 100);

		EXPECT_EQ(player_data.inventory_player[2].first, item.get());
		EXPECT_EQ(player_data.inventory_player[2].second, 10);
	}

	//
	//
	//
	//
	// RECIPE MENU
	//
	//
	//
	//

	TEST(player_data, recipe_select_recipe_group) {
		jactorio::game::Player_data player_data{};
		player_data.recipe_group_select(1);

		EXPECT_EQ(player_data.recipe_group_get_selected(), 1);
	}


	TEST(player_data, recipe_queue) {
		// Queueing 2 recipes will remove the ingredients from the player inventory, but will not return any products
		// since recipe_craft_tick() is not called

		jactorio::game::Player_data player_data{};

		jactorio::core::Resource_guard guard(jactorio::data::data_manager::clear_data);

		const auto item = new jactorio::data::Item();
		const auto item_product = new jactorio::data::Item();


		// Register items
		jactorio::data::data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-1", item);

		jactorio::data::data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-product", item_product);

		// Register recipes
		auto recipe = jactorio::data::Recipe();
		recipe.set_ingredients({{"item-1", 2}});
		recipe.set_product({"item-product", 1});
		recipe.set_crafting_time(1);

		// 10 of item in player inventory
		player_data.inventory_player[0] = {item, 10};

		// Queue 2 crafts
		player_data.recipe_queue(&recipe);
		player_data.recipe_queue(&recipe);

		// Used up 2 * 2 (4) items
		EXPECT_EQ(player_data.inventory_player[0].first, item);
		EXPECT_EQ(player_data.inventory_player[0].second, 6);

		EXPECT_EQ(player_data.inventory_player[1].first, nullptr);


		//


		// Output items should be in slot index 1 after 60 ticks (1 second) for each item
		player_data.recipe_craft_tick(30);  // Not done yet

		EXPECT_EQ(player_data.inventory_player[1].first, nullptr);

		player_data.recipe_craft_tick(90);

		EXPECT_EQ(player_data.inventory_player[1].first, item_product);
		EXPECT_EQ(player_data.inventory_player[1].second, 2);
	}


	// Creates a crafting recipes with the following crafting hierarchy
	/*
	 *        ---
	 *        |1|product
	 *   --3--- ---1--
	 *   |2|item1  | |item2
	 * -5- -10     ---
	 * | | | |
	 * --- ---
	 * sub1  sub2
	 */
	// 3 item1 + 1 item2 -> 1 product
	// 5 sub1 + 10 sub2 -> 2 item1
#define RECIPE_TEST_RECIPE\
	auto* item_product = new jactorio::data::Item();\
	data_manager::data_raw_add(\
		jactorio::data::data_category::item, "item-product", item_product);\
	\
	auto* item1 = new jactorio::data::Item();\
	data_manager::data_raw_add(\
		jactorio::data::data_category::item, "item-1", item1);\
	auto* item2 = new jactorio::data::Item();\
	data_manager::data_raw_add(\
		jactorio::data::data_category::item, "item-2", item2);\
	\
	auto* item_sub1 = new jactorio::data::Item();\
	data_manager::data_raw_add(\
		jactorio::data::data_category::item, "item-sub-1", item_sub1);\
	auto* item_sub2 = new jactorio::data::Item();\
	data_manager::data_raw_add(\
		jactorio::data::data_category::item, "item-sub-2", item_sub2);\
	\
	auto* final_recipe = new jactorio::data::Recipe();\
	final_recipe->set_ingredients({{"item-1", 3}, {"item-2", 1}});\
	final_recipe->set_product({"item-product", 1});\
	\
	data_manager::data_raw_add(\
		jactorio::data::data_category::recipe, "item-product-recipe", final_recipe);\
	\
	auto* item_recipe = new jactorio::data::Recipe();\
	item_recipe->set_ingredients({{"item-sub-1", 5}, {"item-sub-2", 10}});\
	item_recipe->set_product({"item-1", 2});\
	\
	data_manager::data_raw_add(\
		jactorio::data::data_category::recipe, "item-1-recipe", item_recipe);

	TEST(player_data, recipe_craft_resurse) {
		// Should recursively craft the product, crafting intermediate products as necessary
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		RECIPE_TEST_RECIPE

		player_data.inventory_player[0] = {item2, 1};

		player_data.inventory_player[1] = {item_sub1, 10};
		player_data.inventory_player[2] = {item_sub2, 20};

		player_data.recipe_craft_r(final_recipe);

		player_data.recipe_craft_tick(9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(player_data.inventory_player[0].first, item1);  // 1 extra item 1 from crafting
		EXPECT_EQ(player_data.inventory_player[0].second, 1);

		EXPECT_EQ(player_data.inventory_player[1].first, item_product);
		EXPECT_EQ(player_data.inventory_player[1].second, 1);

		EXPECT_EQ(player_data.get_crafting_item_deductions().size(), 0);
		EXPECT_EQ(player_data.get_crafting_item_extras().size(), 0);
	}

	TEST(player_data, recipe_craft_resurse2) {
		// Calculations for recursive crafting should also factor in the excess left by previous recipes

		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		RECIPE_TEST_RECIPE

		// All ingredients should be completely used up
		player_data.inventory_player[0] = {item2, 4};
		player_data.inventory_player[1] = {item_sub1, 30};
		player_data.inventory_player[2] = {item_sub2, 60};

		player_data.recipe_craft_r(final_recipe);
		player_data.recipe_craft_r(final_recipe);
		player_data.recipe_craft_r(final_recipe);

		// This should not craft item1 since there will be 3 in excess from the previous 3 crafting
		player_data.recipe_craft_r(final_recipe);

		player_data.recipe_craft_tick(9999);  // Should be enough ticks to finish crafting

		EXPECT_EQ(player_data.inventory_player[0].first, item_product);
		EXPECT_EQ(player_data.inventory_player[0].second, 4);

		EXPECT_EQ(player_data.get_crafting_item_deductions().size(), 0);
		EXPECT_EQ(player_data.get_crafting_item_extras().size(), 0);

		// Ensure there were no excess items
		for (int i = 1; i < jactorio::game::Player_data::inventory_size; ++i) {
			EXPECT_EQ(player_data.inventory_player[i].first, nullptr);
		}
	}

	TEST(player_data, recipe_can_craft) {
		// Should recursively step through a recipe and determine that it can be crafted
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-product", new jactorio::data::Item());


		auto* item1 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-1", item1);
		auto* item2 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-2", item2);

		auto* item_sub1 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-sub-1", item_sub1);
		auto* item_sub2 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-sub-2", item_sub2);

		auto* final_recipe = new jactorio::data::Recipe();
		final_recipe->set_ingredients({{"item-1", 3}, {"item-2", 1}});
		final_recipe->set_product({"item-product", 1});
		data_manager::data_raw_add(
			jactorio::data::data_category::recipe, "item-product-recipe", final_recipe);

		auto* item_recipe = new jactorio::data::Recipe();
		item_recipe->set_ingredients({{"item-sub-1", 5}, {"item-sub-2", 10}});
		item_recipe->set_product({"item-1", 2});
		data_manager::data_raw_add(
			jactorio::data::data_category::recipe, "item-1-recipe", item_recipe);

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		player_data.inventory_player[0] = {item1, 1};
		player_data.inventory_player[1] = {item2, 2};

		player_data.inventory_player[2] = {item_sub1, 5};
		player_data.inventory_player[3] = {item_sub2, 10};

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(player_data.recipe_can_craft(final_recipe, 1), true);
	}

	TEST(player_data, recipe_can_craft_invalid) {
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-product", new jactorio::data::Item());


		auto* item1 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-1", item1);
		auto* item2 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-2", item2);

		auto* item_sub1 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-sub-1", item_sub1);
		auto* item_sub2 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-sub-2", item_sub2);

		auto* final_recipe = new jactorio::data::Recipe();
		final_recipe->set_ingredients({{"item-1", 3}, {"item-2", 1}});
		final_recipe->set_product({"item-product", 1});
		data_manager::data_raw_add(
			jactorio::data::data_category::recipe, "item-product-recipe", final_recipe);

		auto* item_recipe = new jactorio::data::Recipe();
		item_recipe->set_ingredients({{"item-sub-1", 5}, {"item-sub-2", 10}});
		item_recipe->set_product({"item-1", 2});
		data_manager::data_raw_add(
			jactorio::data::data_category::recipe, "item-1-recipe", item_recipe);

		// 3-B + 1-C -> 1-A
		// 5-B1 + 10-B2 -> 2-B
		player_data.inventory_player[0] = {item1, 1};
		player_data.inventory_player[1] = {item2, 2};

		player_data.inventory_player[2] = {item_sub1, 5};
		// player_inventory[3] = {item_sub2, 10};  // Without this, the recipe cannot be crafted

		// Crafting item-product:
		// Item 1 requires one batch to be crafted
		// Crafting requires for item 2 met
		EXPECT_EQ(player_data.recipe_can_craft(final_recipe, 1), false);
	}

	TEST(player_data, recipe_can_craft_invalid2) {
		// When encountering the same items, it must account for the fact it has already been used earlier
		// Should recursively step through a recipe and determine that it canNOT in fact be crafted
		jactorio::game::Player_data player_data{};
		namespace data_manager = jactorio::data::data_manager;

		jactorio::core::Resource_guard guard(data_manager::clear_data);

		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-product", new jactorio::data::Item());


		auto* item1 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-1", item1);

		auto* item_sub1 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-sub-1", item_sub1);
		auto* item_sub2 = new jactorio::data::Item();
		data_manager::data_raw_add(
			jactorio::data::data_category::item, "item-sub-2", item_sub2);


		auto* final_recipe = new jactorio::data::Recipe();
		final_recipe->set_ingredients({{"item-1", 1}, {"item-sub-2", 5}});
		final_recipe->set_product({"item-product", 1});

		data_manager::data_raw_add(
			jactorio::data::data_category::recipe, "item-product-recipe", final_recipe);

		auto* item_recipe = new jactorio::data::Recipe();
		item_recipe->set_ingredients({{"item-sub-1", 10}, {"item-sub-2", 5}});
		item_recipe->set_product({"item-1", 2});

		data_manager::data_raw_add(
			jactorio::data::data_category::recipe, "item-1-recipe", item_recipe);

		// Final product: item-1 + item-sub-2
		// item-2: item-sub-1 + item-sub-2

		player_data.inventory_player[1] = {item_sub1, 10};
		player_data.inventory_player[2] = {item_sub2, 5};

		EXPECT_EQ(player_data.recipe_can_craft(final_recipe, 1), false);
	}

#undef RECIPE_TEST_RECIPE
}
