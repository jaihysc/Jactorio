#include "game/world/chunk_tile.h"

unsigned short jactorio::game::Chunk_tile::get_layer_index(chunk_layer category) {
	return static_cast<unsigned short>(category);
}

// Tile
jactorio::data::Tile* jactorio::game::Chunk_tile::get_layer_tile_prototype(const chunk_layer category) const {
	assert(category == chunk_layer::base);
	return static_cast<data::Tile*>(layers[get_layer_index(category)].prototype_data);
}

void jactorio::game::Chunk_tile::set_layer_tile_prototype(const chunk_layer category, 
                                                          data::Tile* tile_prototype) const {
	assert(category == chunk_layer::base);
	layers[get_layer_index(category)].prototype_data = tile_prototype;
}


// Entity
jactorio::data::Entity* jactorio::game::Chunk_tile::get_layer_entity_prototype(const chunk_layer category) const {
	assert(category == chunk_layer::resource || category == chunk_layer::entity);
	return static_cast<data::Entity*>(layers[get_layer_index(category)].prototype_data);
}


void jactorio::game::Chunk_tile::set_layer_entity_prototype(const chunk_layer category, 
                                                            data::Entity* tile_prototype) const {
	assert(category == chunk_layer::resource || category == chunk_layer::entity);
	layers[get_layer_index(category)].prototype_data = tile_prototype;
}


// Sprite
jactorio::data::Sprite* jactorio::game::Chunk_tile::get_layer_sprite_prototype(const chunk_layer category) const {
	assert(category == chunk_layer::overlay);
	return static_cast<data::Sprite*>(layers[get_layer_index(category)].prototype_data);
}

void jactorio::game::Chunk_tile::set_layer_sprite_prototype(const chunk_layer category, 
                                                            data::Sprite* tile_prototype) const {
	assert(category == chunk_layer::overlay);
	layers[get_layer_index(category)].prototype_data = tile_prototype;
}
