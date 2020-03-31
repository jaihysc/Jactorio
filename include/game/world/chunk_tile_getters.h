// 
// chunk_tile_getters.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/12/2020
// Last modified: 03/24/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_GETTERS_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_GETTERS_H
#pragma once

#include "jactorio.h"

#include "game/world/chunk_tile.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/tile/tile.h"

///
/// Type checked functions for getting prototype data
namespace jactorio::game::chunk_tile_getter
{
	// ============================================================================================
	// Functions below are type checked to avoid getting / setting the wrong data

	// chunk_layer::base only
	J_NODISCARD const data::Tile* get_tile_prototype(const Chunk_tile& chunk_tile,
	                                                 Chunk_tile::chunkLayer category);
	void set_tile_prototype(const Chunk_tile& chunk_tile,
	                        Chunk_tile::chunkLayer category,
	                        data::Tile* tile_prototype);


	// chunk_layer::resource, chunk_layer::entity only
	J_NODISCARD const data::Entity* get_entity_prototype(const Chunk_tile& chunk_tile,
	                                                     Chunk_tile::chunkLayer category);
	void set_entity_prototype(const Chunk_tile& chunk_tile,
	                          Chunk_tile::chunkLayer category,
	                          data::Entity* tile_prototype);


	// chunk_layer::overlay only
	J_NODISCARD const data::Sprite* get_sprite_prototype(const Chunk_tile& chunk_tile,
	                                                     Chunk_tile::chunkLayer category);
	void set_sprite_prototype(const Chunk_tile& chunk_tile,
	                          Chunk_tile::chunkLayer category,
	                          data::Sprite* tile_prototype);
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_GETTERS_H
