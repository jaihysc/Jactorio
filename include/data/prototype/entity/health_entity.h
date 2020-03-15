// 
// health_entity.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
#pragma once

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

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_HEALTH_ENTITY_H
