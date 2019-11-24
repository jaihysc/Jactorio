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

std::pair<int, int> jactorio::game::Chunk::get_position() const {
	return position_;
}