// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile.h"

#include "proto/abstract_proto/entity.h"
#include "proto/tile.h"

// ======================================================================
// Tile
const jactorio::proto::Tile* jactorio::game::ChunkTile::GetTilePrototype(const TileLayer category) const {

    assert(category == TileLayer::base);
    return static_cast<const proto::Tile*>(layers[GetLayerIndex(category)].prototypeData.Get());
}

void jactorio::game::ChunkTile::SetTilePrototype(const proto::Tile* tile_prototype, const TileLayer category) {

    assert(category == TileLayer::base);
    layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}

// ======================================================================
// Entity
const jactorio::proto::Entity* jactorio::game::ChunkTile::GetEntityPrototype(const TileLayer category) const {

    assert(category == TileLayer::resource || category == TileLayer::entity);
    return static_cast<const proto::Entity*>(layers[GetLayerIndex(category)].prototypeData.Get());
}

void jactorio::game::ChunkTile::SetEntityPrototype(const proto::Entity* tile_prototype, const TileLayer category) {

    assert(category == TileLayer::resource || category == TileLayer::entity);
    layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}
