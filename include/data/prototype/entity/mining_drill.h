// 
// mining_drill.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/02/2020
// Last modified: 04/03/2020
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
	class Mining_drill final : public Health_entity, public Deferred, public Rotatable_entity
	{
	public:
		PYTHON_PROP_REF_I(Mining_drill, double, mining_speed, 1.f);  // Mines 1 resource every 60 game ticks

		/// Number of tiles to extend the mining radius around the entity outside of entity tile width and height	
		PYTHON_PROP_REF_I(Mining_drill, uint16_t, mining_border, 1);

		void on_defer_time_elapsed(Unique_data_base* unique_data) override {

		}


		void on_r_show_gui(game::Player_data& player_data, game::Chunk_tile_layer* tile_layer) const override {
		}


		J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(placementOrientation orientation,
		                                                                    game::World_data& world_data,
		                                                                    std::pair<int, int> world_coords) const override {
			return {0, 0};
		}


		void post_load_validate() const override {
			J_DATA_ASSERT(sprite != nullptr, "North sprite not provided");
			J_DATA_ASSERT(sprite_e != nullptr, "East sprite not provided");
			J_DATA_ASSERT(sprite_s != nullptr, "South sprite not provided");
			J_DATA_ASSERT(sprite_w != nullptr, "West sprite not provided");
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_MINING_DRILL_H
