#ifndef DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H

#include "entity.h"

namespace jactorio::data
{
	class Health_entity : public Entity
	{
	public:
		Health_entity()
			: max_health(0) {
		}

		~Health_entity() override = default;

		/**
		 * How many hit points this entity can have before it dies <br>
		 * 0 max health is invalid
		 */
		PYTHON_PROP_REF(Health_entity, unsigned short, max_health)
	};
}

#endif // DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
