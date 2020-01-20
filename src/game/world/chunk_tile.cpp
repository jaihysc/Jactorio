#include "game/world/chunk_tile.h"

// ====================================
// Chunk_tile_layer

void jactorio::game::Chunk_tile_layer::set_tile_prototype(data::Tile* const tile_prototype) {
	tile_prototype_ = tile_prototype;

	if (tile_prototype != nullptr)
		sprite = tile_prototype->sprite_ptr;
}

// ====================================
// Chunk_tile

unsigned short jactorio::game::Chunk_tile::get_layer_index(chunk_layer category) {
	return static_cast<unsigned short>(category);
}

// Tile prototype
jactorio::data::Tile* jactorio::game::Chunk_tile::get_tile_layer_tile_prototype(const chunk_layer category) const {
	if (entity == nullptr) {
		// Sprite should not exist in the entity layer without an entity prototype set
		assert(layers[get_layer_index(chunk_layer::entity)].sprite == nullptr);
	}
	
	return layers[get_layer_index(category)].get_tile_prototype();
}

void jactorio::game::Chunk_tile::set_tile_layer_tile_prototype(const chunk_layer category,
                                                               data::Tile* tile_prototype) {
	layers[get_layer_index(category)].set_tile_prototype(tile_prototype);
}

// Sprite prototype
jactorio::data::Sprite* jactorio::game::Chunk_tile::get_tile_layer_sprite_prototype(const chunk_layer category) const {
	if (entity == nullptr) {
		// Sprite should not exist in the entity layer without an entity prototype set
		assert(layers[get_layer_index(chunk_layer::entity)].sprite == nullptr);
	}
	
	return layers[get_layer_index(category)].sprite;
}

void jactorio::game::Chunk_tile::set_tile_layer_sprite_prototype(const chunk_layer category,
                                                                 data::Sprite* sprite_prototype) {
	layers[get_layer_index(category)].sprite = sprite_prototype;
}
