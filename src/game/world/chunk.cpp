// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include "game/world/chunk.h"

jactorio::game::Chunk::Chunk(chunk_coord chunk_x, chunk_coord chunk_y)
	: position_({chunk_x, chunk_y}) {

	// Allocate and initialize
	tiles_ = new Chunk_tile[chunk_width * chunk_width];
	for (int i = 0; i < chunk_width * chunk_width; ++i) {
		tiles_[i] = Chunk_tile();
	}
}

jactorio::game::Chunk::Chunk(const chunk_coord chunk_x, const chunk_coord chunk_y, Chunk_tile* tiles)
	: position_({chunk_x, chunk_y}) {
	assert(tiles != nullptr);

	tiles_ = tiles;
}

jactorio::game::Chunk::~Chunk() {
	delete[] tiles_;
}

jactorio::game::Chunk::Chunk(const Chunk& other)
	: position_(other.position_) {

	tiles_ = new Chunk_tile[chunk_width * chunk_width];
	for (int i = 0; i < chunk_width * chunk_width; ++i) {
		tiles_[i] = other.tiles_[i];
	}
}

jactorio::game::Chunk::Chunk(Chunk&& other) noexcept
	: position_{std::move(other.position_)},
	  tiles_{other.tiles_} {
	other.tiles_ = nullptr;
}

jactorio::game::Chunk& jactorio::game::Chunk::operator=(Chunk other) {
	swap(*this, other);
	return *this;
}
