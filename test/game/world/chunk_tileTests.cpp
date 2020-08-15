// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/chunk_tile.h"

#include <memory>

#include "data/prototype/resource_entity.h"
#include "data/prototype/tile.h"

namespace jactorio::game
{
	TEST(ChunkTile, LayerCopy) {
		// Copying a chunk tile needs to also make a unique copy of unique_data_
		const auto entity_proto = std::make_unique<data::ResourceEntity>(data::ResourceEntity());

		auto tile_layer          = ChunkTileLayer();
		tile_layer.prototypeData = entity_proto.get();

		tile_layer.MakeUniqueData<data::ResourceEntityData>(10);

		// Copy layer
		const auto tile_layer_copy = tile_layer;

		// Data should have been allocated differently
		EXPECT_NE(tile_layer_copy.GetUniqueData(), tile_layer.GetUniqueData());
		EXPECT_NE(tile_layer_copy.GetUniqueData(), nullptr);  // Data should have been copied

		// Heap allocated data cleaned up by chunk_tile_layer destructors
	}


	TEST(ChunkTile, LayerMove) {
		// Moving unique_data will set the original unique_data to nullptr to avoid deletion
		const auto entity_proto = std::make_unique<data::ResourceEntity>(data::ResourceEntity());

		auto tile_layer          = ChunkTileLayer();
		tile_layer.prototypeData = entity_proto.get();  // Prototype data needed to delete unique data

		auto* u_data = tile_layer.MakeUniqueData<data::ResourceEntityData>(10);

		// MOVE layer
		const auto tile_layer_new = std::move(tile_layer);

		EXPECT_EQ(tile_layer_new.GetUniqueData(), u_data);
		EXPECT_EQ(tile_layer.GetUniqueData(), nullptr);  // Moved into tile_layer_new, this becomes nullptr

		// Heap allocated data cleaned up by chunk_tile_layer destructors
	}


	TEST(ChunkTile, TilePrototypesInitialization) {
		auto ct = ChunkTile();

		// Should all be nullptr
		for (auto layer : ct.layers) {
			EXPECT_EQ(layer.prototypeData, nullptr);
		}
	}

	TEST(ChunkTile, GetSetChunkLayerProps) {
		auto ct = ChunkTile();

		auto tile_proto = data::Tile();

		ct.SetTilePrototype(&tile_proto);

		EXPECT_EQ(
			ct.GetTilePrototype(),
			&tile_proto);
	}
}
