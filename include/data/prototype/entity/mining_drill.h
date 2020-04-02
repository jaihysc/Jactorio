// 
// mining_drill.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/02/2020
// Last modified: 04/02/2020
// 

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_MINING_DRILL_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_MINING_DRILL_H
#pragma once

#include "data/prototype/entity/health_entity.h"
#include "data/prototype/interface/deferred.h"

namespace jactorio::data
{
	struct Mining_drill_data final : Health_entity_data
	{
		
	};


	///
	/// \brief Drill, Mines resource entities
	class Mining_drill final : public Health_entity, public Deferred
	{
	public:
		PYTHON_PROP_REF_I(Mining_drill, double, mining_speed, 1.f);  // Mines 1 resource every 60 game ticks
		
		void on_defer_time_elapsed(Unique_data_base* unique_data) override {
			
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_MINING_DRILL_H
