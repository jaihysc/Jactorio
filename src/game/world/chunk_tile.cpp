#include "game/world/chunk_tile.h"

jactorio::game::Chunk_tile_layer::Chunk_tile_layer(const Chunk_tile_layer& other)
	: prototype_data_(other.prototype_data_),
	  multi_tile_index(other.multi_tile_index),
	  multi_tile_span(other.multi_tile_span),
	  multi_tile_height(other.multi_tile_height) {
	// Use prototype defined method for copying unique_data_
	if (unique_data != nullptr) {
		assert(prototype_data_ != nullptr);  // No prototype_data_ available for copying unique_data_
		unique_data = prototype_data_->copy_unique_data(other.unique_data);
	}
}

jactorio::game::Chunk_tile_layer::Chunk_tile_layer(Chunk_tile_layer&& other) noexcept
	: prototype_data_(other.prototype_data_),
	  unique_data(other.unique_data),
	  multi_tile_index(other.multi_tile_index),
	  multi_tile_span(other.multi_tile_span),
	  multi_tile_height(other.multi_tile_height) {
}

jactorio::game::Chunk_tile_layer& jactorio::game::Chunk_tile_layer::operator=(const Chunk_tile_layer& other) {
	if (this == &other)
		return *this;
	prototype_data_ = other.prototype_data_;
	multi_tile_index = other.multi_tile_index;
	multi_tile_span = other.multi_tile_span;
	multi_tile_height = other.multi_tile_height;

	// Use prototype defined method for copying unique_data_
	if (unique_data != nullptr) {
		assert(prototype_data_ != nullptr);  // No prototype_data_ available for copying unique_data_
		unique_data = prototype_data_->copy_unique_data(other.unique_data);
	}
	return *this;
}

jactorio::game::Chunk_tile_layer& jactorio::game::Chunk_tile_layer::operator=(Chunk_tile_layer&& other) noexcept {
	if (this == &other)
		return *this;
	prototype_data_ = other.prototype_data_;
	unique_data = other.unique_data;
	multi_tile_index = other.multi_tile_index;
	multi_tile_span = other.multi_tile_span;
	multi_tile_height = other.multi_tile_height;
	return *this;
}


// ====================================
// Chunk_tile

unsigned short jactorio::game::Chunk_tile::get_layer_index(chunk_layer category) {
	return static_cast<unsigned short>(category);
}

// Tile
jactorio::data::Tile* jactorio::game::Chunk_tile::get_layer_tile_prototype(const chunk_layer category) const {
	assert(category == chunk_layer::base);
	return layers[get_layer_index(category)].get_tile_prototype();
}

void jactorio::game::Chunk_tile::set_layer_tile_prototype(const chunk_layer category, 
                                                          data::Tile* tile_prototype) {
	assert(category == chunk_layer::base);
	layers[get_layer_index(category)].set_data(tile_prototype);
}


// Entity
jactorio::data::Entity* jactorio::game::Chunk_tile::get_layer_entity_prototype(const chunk_layer category) const {
	assert(category == chunk_layer::resource || category == chunk_layer::entity);
	return layers[get_layer_index(category)].get_entity_prototype();
}


void jactorio::game::Chunk_tile::set_layer_entity_prototype(const chunk_layer category, 
                                                            data::Entity* tile_prototype) {
	assert(category == chunk_layer::resource || category == chunk_layer::entity);
	layers[get_layer_index(category)].set_data(tile_prototype);
}


// Sprite
jactorio::data::Sprite* jactorio::game::Chunk_tile::get_layer_sprite_prototype(const chunk_layer category) const {
	assert(category == chunk_layer::overlay);
	return layers[get_layer_index(category)].get_sprite_prototype();
}

void jactorio::game::Chunk_tile::set_layer_sprite_prototype(const chunk_layer category, 
                                                            data::Sprite* tile_prototype) {
	assert(category == chunk_layer::overlay);
	layers[get_layer_index(category)].set_data(tile_prototype);
}
