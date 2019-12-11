#include "game/world/chunk_tile.h"

jactorio::game::Chunk_tile::Chunk_tile() {
	for (auto& tile_prototype : tile_prototypes) {
		tile_prototype = nullptr;
	}
}

jactorio::data::Tile* jactorio::game::Chunk_tile::get_tile_prototype(prototype_category category) const {
	return tile_prototypes[static_cast<int>(category)];
}

void jactorio::game::Chunk_tile::set_tile_prototype(prototype_category category,
                                                    data::Tile* tile_prototype) {
	tile_prototypes[static_cast<int>(category)] = tile_prototype;
}
