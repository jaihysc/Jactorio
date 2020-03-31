// 
// chunk.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/15/2019
// Last modified: 03/14/2020
// 

#include <game/world/chunk.h>

jactorio::game::Chunk::Chunk(const int x, const int y, Chunk_tile* tiles) {
	position_.first = x;
	position_.second = y;

	if (tiles == nullptr) {
		// Allocate and initialize
		tiles_ = new Chunk_tile[32 * 32];
		for (int i = 0; i < 32 * 32; ++i) {
			tiles_[i] = Chunk_tile();
		}
	}
	else {
		tiles_ = tiles;
	}
}

jactorio::game::Chunk::~Chunk() {
	delete[] tiles_;
}

jactorio::game::Chunk::Chunk(const Chunk& other)
	: position_(other.position_) {

	tiles_ = new Chunk_tile[32 * 32];
	for (int i = 0; i < 32 * 32; ++i) {
		tiles_[i] = other.tiles_[i];
	}
}

jactorio::game::Chunk::Chunk(Chunk&& other) noexcept
	: position_(std::move(other.position_)),
	  tiles_(other.tiles_) {
}

jactorio::game::Chunk& jactorio::game::Chunk::operator=(const Chunk& other) {
	if (this == &other)
		return *this;
	position_ = other.position_;

	tiles_ = new Chunk_tile[32 * 32];
	for (int i = 0; i < 32 * 32; ++i) {
		tiles_[i] = other.tiles_[i];
	}
	return *this;
}

jactorio::game::Chunk& jactorio::game::Chunk::operator=(Chunk&& other) noexcept {
	if (this == &other)
		return *this;
	position_ = std::move(other.position_);
	tiles_ = other.tiles_;
	return *this;
}


std::pair<int, int> jactorio::game::Chunk::get_position() const {
	return position_;
}
