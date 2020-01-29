#include <gtest/gtest.h>

#include "game/world/chunk.h"
#include <memory>

namespace game
{
	TEST(world_chunk, chunk_set_tile) {
		// The tiles pointer is only stored by the Chunk, modifying the original tiles pointer
		// will modify the tiles of the chunk

		// Mock data
		const auto sprite_proto = std::make_unique<jactorio::data::Sprite>(jactorio::data::Sprite());
		const auto tile_proto = std::make_unique<jactorio::data::Tile>(jactorio::data::Tile());
		tile_proto->sprite_ptr = sprite_proto.get();


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
		EXPECT_EQ(chunk.tiles_ptr()[0].layers[0].get_tile_prototype(), tile_layer_1.get_tile_prototype());

		tiles[0] = chunk_tile_2;
		EXPECT_EQ(chunk.tiles_ptr()[0].layers[0].get_tile_prototype(), tile_layer_2.get_tile_prototype());


		// Prototype data in the actual application is managed by data_manager
	}
}
