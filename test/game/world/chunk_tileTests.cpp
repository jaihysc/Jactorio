#include <gtest/gtest.h>

#include "game/world/chunk_tile.h"

#include <memory>

namespace game
{
	TEST(chunk_tile, chunk_tile_layer_init) {
		// Init without tile
		const jactorio::game::Chunk_tile_layer tile_layer;
		// Should initialize to nullptr by default
		EXPECT_EQ(tile_layer.sprite, nullptr);
		EXPECT_EQ(tile_layer.get_tile_prototype(), nullptr);
	}
	
	TEST(chunk_tile, chunk_tile_layer_init_tile) {
		// Init with tile in constructor
		// Sprite prototype
		const auto sprite_proto = std::make_unique<jactorio::data::Sprite>(jactorio::data::Sprite());
		const auto tile_proto = std::make_unique<jactorio::data::Tile>(jactorio::data::Tile());

		tile_proto->sprite_ptr = sprite_proto.get();


		// Using the constructor with a Tile prototype provided will:
		// The tile's sprite pointer will be given to Chunk_tile_layer,
		// Remember the tile ptr
		const auto tile_layer = jactorio::game::Chunk_tile_layer(tile_proto.get());
		EXPECT_EQ(tile_layer.sprite, sprite_proto.get());
		EXPECT_EQ(tile_layer.get_tile_prototype(), tile_proto.get());
	}

	TEST(chunk_tile, chunk_tile_layer_init_tile_setter) {
		// Init with tile setter

		// Set tile prototype should set sprite prototype
		const auto sprite_proto = new jactorio::data::Sprite();
		const auto tile_proto = new jactorio::data::Tile();
		tile_proto->sprite_ptr = sprite_proto;

		// Pointers not nullptr
		{
			// Using the setter for Tile* will also set the sprite* in Chunk_tile_layer to the one from Tile*
			auto tile_layer = jactorio::game::Chunk_tile_layer();
			tile_layer.set_tile_prototype(tile_proto);

			EXPECT_EQ(tile_layer.sprite, sprite_proto);
			EXPECT_EQ(tile_layer.get_tile_prototype(), tile_proto);
		}
		// Nullptr
		{
			// Should NOT set sprite to nullptr
			auto tile_layer = jactorio::game::Chunk_tile_layer();
			EXPECT_EQ(tile_layer.sprite, nullptr);  // Currently nullptr after constructing
			tile_layer.sprite = sprite_proto;  // Is now a sprite pointer

			tile_layer.set_tile_prototype(nullptr);

			EXPECT_EQ(tile_layer.sprite, sprite_proto);
			EXPECT_EQ(tile_layer.get_tile_prototype(), nullptr);
		}
	}
	
	
	TEST(chunk_tile, tile_prototypes_initialization) {
		auto ct = jactorio::game::Chunk_tile();
		
		// Should all be nullptr
		EXPECT_EQ(ct.entity, nullptr);
		EXPECT_EQ(ct.entity_index, 0);  // Defaults to 0, which is used for single tiles
		
		for (int i = 0; i < jactorio::game::Chunk_tile::layers_count; ++i) {
			EXPECT_EQ(ct.layers[i].sprite, nullptr);
			EXPECT_EQ(ct.layers[i].get_tile_prototype(), nullptr);
		}
	}

	TEST(chunk_tile, get_set_chunk_layer_props) {
		auto ct = jactorio::game::Chunk_tile();

		{
			auto tile_proto = jactorio::data::Tile();

			ct.set_tile_layer_tile_prototype(jactorio::game::Chunk_tile::chunk_layer::base, &tile_proto);

			EXPECT_EQ(
				ct.get_tile_layer_tile_prototype(jactorio::game::Chunk_tile::chunk_layer::base),
				&tile_proto);
		}
		{
			auto sprite_proto = jactorio::data::Sprite();

			ct.set_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::base, &sprite_proto);

			EXPECT_EQ(
				ct.get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::base),
				&sprite_proto);
		}
	}
}