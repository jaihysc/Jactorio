#include "game/world/chunk_tile_layer.h"

jactorio::game::Chunk_tile_layer::Chunk_tile_layer(const Chunk_tile_layer& other)
	: prototype_data(other.prototype_data),
	  multi_tile_index(other.multi_tile_index),
	  multi_tile_span(other.multi_tile_span),
	  multi_tile_height(other.multi_tile_height) {
	// Use prototype defined method for copying unique_data_
	if (unique_data != nullptr) {
		assert(prototype_data != nullptr);  // No prototype_data_ available for copying unique_data_
		unique_data = prototype_data->copy_unique_data(other.unique_data);
	}
}

jactorio::game::Chunk_tile_layer::Chunk_tile_layer(Chunk_tile_layer&& other) noexcept
	: prototype_data(other.prototype_data),
	  unique_data(other.unique_data),
	  multi_tile_index(other.multi_tile_index),
	  multi_tile_span(other.multi_tile_span),
	  multi_tile_height(other.multi_tile_height) {
}

jactorio::game::Chunk_tile_layer& jactorio::game::Chunk_tile_layer::operator=(const Chunk_tile_layer& other) {
	if (this == &other)
		return *this;
	prototype_data = other.prototype_data;
	multi_tile_index = other.multi_tile_index;
	multi_tile_span = other.multi_tile_span;
	multi_tile_height = other.multi_tile_height;

	// Use prototype defined method for copying unique_data_
	if (unique_data != nullptr) {
		assert(prototype_data != nullptr);  // No prototype_data_ available for copying unique_data_
		unique_data = prototype_data->copy_unique_data(other.unique_data);
	}
	return *this;
}

jactorio::game::Chunk_tile_layer& jactorio::game::Chunk_tile_layer::operator=(Chunk_tile_layer&& other) noexcept {
	if (this == &other)
		return *this;
	prototype_data = other.prototype_data;
	unique_data = other.unique_data;
	multi_tile_index = other.multi_tile_index;
	multi_tile_span = other.multi_tile_span;
	multi_tile_height = other.multi_tile_height;
	return *this;
}


void jactorio::game::Chunk_tile_layer::clear() {
	delete_unique_data();
	unique_data = nullptr;
	prototype_data = nullptr;

	multi_tile_index = 0;
	multi_tile_span = 1;
	multi_tile_height = 1;
}

void jactorio::game::Chunk_tile_layer::delete_unique_data() const {
	if (unique_data != nullptr) {
		assert(prototype_data != nullptr);  // Unique_data_ was defined, but no prototype_data_ is available to delete it
		prototype_data->delete_unique_data(unique_data);
	}
}
