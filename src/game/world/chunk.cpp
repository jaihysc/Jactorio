#include <game/world/chunk.h>

jactorio::game::Chunk::Chunk(const int x, const int y, ChunkTile* tiles) {
	position_.first = x;
	position_.second = y;

	if (tiles == nullptr) {
		for (auto& tile : tiles_) {
			tile = new ChunkTile();
		}
	}
	else {
		for (int i = 0; i < 32 * 32; ++i) {
			// Make a copy of the provided tile so it can be deleted without affecting anything else
			const auto tile_copy = new ChunkTile();
			*tile_copy = tiles[i];
			
			tiles_[i] = tile_copy;
		}
	}
}

jactorio::game::Chunk::~Chunk() {
	for (auto& tile : tiles_) {
		delete tile;
	}
}

std::pair<int, int> jactorio::game::Chunk::get_position() const {
	return position_;
}