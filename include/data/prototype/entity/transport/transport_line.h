// 
// transport_line.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/10/2020
// Last modified: 03/23/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#pragma once

#include <vector>

#include "core/data_type.h"
#include "data/prototype/entity/health_entity.h"

namespace jactorio::data
{
	struct Transport_line_data : Health_entity_data
	{
		///
		/// <Entry direction>_<Exit direction>
		enum class lineOrientation
		{
			// Following the layout of the sprite
			up_left = 9,
			up = 17,
			up_right = 11,

			right_up = 13,
			right = 19,
			right_down = 8,

			down_right = 14,
			down = 16,
			down_left = 12,

			left_down = 10,
			left = 18,
			left_up = 15,
		};

		lineOrientation orientation = lineOrientation::up;

		///
		/// \brief Updates orientation and member set for rendering 
		void set_orientation(lineOrientation orientation) {
			this->orientation = orientation;
			this->set = static_cast<uint16_t>(orientation);
		}
	};

	///
	/// \brief Abstract class for all everything which moves items (belts, underground belts, splitters)
	class Transport_line : public Health_entity, public Rotatable_entity
	{
	protected:
		Transport_line() = default;

	public:
		///
		/// \brief Number of tiles traveled by each item on the belt per tick
		/// \remark For Python API use only
		PYTHON_PROP_I(Transport_line, float, speed_float, 0.01);

		/// Number of tiles traveled by each item on the belt per tick
		transport_line_offset speed;


		// ======================================================================
		// Game events

		///
		///	\brief Updates the orientation of current and neighboring transport lines 
		static void update_neighboring_orientation(game::World_data& world_data, std::pair<int, int> world_coords,
		                                           Transport_line_data* t_center,
		                                           Transport_line_data* c_right,
		                                           Transport_line_data* b_center,
		                                           Transport_line_data* c_left, Transport_line_data* center);

		void on_build(game::World_data& world_data, std::pair<int, int> world_coords,
		              game::Chunk_tile_layer& tile_layer, uint16_t frame, placementOrientation orientation) const override;

		void on_remove(game::World_data& world_data, std::pair<int, int> world_coords,
		               game::Chunk_tile_layer& tile_layer) const override;

		J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(placementOrientation orientation,
		                                                                    game::World_data& world_data,
		                                                                    std::pair<int, int> world_coords) const override;


		// ======================================================================
		// Data events
		void delete_unique_data(void* ptr) const override {
			delete static_cast<Transport_line_data*>(ptr);
		}

		void post_load() override {
			// Convert floating point speed to fixed precision decimal speed
			speed = transport_line_offset(speed_float);
		}

		void post_load_validate() const override {
			J_DATA_ASSERT(speed_float > 0.001, "Transport line speed below minimum 0.001");
			// Cannot exceed item_width because of limitations in the logic
			J_DATA_ASSERT(speed_float < 0.25, "Transport line speed equal or above maximum of 0.25");
		}

		void on_r_show_gui(game::Player_data& player_data, game::Chunk_tile_layer* tile_layer) const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
