// 
// placement_controller.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 04/03/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
#pragma once

#include "jactorio.h"

#include "data/prototype/entity/entity.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	namespace placement_c
	{
		///
		/// \return true if a entity with the specified dimensions can be placed at x, y
		bool placement_location_valid(World_data& world_data, uint8_t tile_width,
		                              uint8_t tile_height, int x, int y);

		// ======================================================================
		// Entity placement

		///
		/// \brief Places / removes entities at world coordinates
		/// \remark Pass nullptr for entity to remove
		/// \remark Assumes placement / removal location is valid
		/// Chunk_tile_layer.clear() for the entity layer is called on removal 
		bool place_entity_at_coords(World_data& world_data, data::Entity* entity, int x, int y);


		// ======================================================================
		// Sprite placement

		/// \brief Places / remove sprite at world coordinates
		/// \remark Pass nullptr for sprite to remove
		/// \remark Assumes placement / removal location is valid
		void place_sprite_at_coords(World_data& world_data, Chunk_tile::chunkLayer layer,
		                            data::Sprite* sprite, uint8_t tile_width, uint8_t tile_height, int x, int y);


		// ======================================================================
		// Lower level placement functions

		///
		/// Places a multi-tile with x, y being the top left of the multi-tile placed
		void place_at_coords(World_data& world_data,
		                     Chunk_tile::chunkLayer layer, uint8_t tile_width,
		                     uint8_t tile_height, int x,
		                     int y, void (*place_func)(Chunk_tile*));

		///
		/// \brief Removes a entire multi-tile item at x, y.
		/// x, y can be anywhere within the multi-tile
		void remove_at_coords(World_data& world_data,
		                      Chunk_tile::chunkLayer layer, uint8_t tile_width,
		                      uint8_t tile_height, int x,
		                      int y, void (*remove_func)(Chunk_tile*));
	}
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
