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
		Entity()
			: tile_width(1), tile_height(1), rotatable(false), item(nullptr) {
		}
		
		~Entity() override = default;
		
		// Number of tiles this entity spans
		PYTHON_PROP_REF(Entity, unsigned short, tile_width)
		PYTHON_PROP_REF(Entity, unsigned short, tile_height)
		
		// Can be rotated by player?
		PYTHON_PROP_REF(Entity, bool, rotatable)

		// Item when entity is picked up
		PYTHON_PROP(Entity, Item*, item)
	};
}

#endif // DATA_PROTOTYPE_ENTITY_ENTITY_H
