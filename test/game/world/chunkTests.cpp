#include <gtest/gtest.h>

#include "game/world/chunk.h"

namespace game
{
	TEST(world_chunk, chunk_set_tile) {
		// The tiles pointer stored by the Chunk, modifying the original tiles pointer
		// will modify the tiles of the chunk

		auto chunk_tile_1 = jactorio::game::Chunk_tile();
		auto tile1 = new jactorio::data::Tile();
		chunk_tile_1.tile_prototypes.push_back(tile1);

		auto chunk_tile_2 = jactorio::game::Chunk_tile();
		auto tile2 = new jactorio::data::Tile();
		chunk_tile_2.tile_prototypes.push_back(tile2);


		auto tiles = new jactorio::game::Chunk_tile[32 * 32];
		tiles[0] = chunk_tile_1;
		jactorio::game::Chunk chunk{ 0, 0, tiles };

		EXPECT_EQ(chunk.tiles_ptr()[0].tile_prototypes[0], tile1);

		tiles[0] = chunk_tile_2;
		EXPECT_EQ(chunk.tiles_ptr()[0].tile_prototypes[0], tile2);
	}
}
