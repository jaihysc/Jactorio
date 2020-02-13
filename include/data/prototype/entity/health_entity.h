#ifndef DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H

#include "entity.h"

namespace jactorio::data
{
	struct Health_entity_data : Entity_data
	{
		uint16_t health;
	};
	
	class Health_entity : public Entity
	{
	protected:
		Health_entity()
			: max_health(0) {
		}

	public:
		/**
		 * How many hit points this entity can have before it dies <br>
		 * 0 max health is invalid
		 */
		PYTHON_PROP_REF(Health_entity, unsigned short, max_health)
	};
}

#endif // DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
