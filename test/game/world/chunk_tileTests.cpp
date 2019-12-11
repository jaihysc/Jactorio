#include <gtest/gtest.h>

#include "game/world/chunk_tile.h"

namespace game
{
	TEST(chunk_tile, tile_prototypes_initialization) {
		auto ct = jactorio::game::Chunk_tile();
		
		// Should all be nullptr
		for (int i = 0; i < jactorio::game::Chunk_tile::tile_prototypes_count; ++i) {
			EXPECT_EQ(ct.tile_prototypes[i], nullptr);
		}
	}

	TEST(chunk_tile, get_set_prototype) {
		auto ct = jactorio::game::Chunk_tile();

		auto tile_proto = jactorio::data::Tile();

		ct.set_tile_prototype(jactorio::game::Chunk_tile::prototype_category::base, &tile_proto);
		
		EXPECT_EQ(
			ct.get_tile_prototype(jactorio::game::Chunk_tile::prototype_category::base),
			&tile_proto);
	}
}