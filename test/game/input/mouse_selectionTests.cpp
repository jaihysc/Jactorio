// 
// mouse_selectionTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/05/2020
// Last modified: 04/08/2020
// 

#include <gtest/gtest.h>


#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "game/input/mouse_selection.h"

namespace game
{
#define MOUSE_SELECTION_TEST_HEADER\
		jactorio::game::World_data world_data{};\
		jactorio::game::Player_data player_data{};\
		player_data.set_player_world(&world_data);\
		\
		world_data.add_chunk(new jactorio::game::Chunk(0, 0));\
		jactorio::game::Mouse_selection mouse_selection{};

	// If the test involves the selection cursor, which is loaded in the data_manager
#define MOUSE_SELECTION_CURSOR\
		auto guard = jactorio::core::Resource_guard(jactorio::data::data_manager::clear_data);\
		auto* cursor_sprite = new jactorio::data::Sprite{};\
		\
		jactorio::data::data_manager::data_raw_add("__core__/cursor-select", cursor_sprite);


	TEST(mouse_selection_overlay, draw_overlay_item_selected_placeable) {
		// An item is selected
		// Clear last and draw new
		MOUSE_SELECTION_TEST_HEADER

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;
		entity.placeable = true;

		// Should set item's sprite at overlay layer at world position 0, 0
		mouse_selection.draw_overlay(player_data, &entity, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			&entity_sprite
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouse_selection.draw_overlay(player_data, &entity, 1, 0, jactorio::data::placementOrientation::up);
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
		mouse_selection.draw_overlay(player_data, &entity, 0, 0, jactorio::data::placementOrientation::up);
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
		MOUSE_SELECTION_CURSOR

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;
		entity.placeable = false;

		world_data.get_tile_world_coords(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &entity;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouse_selection.draw_overlay(player_data, &entity, 0, 0, jactorio::data::placementOrientation::up);
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
		mouse_selection.draw_overlay(player_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data
			,
			nullptr
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouse_selection.draw_overlay(player_data, nullptr, 1, 0, jactorio::data::placementOrientation::up);
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
		MOUSE_SELECTION_CURSOR

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;


		world_data.get_tile_world_coords(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &entity;


		mouse_selection.draw_overlay(player_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
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
		MOUSE_SELECTION_CURSOR

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;
		entity.placeable = false;


		world_data.get_tile_world_coords(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &entity;


		mouse_selection.draw_overlay(player_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
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
		jactorio::game::Player_data player_data{};
		player_data.set_player_world(&world_data);

		jactorio::game::Mouse_selection mouse_selection{};

		mouse_selection.draw_overlay(player_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
	}

	TEST(mouse_selection_overlay, draw_overlay_3x3) {
		MOUSE_SELECTION_TEST_HEADER
		MOUSE_SELECTION_CURSOR

		jactorio::data::Mining_drill drill{};
		drill.tile_width = 3;
		drill.tile_height = 3;

		jactorio::data::Sprite drill_sprite{};
		drill.sprite = &drill_sprite;

		mouse_selection.draw_overlay(player_data, &drill, 0, 0, jactorio::data::placementOrientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const jactorio::data::Prototype_base* data =
					world_data.get_tile_world_coords(x, y)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
					           prototype_data;
				EXPECT_EQ(data, &drill_sprite);
			}
		}


		// Removing the drawn overlay, no resource or entity data so no cursor is drawn
		mouse_selection.draw_overlay(player_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const jactorio::data::Prototype_base* data =
					world_data.get_tile_world_coords(x, y)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
					           prototype_data;
				EXPECT_EQ(data, nullptr);
			}
		}


		// Drawing a cursor should be 1 x 1
		world_data.get_tile_world_coords(0, 0) // Create an entity
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &drill;

		// Should not touch this
		jactorio::game::Chunk_tile_layer& untouched =
			world_data.get_tile_world_coords(1, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay);
		untouched.prototype_data = &drill;


		mouse_selection.draw_overlay(player_data, nullptr,
		                             0, 0, jactorio::data::placementOrientation::up);

		EXPECT_EQ(untouched.prototype_data, &drill);
	}

	TEST(mouse_selection_overlay, draw_overlay_call_get_sprite) {
		class Mock_entity final : public jactorio::data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			mutable bool r_get_sprite_called = false;

			J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(jactorio::data::placementOrientation orientation,
			                                                                    jactorio::game::World_data& world_data,
			                                                                    std::pair<int, int> world_coords) const override {
				return {16, 17};
			}

			void on_r_show_gui(jactorio::game::Player_data& player_data,
			                   jactorio::game::Chunk_tile_layer* tile_layer) const override {
			}


			void on_build(jactorio::game::World_data& world_data,
			              std::pair<jactorio::game::World_data::world_coord, jactorio::game::World_data::world_coord>
			              world_coords,
			              jactorio::game::Chunk_tile_layer& tile_layer, uint16_t frame,
			              jactorio::data::placementOrientation orientation) const override {
			}

			void on_remove(jactorio::game::World_data& world_data, std::pair<int, int> world_coords,
			               jactorio::game::Chunk_tile_layer& tile_layer) const override {
			}

			// Overriden to give nullptr
			jactorio::data::Sprite* on_r_get_sprite(jactorio::data::Unique_data_base* unique_data) const override {
				r_get_sprite_called = true;
				return nullptr;
			}
		};

		MOUSE_SELECTION_TEST_HEADER

		Mock_entity entity{};
		entity.rotatable = true;
		entity.placeable = true;

		mouse_selection.draw_overlay(player_data, &entity, 0, 0, jactorio::data::placementOrientation::up);

		// On_r_get_sprite should have been called 
		EXPECT_TRUE(entity.r_get_sprite_called);
	}
}
