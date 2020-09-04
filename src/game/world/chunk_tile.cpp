// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile.h"

#include "data/prototype/abstract_proto/entity.h"
#include "data/prototype/tile.h"

// ======================================================================
// Tile
const jactorio::data::Tile* jactorio::game::ChunkTile::GetTilePrototype(const TileLayer category) const {

    assert(category == TileLayer::base);
    return static_cast<const data::Tile*>(layers[GetLayerIndex(category)].prototypeData.Get());
}

void jactorio::game::ChunkTile::SetTilePrototype(const data::Tile* tile_prototype, const TileLayer category) {

    assert(category == TileLayer::base);
    layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}

// ======================================================================
// Entity
const jactorio::data::Entity* jactorio::game::ChunkTile::GetEntityPrototype(const TileLayer category) const {

    assert(category == TileLayer::resource || category == TileLayer::entity);
    return static_cast<const data::Entity*>(layers[GetLayerIndex(category)].prototypeData.Get());
}

void jactorio::game::ChunkTile::SetEntityPrototype(const data::Entity* tile_prototype, const TileLayer category) {

    assert(category == TileLayer::resource || category == TileLayer::entity);
    layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}
