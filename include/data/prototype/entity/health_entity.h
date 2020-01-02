#ifndef DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H

#include "entity.h"

namespace jactorio::data
{
	class Health_entity : public Entity
	{
	public:
		Health_entity() = default;

		~Health_entity() override = default;
		
		unsigned short max_health;
	};
}

#endif // DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
