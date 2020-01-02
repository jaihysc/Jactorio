#ifndef DATA_PROTOTYPE_ENTITY_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_ENTITY_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/item/item.h"

namespace jactorio::data
{
	/**
	 * Placeable items in the world
	 */
	class Entity : public Prototype_base
	{
	public:
		~Entity() override = default;
		
		// Number of tiles this entity spans
		unsigned short tile_width = 1;
		unsigned short tile_height = 1;
		
		// Can be rotated by player?
		bool rotatable = false;
		
		// Item when entity is picked up
		Item* item = nullptr;
	};
}

#endif // DATA_PROTOTYPE_ENTITY_ENTITY_H
