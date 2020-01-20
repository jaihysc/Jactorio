#ifndef GAME_LOGIC_PLACEMENT_CONTROLLER_H
#define GAME_LOGIC_PLACEMENT_CONTROLLER_H

#include "jactorio.h"
#include "data/prototype/entity/entity.h"
#include "game/world/chunk_tile.h"

/**
 * All headers in game/logic/ fall under the logic namespace instead of their own.
 * They together make up the game logic
 */
namespace jactorio::game
{
	namespace placement_c
	{		
		/**
		 * @return true if a entity with the specified dimensions can be placed at x, y
		 */
		bool placement_location_valid(uint8_t tile_width, uint8_t tile_height,
		                              int x, int y);

		// Entity placement

		/**
		 * Places an entity at the specified world coordinates if within range of the player <br>
		 * Passing an entity of nullptr will remove the entity on x, y
		 * @return true if successfully placed
		 */
		bool place_entity_at_coords_ranged(data::Entity* entity, int x, int y);

		/**
		 * Same as place_entity_at_coords(), range check is not performed
		 */
		bool place_entity_at_coords(data::Entity* entity, int x, int y);


		//
		// Sprite placement

		/**
		 * Pass nullptr as sprite to remove <br>
		 * It is always assumed that the placement / removal location is valid
		 */
		void place_sprite_at_coords(Chunk_tile::chunk_layer layer, data::Sprite* sprite,
		                            uint8_t tile_width, uint8_t tile_height, int x, int y);


		//
		// Lower level placement functions

		/**
		 * Places a multi-tile with x, y being the top left of the multi-tile placed
		 * @return true if placed successfully
		 */
		void place_at_coords(Chunk_tile::chunk_layer layer,
		                     uint8_t tile_width, uint8_t tile_height,
		                     int x, int y,
		                     void (*place_func)(Chunk_tile*));

		/**
		 * Removes a entire multi-tile item at x, y <br>
		 * x, y can be anywhere within the multi-tile
		 */
		void remove_at_coords(Chunk_tile::chunk_layer layer,
		                      uint8_t tile_width, uint8_t tile_height,
		                      int x, int y,
		                      void (*remove_func)(Chunk_tile*));
	}
}

#endif // GAME_LOGIC_PLACEMENT_CONTROLLER_H
