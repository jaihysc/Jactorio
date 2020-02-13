#include <gtest/gtest.h>

#include "game/world/chunk.h"
#include <memory>

namespace game
{
	TEST(chunk, chunk_set_tile) {
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
		EXPECT_EQ(chunk.tiles_ptr()[0].layers[0].prototype_data, tile_layer_1.prototype_data);

		tiles[0] = chunk_tile_2;
		EXPECT_EQ(chunk.tiles_ptr()[0].layers[0].prototype_data, tile_layer_2.prototype_data);


		// Prototype data in the actual application is managed by data_manager
	}

	TEST(chunk, chunk_copy) {
		auto* tiles = new jactorio::game::Chunk_tile[32 * 32];
		const jactorio::game::Chunk chunk_a{0, 0, tiles};

		const auto chunk_copy = chunk_a;
		// Should not copy the pointer for tiles
		EXPECT_NE(chunk_copy.tiles_ptr(), chunk_a.tiles_ptr());
	}

	TEST(chunk, get_object_layer) {
		jactorio::game::Chunk chunk_a{0, 0, nullptr};

		// Should return the layer specified by the index of the enum object_layer
		EXPECT_EQ(&chunk_a.get_object(jactorio::game::Chunk::object_layer::item_entity), &chunk_a.objects[0]);
	}
}
