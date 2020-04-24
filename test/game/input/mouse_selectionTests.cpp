// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/05/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "game/input/mouse_selection.h"

namespace game
{
	class MouseSelectionOverlayTest : public testing::Test
	{
		bool setup_cursor_ = false;

	protected:
		jactorio::game::World_data world_data_{};
		jactorio::game::Player_data player_data_{};
		jactorio::game::Mouse_selection mouse_selection_{};

		// Call setup_mouse_cursor before using
		jactorio::data::Sprite* cursor_sprite_ = nullptr;


		void SetUp() override {
			player_data_.set_player_world(&world_data_);
			world_data_.add_chunk(new jactorio::game::Chunk(0, 0));
		}

		void TearDown() override {
			if (setup_cursor_) {
				jactorio::data::clear_data();
			}
		}

		///
		/// \brief If the test involves the selection cursor, which is loaded in the data_manager
		void setup_mouse_cursor() {
			setup_cursor_  = true;
			cursor_sprite_ = new jactorio::data::Sprite{};
			data_raw_add("__core__/cursor-select", cursor_sprite_);
		}
	};

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedPlaceable) {
		// An item is selected
		// Clear last and draw new
		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = true;

		// Should set item's sprite at overlay layer at world position 0, 0
		mouse_selection_.draw_overlay(player_data_, &entity, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data,
			&entity_sprite
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouse_selection_.draw_overlay(player_data_, &entity, 1, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data,
			nullptr
		);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(1, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data,
			&entity_sprite
		);

	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceable) {
		// A non placeable item is selected
		// Clear last and draw nothing new

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouse_selection_.draw_overlay(player_data_, &entity, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data,
			nullptr);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceableHoverEntity) {
		// A non placeable item is selected over an entity
		// Clear last and draw cursor

		setup_mouse_cursor();

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;

		world_data_.get_tile_world_coords(0, 0)
		           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &entity;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouse_selection_.draw_overlay(player_data_, &entity, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data,
			cursor_sprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelected) {
		// NO item is selected
		// CLear last and draw nothing new

		// Should set item's sprite at overlay layer at world position 0, 0
		mouse_selection_.draw_overlay(player_data_, nullptr, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data,
			nullptr
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouse_selection_.draw_overlay(player_data_, nullptr, 1, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data,
			nullptr
		);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(1, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
			prototype_data,
			nullptr
		);

	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedEntityHover) {
		// NO item is selected, but cursor is hovered over an entity
		// Draw selection cursor

		setup_mouse_cursor();

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite = &entity_sprite;


		world_data_.get_tile_world_coords(0, 0)
		           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &entity;


		mouse_selection_.draw_overlay(player_data_, nullptr, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data,
			cursor_sprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedResourceEntityHover) {
		// NO item is selected, but cursor is hovered over a resource entity
		// Draw selection cursor

		setup_mouse_cursor();

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::Container_entity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;


		world_data_.get_tile_world_coords(0, 0)
		           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &entity;


		mouse_selection_.draw_overlay(player_data_, nullptr, 0, 0, jactorio::data::placementOrientation::up);
		EXPECT_EQ(
			world_data_.get_tile_world_coords(0, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay)
			.prototype_data,
			cursor_sprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNoWorld) {
		// When selecting no world / chunk not generated
		// Attempt to clear last tile and do nothing

		jactorio::game::World_data world_data{};
		jactorio::game::Player_data player_data{};
		player_data.set_player_world(&world_data);

		jactorio::game::Mouse_selection mouse_selection{};

		mouse_selection.draw_overlay(player_data, nullptr, 0, 0, jactorio::data::placementOrientation::up);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlay3x3) {

		setup_mouse_cursor();

		jactorio::data::Mining_drill drill{};
		drill.tile_width  = 3;
		drill.tile_height = 3;

		jactorio::data::Sprite drill_sprite{};
		drill.sprite = &drill_sprite;

		mouse_selection_.draw_overlay(player_data_, &drill, 0, 0, jactorio::data::placementOrientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const jactorio::data::Prototype_base* data =
					world_data_.get_tile_world_coords(x, y)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
					            prototype_data;
				EXPECT_EQ(data, &drill_sprite);
			}
		}


		// Removing the drawn overlay, no resource or entity data so no cursor is drawn
		mouse_selection_.draw_overlay(player_data_, nullptr, 0, 0, jactorio::data::placementOrientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const jactorio::data::Prototype_base* data =
					world_data_.get_tile_world_coords(x, y)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).
					            prototype_data;
				EXPECT_EQ(data, nullptr);
			}
		}


		// Drawing a cursor should be 1 x 1
		world_data_.get_tile_world_coords(0, 0) // Create an entity
		           ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).prototype_data = &drill;

		// Should not touch this
		jactorio::game::Chunk_tile_layer& untouched =
			world_data_.get_tile_world_coords(1, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay);
		untouched.prototype_data = &drill;


		mouse_selection_.draw_overlay(player_data_,
		                              nullptr,
		                              0,
		                              0,
		                              jactorio::data::placementOrientation::up);

		EXPECT_EQ(untouched.prototype_data, &drill);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayCallGetSprite) {
		class Mock_entity final : public jactorio::data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			mutable bool r_get_sprite_called = false;

			J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(jactorio::data::placementOrientation,
			                                                                    jactorio::game::World_data&,
			                                                                    std::pair<int, int>) const override {
				return {16, 17};
			}

			void on_r_show_gui(jactorio::game::Player_data&,
			                   jactorio::game::Chunk_tile_layer*) const override {
			}


			void on_build(jactorio::game::World_data&,
			              std::pair<jactorio::game::World_data::world_coord, jactorio::game::World_data::world_coord>,
			              jactorio::game::Chunk_tile_layer&,
			              uint16_t,
			              jactorio::data::placementOrientation) const override {
			}

			void on_remove(jactorio::game::World_data&,
			               std::pair<int, int>,
			               jactorio::game::Chunk_tile_layer&) const override {
			}

			// Overriden to give nullptr
			jactorio::data::Sprite* on_r_get_sprite(jactorio::data::Unique_data_base* /*unique_data*/) const override {
				r_get_sprite_called = true;
				return nullptr;
			}
		};

		Mock_entity entity{};
		entity.rotatable = true;
		entity.placeable = true;

		mouse_selection_.draw_overlay(player_data_, &entity, 0, 0, jactorio::data::placementOrientation::up);

		// On_r_get_sprite should have been called 
		EXPECT_TRUE(entity.r_get_sprite_called);
	}
}
