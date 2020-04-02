// 
// chunk_tile.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/02/2020
// Last modified: 04/02/2020
// 

#include "game/world/chunk_tile.h"

#include "data/prototype/tile/tile.h"
#include "data/prototype/entity/entity.h"

// ======================================================================
// Tile
const jactorio::data::Tile* jactorio::game::Chunk_tile::get_tile_prototype(const chunkLayer category) const {

	assert(category == Chunk_tile::chunkLayer::base);
	return static_cast<const data::Tile*>(layers[get_layer_index(category)].prototype_data);
}

void jactorio::game::Chunk_tile::set_tile_prototype(const chunkLayer category, const data::Tile* tile_prototype) const {

	assert(category == Chunk_tile::chunkLayer::base);
	layers[get_layer_index(category)].prototype_data = tile_prototype;
}

// ======================================================================
// Entity
const jactorio::data::Entity* jactorio::game::Chunk_tile::get_entity_prototype(const chunkLayer category) const {

	assert(category == Chunk_tile::chunkLayer::resource || category == Chunk_tile::chunkLayer::entity);
	return static_cast<const data::Entity*>(layers[get_layer_index(category)].prototype_data);
}

void jactorio::game::Chunk_tile::set_entity_prototype(const chunkLayer category, const data::Entity* tile_prototype) const {

	assert(category == Chunk_tile::chunkLayer::resource || category == Chunk_tile::chunkLayer::entity);
	layers[get_layer_index(category)].prototype_data = tile_prototype;
}

// ======================================================================
// Sprite
const jactorio::data::Sprite* jactorio::game::Chunk_tile::get_sprite_prototype(const chunkLayer category) const {

	assert(category == Chunk_tile::chunkLayer::overlay);
	return static_cast<const data::Sprite*>(layers[get_layer_index(category)].prototype_data);
}

void jactorio::game::Chunk_tile::set_sprite_prototype(const chunkLayer category, const data::Sprite* tile_prototype) const {

	assert(category == Chunk_tile::chunkLayer::overlay);
	layers[get_layer_index(category)].prototype_data = tile_prototype;
}
