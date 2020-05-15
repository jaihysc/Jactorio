// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#include "game/world/chunk_tile.h"

#include "data/prototype/entity/entity.h"
#include "data/prototype/tile/tile.h"

// ======================================================================
// Tile
const jactorio::data::Tile* jactorio::game::ChunkTile::GetTilePrototype(const ChunkLayer category) const {

	assert(category == ChunkTile::ChunkLayer::base);
	return static_cast<const data::Tile*>(layers[GetLayerIndex(category)].prototypeData);
}

void jactorio::game::ChunkTile::SetTilePrototype(const ChunkLayer category, const data::Tile* tile_prototype) const {

	assert(category == ChunkTile::ChunkLayer::base);
	layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}

// ======================================================================
// Entity
const jactorio::data::Entity* jactorio::game::ChunkTile::GetEntityPrototype(const ChunkLayer category) const {

	assert(category == ChunkTile::ChunkLayer::resource || category == ChunkTile::ChunkLayer::entity);
	return static_cast<const data::Entity*>(layers[GetLayerIndex(category)].prototypeData);
}

void jactorio::game::ChunkTile::SetEntityPrototype(const ChunkLayer category, const data::Entity* tile_prototype) const {

	assert(category == ChunkTile::ChunkLayer::resource || category == ChunkTile::ChunkLayer::entity);
	layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}

// ======================================================================
// Sprite
const jactorio::data::Sprite* jactorio::game::ChunkTile::GetSpritePrototype(const ChunkLayer category) const {

	assert(category == ChunkTile::ChunkLayer::overlay);
	return static_cast<const data::Sprite*>(layers[GetLayerIndex(category)].prototypeData);
}

void jactorio::game::ChunkTile::SetSpritePrototype(const ChunkLayer category, const data::Sprite* tile_prototype) const {

	assert(category == ChunkTile::ChunkLayer::overlay);
	layers[GetLayerIndex(category)].prototypeData = tile_prototype;
}
