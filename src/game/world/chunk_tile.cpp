// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile.h"

#include "proto/abstract/entity.h"
#include "proto/tile.h"

using namespace jactorio;

// ======================================================================
// Tile
const proto::Tile* game::ChunkTile::GetTilePrototype(const TileLayer category) const {

    assert(category == TileLayer::base);
    return static_cast<const proto::Tile*>(layers[GetLayerIndex(category)].GetPrototype());
}

void game::ChunkTile::SetTilePrototype(const Orientation orientation,
                                       const proto::Tile* tile_prototype,
                                       const TileLayer category) {

    assert(category == TileLayer::base);
    layers[GetLayerIndex(category)].SetPrototype(orientation, tile_prototype);
}

// ======================================================================
// Entity
const proto::Entity* game::ChunkTile::GetEntityPrototype(const TileLayer category) const {

    assert(category == TileLayer::resource || category == TileLayer::entity);
    return static_cast<const proto::Entity*>(layers[GetLayerIndex(category)].GetPrototype());
}

void game::ChunkTile::SetEntityPrototype(const Orientation orientation,
                                         const proto::Entity* tile_prototype,
                                         const TileLayer category) {

    assert(category == TileLayer::resource || category == TileLayer::entity);
    layers[GetLayerIndex(category)].SetPrototype(orientation, tile_prototype);
}
