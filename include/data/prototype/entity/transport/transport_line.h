// 
// transport_line.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/10/2020
// Last modified: 03/20/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#pragma once

#include "core/data_type.h"
#include "data/prototype/entity/health_entity.h"

namespace jactorio::data
{
	struct Transport_line_data : Health_entity_data
	{
		///
		/// <Entry direction>_<Exit direction>
		enum class orientation
		{
			up_left = 0,
			up,
			up_right,

			right_up,
			right,
			right_down,

			down_right,
			down,
			down_left,

			left_down,
			left,
			left_up,

			count_
		};

		orientation orientation = orientation::up;
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


		void delete_unique_data(void* ptr) const override {
			delete static_cast<Transport_line_data*>(ptr);
		}


		// ======================================================================
		// Game events
		void on_build(game::Chunk_tile_layer* tile_layer) const override {
			tile_layer->unique_data = new Transport_line_data();
		}


		std::pair<uint16_t, uint16_t> map_placement_orientation(const placementOrientation orientation,
		                                                        game::Chunk_tile_layer* up,
		                                                        game::Chunk_tile_layer* right,
		                                                        game::Chunk_tile_layer* down,
		                                                        game::Chunk_tile_layer* left) const override {
			switch (orientation) {
			case placementOrientation::up:
				return {17, 0};
			case placementOrientation::right:
				return {19, 0};
			case placementOrientation::down:
				return {16, 0};
			case placementOrientation::left:
				return {18, 0};
			default:
				assert(false); // Missing switch case
			}

			return {0, 0};
		}


		// ======================================================================
		// Data events
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
