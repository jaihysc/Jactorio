#include <game/world/chunk.h>

jactorio::game::Chunk::Chunk(const int x, const int y, const Tile* tiles) {
	position_.first = x;
	position_.second = y;

	for (int i = 0; i < 32 * 32; ++i) {
		// TODO
		// tiles_[i] = tiles[i];
	}
}

std::pair<int, int> jactorio::game::Chunk::get_position() const {
	return position_;
}

jactorio::game::Tile* const* jactorio::game::Chunk::tiles_ptr() const {
	return tiles_;
}
