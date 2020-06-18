// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/21/2019

#include <gtest/gtest.h>

#include "game/world/chunk_tile.h"

#include <memory>

#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/tile/tile.h"

namespace game
{
	TEST(ChunkTile, LayerCopy) {
		// Copying a chunk tile needs to also make a unique copy of unique_data_
		const auto entity_proto = std::make_unique<jactorio::data::ResourceEntity>(jactorio::data::ResourceEntity());

		auto tile_layer          = jactorio::game::ChunkTileLayer();
		tile_layer.prototypeData = entity_proto.get();

		tile_layer.MakeUniqueData<jactorio::data::ResourceEntityData>(10);

		// Copy layer
		const auto tile_layer_copy = tile_layer;

		// Data should have been allocated differently
		EXPECT_NE(tile_layer_copy.GetUniqueData(), tile_layer.GetUniqueData());
		EXPECT_NE(tile_layer_copy.GetUniqueData(), nullptr);  // Data should have been copied

		// Heap allocated data cleaned up by chunk_tile_layer destructors
	}


	TEST(ChunkTile, LayerMove) {
		// Moving unique_data will set the original unique_data to nullptr to avoid deletion
		const auto entity_proto = std::make_unique<jactorio::data::ResourceEntity>(jactorio::data::ResourceEntity());

		auto tile_layer          = jactorio::game::ChunkTileLayer();
		tile_layer.prototypeData = entity_proto.get();  // Prototype data needed to delete unique data

		auto* u_data = tile_layer.MakeUniqueData<jactorio::data::ResourceEntityData>(10);

		// MOVE layer
		const auto tile_layer_new = std::move(tile_layer);

		EXPECT_EQ(tile_layer_new.GetUniqueData(), u_data);
		EXPECT_EQ(tile_layer.GetUniqueData(), nullptr);  // Moved into tile_layer_new, this becomes nullptr

		// Heap allocated data cleaned up by chunk_tile_layer destructors
	}


	TEST(ChunkTile, TilePrototypesInitialization) {
		auto ct = jactorio::game::ChunkTile();

		// Should all be nullptr
		for (auto layer : ct.layers) {
			EXPECT_EQ(layer.prototypeData, nullptr);
		}
	}

	TEST(ChunkTile, GetSetChunkLayerProps) {
		auto ct = jactorio::game::ChunkTile();

		{
			auto tile_proto = jactorio::data::Tile();

			ct.SetTilePrototype(jactorio::game::ChunkTile::ChunkLayer::base, &tile_proto);

			EXPECT_EQ(
				ct.GetTilePrototype(jactorio::game::ChunkTile::ChunkLayer::base),
				&tile_proto);
		}
		{
			auto sprite_proto = jactorio::data::Sprite();

			ct.SetSpritePrototype(jactorio::game::ChunkTile::ChunkLayer::overlay, &sprite_proto);

			EXPECT_EQ(
				ct.GetSpritePrototype(jactorio::game::ChunkTile::ChunkLayer::overlay),
				&sprite_proto);
		}
	}
}
