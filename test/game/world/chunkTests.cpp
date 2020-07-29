// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"
#include "data/prototype/tile/tile.h"
#include "game/world/chunk.h"
#include "game/world/world_data.h"

#include <memory>

namespace jactorio::game
{
	TEST(Chunk, ChunkSetTile) {
		constexpr auto chunk_width = WorldData::kChunkWidth;
		// The tiles pointer is only stored by the Chunk, modifying the original tiles pointer
		// will modify the tiles of the chunk

		// Mock data
		const auto sprite_proto = std::make_unique<data::Sprite>(data::Sprite());
		const auto tile_proto   = std::make_unique<data::Tile>(data::Tile());
		tile_proto->sprite      = sprite_proto.get();


		// Chunk tile 1
		auto chunk_tile_1 = ChunkTile();

		const auto tile_layer_1 = ChunkTileLayer(tile_proto.get());
		chunk_tile_1.layers[0]  = tile_layer_1;


		// Chunk tile 2
		auto chunk_tile_2 = ChunkTile();

		const ChunkTileLayer tile_layer_2{};
		chunk_tile_2.layers[0] = tile_layer_2;


		// Deleted by chunk
		auto* tiles = new ChunkTile[chunk_width * chunk_width];
		tiles[0]    = chunk_tile_1;

		const Chunk chunk{0, 0, tiles};
		EXPECT_EQ(chunk.Tiles()[0].layers[0].prototypeData, tile_layer_1.prototypeData);

		tiles[0] = chunk_tile_2;
		EXPECT_EQ(chunk.Tiles()[0].layers[0].prototypeData, tile_layer_2.prototypeData);


		// Prototype data in the actual application is managed by data_manager
	}

	TEST(Chunk, ChunkCopy) {
		const Chunk chunk_a{0, 0};

		const auto chunk_copy = chunk_a;
		// Should not copy the pointer for tiles
		EXPECT_NE(chunk_copy.Tiles(), chunk_a.Tiles());
	}

	TEST(Chunk, ChunkMove) {
		Chunk chunk_a{0, 0};

		const auto chunk_move = std::move(chunk_a);
		EXPECT_EQ(chunk_a.Tiles(), nullptr);
	}

	TEST(Chunk, GetObjectLayer) {
		Chunk chunk_a{0, 0};

		// Should return the layer specified by the index of the enum objectLayer
		EXPECT_EQ(&chunk_a.GetOverlay(OverlayLayer::general), &chunk_a.overlays[0]);
	}

	TEST(Chunk, GetLogicGroup) {
		Chunk chunk{0, 0};

		// Should return the layer specified by the index of the enum objectLayer
		EXPECT_EQ(&chunk.GetLogicGroup(Chunk::LogicGroup::transport_line), &chunk.logicGroups[0]);
	}
}
