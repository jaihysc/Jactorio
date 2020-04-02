// 
// chunk_tileTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/21/2019
// Last modified: 04/02/2020
// 

#include <gtest/gtest.h>

#include "game/world/chunk_tile.h"

#include <memory>

#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/tile/tile.h"

namespace game
{
	TEST(chunk_tile, layer_copy) {
		// Copying a chunk tile needs to also make a unique copy of unique_data_
		const auto entity_proto = std::make_unique<jactorio::data::Resource_entity>(jactorio::data::Resource_entity());

		auto* u_data1 = new jactorio::data::Resource_entity_data();

		auto tile_layer = jactorio::game::Chunk_tile_layer();
		tile_layer.prototype_data = entity_proto.get();
		tile_layer.unique_data = u_data1;

		// Copy layer
		const auto tile_layer_copy = tile_layer;

		// Data should have been allocated differently
		EXPECT_NE(tile_layer_copy.unique_data, tile_layer.unique_data);
		EXPECT_NE(tile_layer_copy.unique_data, nullptr);  // Data should have been copied

		// Heap allocated data cleaned up by chunk_tile_layer destructors
	}


	TEST(chunk_tile, layer_move) {
		// Moving unique_data will set the original unique_data to nullptr to avoid deletion
		auto* u_data = new jactorio::data::Resource_entity_data();
		const auto entity_proto = std::make_unique<jactorio::data::Resource_entity>(jactorio::data::Resource_entity());

		auto tile_layer = jactorio::game::Chunk_tile_layer();
		tile_layer.prototype_data = entity_proto.get();  // Prototype data needed to delete unique data
		tile_layer.unique_data = u_data;

		// MOVE layer
		const auto tile_layer_new = std::move(tile_layer);

		EXPECT_EQ(tile_layer_new.unique_data, u_data);
		EXPECT_EQ(tile_layer.unique_data, nullptr);  // Moved into tile_layer_new, this becomes nullptr

		// Heap allocated data cleaned up by chunk_tile_layer destructors
	}


	TEST(chunk_tile, tile_prototypes_initialization) {
		auto ct = jactorio::game::Chunk_tile();

		// Should all be nullptr
		for (auto layer : ct.layers) {
			EXPECT_EQ(layer.prototype_data, nullptr);
		}
	}

	TEST(chunk_tile, get_set_chunk_layer_props) {
		auto ct = jactorio::game::Chunk_tile();

		{
			auto tile_proto = jactorio::data::Tile();

			ct.set_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base, &tile_proto);

			EXPECT_EQ(
				ct.get_tile_prototype(jactorio::game::Chunk_tile::chunkLayer::base),
				&tile_proto);
		}
		{
			auto sprite_proto = jactorio::data::Sprite();

			ct.set_sprite_prototype(jactorio::game::Chunk_tile::chunkLayer::overlay, &sprite_proto);

			EXPECT_EQ(
				ct.get_sprite_prototype(jactorio::game::Chunk_tile::chunkLayer::overlay),
				&sprite_proto);
		}
	}
}
