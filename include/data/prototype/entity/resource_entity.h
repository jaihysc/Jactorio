// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 02/07/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#pragma once

#include "data/prototype/entity/entity.h"

namespace jactorio::data
{
	// Unique per resource entity placed
	struct Resource_entity_data : Entity_data
	{
		explicit Resource_entity_data(const uint16_t resource_amount)
			: resource_amount(resource_amount) {
		}

		/**
		 * Amount of product which can still be extracted from this tile
		 */
		uint16_t resource_amount;
	};

	class Resource_entity final : public Entity
	{
	public:
		PROTOTYPE_CATEGORY(resource_entity);

		Resource_entity() {
			// Resource entities can never be placed
			this->placeable = false;
		}

		Unique_data_base* copy_unique_data(Unique_data_base* ptr) const override {
			return new Resource_entity_data(*static_cast<Resource_entity_data*>(ptr));
		}


		void on_r_show_gui(game::Player_data& player_data, game::Chunk_tile_layer* tile_layer) const override {
		}

		std::pair<uint16_t, uint16_t> map_placement_orientation(placementOrientation orientation, game::World_data& world_data,
		                                                        std::pair<int, int> world_coords) const override {
			return {0, 0};
		}


		void on_build(game::World_data& world_data,
		              std::pair<game::World_data::world_coord, game::World_data::world_coord> world_coords,
		              game::Chunk_tile_layer& tile_layer, uint16_t frame, placementOrientation orientation) const override {
			assert(false);  // Is not player placeable
		}

		void on_remove(game::World_data& world_data,
		               std::pair<game::World_data::world_coord, game::World_data::world_coord> world_coords,
		               game::Chunk_tile_layer& tile_layer) const override {
		}


		void post_load_validate() const override {
			// Must convert to at least 1 game tick
			J_DATA_ASSERT(pickup_time * JC_GAME_HERTZ >= 1, "Pickup time is too small");
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
