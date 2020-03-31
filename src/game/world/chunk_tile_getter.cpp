// 
// chunk_tile_getter.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/21/2019
// Last modified: 03/24/2020
// 

#include "game/world/chunk_tile_getters.h"

// ======================================================================
// Tile
const jactorio::data::Tile* jactorio::game::chunk_tile_getter::get_tile_prototype(
	const Chunk_tile& chunk_tile, const Chunk_tile::chunkLayer category) {

	assert(category == Chunk_tile::chunkLayer::base);
	return static_cast<const data::Tile*>(chunk_tile.layers[Chunk_tile::get_layer_index(category)].prototype_data);
}

void jactorio::game::chunk_tile_getter::set_tile_prototype(
	const Chunk_tile& chunk_tile, const Chunk_tile::chunkLayer category,
	data::Tile* tile_prototype) {

	assert(category == Chunk_tile::chunkLayer::base);
	chunk_tile.layers[Chunk_tile::get_layer_index(category)].prototype_data = tile_prototype;
}


// ======================================================================
// Entity
const jactorio::data::Entity* jactorio::game::chunk_tile_getter::get_entity_prototype(
	const Chunk_tile& chunk_tile, const Chunk_tile::chunkLayer category) {
	assert(category == Chunk_tile::chunkLayer::resource || category == Chunk_tile::chunkLayer::entity);
	return static_cast<const data::Entity*>(chunk_tile.layers[Chunk_tile::get_layer_index(category)].prototype_data);
}


void jactorio::game::chunk_tile_getter::set_entity_prototype(
	const Chunk_tile& chunk_tile, const Chunk_tile::chunkLayer category,
	data::Entity* tile_prototype) {

	assert(category == Chunk_tile::chunkLayer::resource || category == Chunk_tile::chunkLayer::entity);
	chunk_tile.layers[Chunk_tile::get_layer_index(category)].prototype_data = tile_prototype;
}


// ======================================================================
// Sprite
const jactorio::data::Sprite* jactorio::game::chunk_tile_getter::get_sprite_prototype(
	const Chunk_tile& chunk_tile, const Chunk_tile::chunkLayer category) {

	assert(category == Chunk_tile::chunkLayer::overlay);
	return static_cast<const data::Sprite*>(chunk_tile.layers[Chunk_tile::get_layer_index(category)].prototype_data);
}

void jactorio::game::chunk_tile_getter::set_sprite_prototype(
	const Chunk_tile& chunk_tile, const Chunk_tile::chunkLayer category,
	data::Sprite* tile_prototype) {

	assert(category == Chunk_tile::chunkLayer::overlay);
	chunk_tile.layers[Chunk_tile::get_layer_index(category)].prototype_data = tile_prototype;
}
