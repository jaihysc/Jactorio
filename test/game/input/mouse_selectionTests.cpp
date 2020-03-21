// 
// mouse_selectionTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/05/2020
// Last modified: 03/21/2020
// 

#include <gtest/gtest.h>


#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "game/input/mouse_selection.h"

namespace game
{
#define MOUSE_SELECTION_TEST_HEADER\
		jactorio::game::World_data world_data{};\
		world_data.add_chunk(new jactorio::game::Chunk(0, 0, nullptr));\
		jactorio::game::Mouse_selection mouse_selection{};

	TEST(mouse_selection_overlay, draw_overlay_item_selected_placeable) {
		// An item is selected
		// Clear last and draw new
		MOUSE_SELECTION_TEST_HEADER

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;
		entity.placeable = true;

		// Should set item's sprite at overlay layer at world position 0, 0
		mouse_selection.draw_overlay(world_data, &entity, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			&entity_sprite
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouse_selection.draw_overlay(world_data, &entity, 1, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			nullptr
		);
		EXPECT_EQ(
			world_data.get_tile_world_coords(1, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			&entity_sprite
		);

	}

	TEST(mouse_selection_overlay, draw_overlay_item_selected_non_placeable) {
		// A non placeable item is selected
		// Clear last and draw nothing new
		MOUSE_SELECTION_TEST_HEADER

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;
		entity.placeable = false;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouse_selection.draw_overlay(world_data, &entity, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data
			,
			nullptr);
	}

	TEST(mouse_selection_overlay, draw_overlay_item_selected_non_placeable_hover_entity) {
		// A non placeable item is selected over an entity
		// Clear last and draw cursor
		MOUSE_SELECTION_TEST_HEADER

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;
		entity.placeable = false;

		world_data.get_tile_world_coords(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &entity;

		auto guard = jactorio::core::Resource_guard(jactorio::data::data_manager::clear_data);
		auto* cursor_sprite = new jactorio::data::Sprite{};

		jactorio::data::data_manager::data_raw_add(
			jactorio::data::data_category::sprite, "__core__/cursor-select", cursor_sprite);


		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouse_selection.draw_overlay(world_data, &entity, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data
			,
			cursor_sprite);
	}

	TEST(mouse_selection_overlay, draw_overlay_not_selected) {
		// NO item is selected
		// CLear last and draw nothing new
		MOUSE_SELECTION_TEST_HEADER

		// Should set item's sprite at overlay layer at world position 0, 0
		mouse_selection.draw_overlay(world_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			nullptr
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouse_selection.draw_overlay(world_data, nullptr, 1, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			nullptr
		);
		EXPECT_EQ(
			world_data.get_tile_world_coords(1, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			nullptr
		);

	}

	TEST(mouse_selection_overlay, draw_overlay_not_selected_entity_hover) {
		// NO item is selected, but cursor is hovered over an entity
		// Draw selection cursor
		MOUSE_SELECTION_TEST_HEADER

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;

		auto guard = jactorio::core::Resource_guard(jactorio::data::data_manager::clear_data);
		auto* cursor_sprite = new jactorio::data::Sprite{};

		jactorio::data::data_manager::data_raw_add(
			jactorio::data::data_category::sprite, "__core__/cursor-select", cursor_sprite);

		world_data.get_tile_world_coords(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &entity;


		mouse_selection.draw_overlay(world_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data
			,
			cursor_sprite);
	}

	TEST(mouse_selection_overlay, draw_overlay_not_selected_resource_entity_hover) {
		// NO item is selected, but cursor is hovered over a resource entity
		// Draw selection cursor
		MOUSE_SELECTION_TEST_HEADER

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;
		entity.placeable = false;

		
		auto guard = jactorio::core::Resource_guard(jactorio::data::data_manager::clear_data);
		auto* cursor_sprite = new jactorio::data::Sprite{};

		jactorio::data::data_manager::data_raw_add(
			jactorio::data::data_category::sprite, "__core__/cursor-select", cursor_sprite);

		world_data.get_tile_world_coords(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &entity;


		mouse_selection.draw_overlay(world_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data
			,
			cursor_sprite);
	}

	TEST(mouse_selection_overlay, draw_overlay_no_world) {
		// When selecting no world / chunk not generated
		// Attempt to clear last tile and do nothing

		jactorio::game::World_data world_data{};
		jactorio::game::Mouse_selection mouse_selection{};

		mouse_selection.draw_overlay(world_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
	}
}
