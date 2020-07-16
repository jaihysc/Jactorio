// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk.h"

jactorio::game::Chunk::Chunk(ChunkCoord chunk_x, ChunkCoord chunk_y)
	: position_({chunk_x, chunk_y}) {

	// Allocate and initialize
	tiles_ = new ChunkTile[kChunkWidth * kChunkWidth];
	for (int i = 0; i < kChunkWidth * kChunkWidth; ++i) {
		tiles_[i] = ChunkTile();
	}
}

jactorio::game::Chunk::Chunk(const ChunkCoord chunk_x, const ChunkCoord chunk_y, ChunkTile* tiles)
	: position_({chunk_x, chunk_y}) {
	assert(tiles != nullptr);

	this->tiles_ = tiles;
}

jactorio::game::Chunk::~Chunk() {
	delete[] tiles_;
}

jactorio::game::Chunk::Chunk(const Chunk& other)
	: position_(other.position_) {

	tiles_ = new ChunkTile[kChunkWidth * kChunkWidth];
	for (int i = 0; i < kChunkWidth * kChunkWidth; ++i) {
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
