#include <gtest/gtest.h>

#include "game/world/chunk_tile.h"

#include <memory>

namespace game
{
	TEST(chunk_tile, chunk_tile_layer_init) {
		// Init without tile
		const jactorio::game::Chunk_tile_layer tile_layer;

		// Should initialize to nullptr by default
		EXPECT_EQ(tile_layer.get_tile_prototype(), nullptr);
		EXPECT_EQ(tile_layer.get_entity_prototype(), nullptr);
		EXPECT_EQ(tile_layer.get_sprite_prototype(), nullptr);
	}
	
	TEST(chunk_tile, chunk_tile_layer_init_tile) {
		// Init with tile in constructor
		const auto tile_proto = std::make_unique<jactorio::data::Tile>(jactorio::data::Tile());

		const auto tile_layer = jactorio::game::Chunk_tile_layer(tile_proto.get());
		EXPECT_EQ(tile_layer.get_tile_prototype(), tile_proto.get());
	}

	TEST(chunk_tile, chunk_tile_layer_init_tile_setter) {
		// Init with tile setter
		const auto tile_proto = std::make_unique<jactorio::data::Tile>(jactorio::data::Tile());

		// Pointers not nullptr
		{
			auto tile_layer = jactorio::game::Chunk_tile_layer();
			
			tile_layer.set_data(tile_proto.get());

			// The getter only converts the types
			EXPECT_EQ(tile_layer.get_tile_prototype(), tile_proto.get());
			EXPECT_EQ(tile_layer.get_entity_prototype(), reinterpret_cast<jactorio::data::Entity*>(tile_proto.get()));
			EXPECT_EQ(tile_layer.get_sprite_prototype(), reinterpret_cast<jactorio::data::Sprite*>(tile_proto.get()));
		}
		// Nullptr
		{
			auto tile_layer = jactorio::game::Chunk_tile_layer();

			tile_layer.set_data(nullptr);
			EXPECT_EQ(tile_layer.get_tile_prototype(), nullptr);
			EXPECT_EQ(tile_layer.get_entity_prototype(), nullptr);
			EXPECT_EQ(tile_layer.get_sprite_prototype(), nullptr);
		}
	}
	
	
	TEST(chunk_tile, tile_prototypes_initialization) {
		auto ct = jactorio::game::Chunk_tile();
		
		// Should all be nullptr
		for (auto layer : ct.layers) {
			EXPECT_EQ(layer.get_tile_prototype(), nullptr);
		}
	}

	TEST(chunk_tile, get_set_chunk_layer_props) {
		auto ct = jactorio::game::Chunk_tile();

		{
			auto tile_proto = jactorio::data::Tile();

			ct.set_layer_tile_prototype(jactorio::game::Chunk_tile::chunk_layer::base, &tile_proto);

			EXPECT_EQ(
				ct.get_layer_tile_prototype(jactorio::game::Chunk_tile::chunk_layer::base),
				&tile_proto);
		}
		{
			auto sprite_proto = jactorio::data::Sprite();

			ct.set_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::overlay, &sprite_proto);

			EXPECT_EQ(
				ct.get_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::overlay),
				&sprite_proto);
		}
	}
}