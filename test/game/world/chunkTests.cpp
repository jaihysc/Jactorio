// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/09/2019

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"
#include "data/prototype/tile/tile.h"
#include "game/world/chunk.h"
#include "game/world/logic_chunk.h"

#include <memory>

namespace game
{
	TEST(chunk, chunk_set_tile) {
		// The tiles pointer is only stored by the Chunk, modifying the original tiles pointer
		// will modify the tiles of the chunk

		// Mock data
		const auto sprite_proto = std::make_unique<jactorio::data::Sprite>(jactorio::data::Sprite());
		const auto tile_proto = std::make_unique<jactorio::data::Tile>(jactorio::data::Tile());
		tile_proto->sprite = sprite_proto.get();


		// Chunk tile 1
		auto chunk_tile_1 = jactorio::game::Chunk_tile();

		const auto tile_layer_1 = jactorio::game::Chunk_tile_layer(tile_proto.get());
		chunk_tile_1.layers[0] = tile_layer_1;


		// Chunk tile 2
		auto chunk_tile_2 = jactorio::game::Chunk_tile();

		const jactorio::game::Chunk_tile_layer tile_layer_2{};
		chunk_tile_2.layers[0] = tile_layer_2;


		const auto tiles = new jactorio::game::Chunk_tile[32 * 32];

		tiles[0] = chunk_tile_1;

		const jactorio::game::Chunk chunk{0, 0, tiles};
		EXPECT_EQ(chunk.tiles_ptr()[0].layers[0].prototype_data, tile_layer_1.prototype_data);

		tiles[0] = chunk_tile_2;
		EXPECT_EQ(chunk.tiles_ptr()[0].layers[0].prototype_data, tile_layer_2.prototype_data);


		// Prototype data in the actual application is managed by data_manager
	}

	TEST(chunk, chunk_copy) {
		const jactorio::game::Chunk chunk_a{0, 0};

		const auto chunk_copy = chunk_a;
		// Should not copy the pointer for tiles
		EXPECT_NE(chunk_copy.tiles_ptr(), chunk_a.tiles_ptr());
	}

	TEST(chunk, chunk_move) {
		jactorio::game::Chunk chunk_a{0, 0};

		const auto chunk_move = std::move(chunk_a);
		EXPECT_EQ(chunk_a.tiles_ptr(), nullptr);
	}

	TEST(chunk, get_object_layer) {
		jactorio::game::Chunk chunk_a{0, 0};

		// Should return the layer specified by the index of the enum objectLayer
		EXPECT_EQ(&chunk_a.get_object(jactorio::game::Chunk::objectLayer::tree), &chunk_a.objects[0]);
	}

	TEST(logic_chunk, get_struct_layer) {
		jactorio::game::Chunk chunk_a{0, 0};
		jactorio::game::Logic_chunk l_chunk(&chunk_a);

		// Should return the layer specified by the index of the enum objectLayer
		EXPECT_EQ(&l_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line), &l_chunk.structs[0]);
	}
}
