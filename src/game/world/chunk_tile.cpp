// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile.h"

#include "data/prototype/tile.h"
#include "data/prototype/abstract_proto/entity.h"

// ======================================================================
// Tile
const jactorio::data::Tile* jactorio::game::ChunkTile::GetTilePrototype(const ChunkLayer category) const {

	assert(category == ChunkTile::ChunkLayer::base);
	return static_cast<const data::Tile*>(layers[GetLayerIndex(category)].prototypeData.Get());
}

void jactorio::game::ChunkTile::SetTilePrototype(const data::Tile* tile_prototype, const ChunkLayer category) {

	assert(category == ChunkTile::ChunkLayer::base);
	layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}

// ======================================================================
// Entity
const jactorio::data::Entity* jactorio::game::ChunkTile::GetEntityPrototype(const ChunkLayer category) const {

	assert(category == ChunkTile::ChunkLayer::resource || category == ChunkTile::ChunkLayer::entity);
	return static_cast<const data::Entity*>(layers[GetLayerIndex(category)].prototypeData.Get());
}

void jactorio::game::ChunkTile::SetEntityPrototype(const data::Entity* tile_prototype, const ChunkLayer category) {

	assert(category == ChunkTile::ChunkLayer::resource || category == ChunkTile::ChunkLayer::entity);
	layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}
