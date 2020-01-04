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
		Container_entity()
			: inventory_size(0) {
		}

		~Container_entity() override = default;

		
		PYTHON_PROP_REF(Container_entity, unsigned short, inventory_size)
	};
}

#endif // DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
