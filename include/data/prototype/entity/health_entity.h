// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
#pragma once

#include "entity.h"

namespace jactorio::data
{
	/// Default health of all Health entities
	constexpr uint16_t default_health = 1;

	struct Health_entity_data : Entity_data
	{
		uint16_t health;
	};

	class Health_entity : public Entity
	{
	protected:
		Health_entity() = default;

	public:
		/**
		 * How many hit points this entity can have before it dies <br>
		 * 0 max health is invalid
		 */
		PYTHON_PROP_REF_I(Health_entity, uint16_t, max_health, default_health);
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
