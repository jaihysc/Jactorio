#ifndef GAME_LOGIC_ENTITY_PLACE_CONTROLLER_H
#define GAME_LOGIC_ENTITY_PLACE_CONTROLLER_H

#include "data/prototype/entity/entity.h"

/**
 * All headers in game/logic/ fall under the logic namespace instead of their own.
 * They together make up the game logic
 */
namespace jactorio::game::logic
{
	/**
	 * Places an entity at the specified world coordinates if within range of the player
	 * @return true if successfully placed
	 */
	bool place_entity_at_coords(data::Entity* entity, int x, int y);

	/**
	 * Same as place_entity_at_coords(), range check is not performed
	 */
	bool place_entity_at_coords_unranged(data::Entity* entity, int x, int y);
}

#endif // GAME_LOGIC_ENTITY_PLACE_CONTROLLER_H
