#ifndef DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H

#include "health_entity.h"

namespace jactorio::data
{
	/**
	 * An entity with an inventory, such as a chest
	 */
	class Container_entity : public Health_entity
	{
	public:
		Container_entity() = default;

		~Container_entity() override = default;

		unsigned short inventory_size;

		/**
		 * Sprite drawn when placed in the world
		 */
		Sprite* sprite;
	};
}

#endif // DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
