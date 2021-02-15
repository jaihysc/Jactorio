// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile.h"

#include "proto/abstract/entity.h"
#include "proto/resource_entity.h"
#include "proto/tile.h"

using namespace jactorio;

const proto::Tile* game::ChunkTile::BasePrototype() const {
    return Base().GetPrototype<proto::Tile>();
}

const proto::ResourceEntity* game::ChunkTile::ResourcePrototype() const {
    return Resource().GetPrototype<proto::ResourceEntity>();
}

const proto::Entity* game::ChunkTile::EntityPrototype() const {
    return Entity().GetPrototype<proto::Entity>();
}
