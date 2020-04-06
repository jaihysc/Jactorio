// 
// transport_line.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/31/2020
// Last modified: 04/06/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#pragma once

#include <functional>

#include "core/data_type.h"
#include "data/prototype/entity/health_entity.h"
#include "game/logic/transport_line_structure.h"

namespace jactorio::data
{
	struct Transport_line_data : Health_entity_data
	{
		explicit Transport_line_data(game::Transport_line_segment& line_segment)
			: line_segment(line_segment) {
		}

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

		/// The logic chunk line_segment associated
		game::Transport_line_segment& line_segment;
	};

	///
	/// \brief Abstract class for all everything which moves items (belts, underground belts, splitters)
	class Transport_line : public Health_entity
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
		/// \brief Attempts to retrieve transport line data at world coordinates on tile
		/// \return pointer to data or nullptr if non existent
		J_NODISCARD static Transport_line_data* get_line_data(const game::World_data& world_data,
		                                                      game::World_data::world_coord world_x,
		                                                      game::World_data::world_coord world_y);

		///
		/// \brief Attempts to find transport line at world_x, world_y
		/// \param callback Called for each Chunk_struct_layer found matching Transport_line_data at world_x, world_y
		static void get_transport_line_struct_layer(game::World_data& world_data,
		                                            game::World_data::world_coord world_x, game::World_data::world_coord world_y,
		                                            const std::function<void(game::Chunk_struct_layer&)>& callback);
	private:
		///
		///	\brief Updates the orientation of current and neighboring transport lines 
		static void update_neighboring_orientation(const game::World_data& world_data,
		                                           std::pair<game::World_data::world_coord, game::World_data::world_coord>
		                                           world_coords,
		                                           Transport_line_data* t_center,
		                                           Transport_line_data* c_right,
		                                           Transport_line_data* b_center,
		                                           Transport_line_data* c_left, Transport_line_data* center);

		using update_segment_func = std::function<
			void(game::World_data& world_data,
			     int world_x, int world_y,
			     float world_offset_x, float world_offset_y,
			     game::Transport_line_segment::terminationType termination_type)>;

		using update_segment_side_only_func = std::function<
			void(game::World_data& world_data,
			     int world_x, int world_y,
			     float world_offset_x, float world_offset_y,
			     game::Transport_line_segment::moveDir direction,
			     game::Transport_line_segment::terminationType termination_type)>;

		///
		/// \brief Change the neighboring line segment termination type to a bend depending on Transport_line_data orientation
		/// Since the line_orientations were applied, it is confirmed that segments exist at neighboring locations
		/// \remark This does not move across logic chunks and may make the position negative
		/// \param func Called when line orientation is bending for updating provided line segment
		/// \param side_only_func Called when line orientation is straight for updating provided line segment 
		static void update_neighboring_transport_segment(game::World_data& world_data,
		                                                 int32_t world_x, int32_t world_y,
		                                                 Transport_line_data::lineOrientation line_orientation,
		                                                 const update_segment_func& func,
		                                                 const update_segment_side_only_func& side_only_func);
		///
		/// \brief Updates the transport segments of world_coords neighbor's neighbors
		static void update_neighboring_neighbor_transport_segment(game::World_data& world_data,
		                                                          std::pair<int, int> world_coords,
		                                                          Transport_line_data* t_center,
		                                                          Transport_line_data* c_right,
		                                                          Transport_line_data* c_left,
		                                                          Transport_line_data* b_center);
	public:

		void on_build(game::World_data& world_data, std::pair<int, int> world_coords,
		              game::Chunk_tile_layer& tile_layer, uint16_t frame, placementOrientation orientation) const override;

		void on_remove(game::World_data& world_data,
		               std::pair<game::World_data::world_coord, game::World_data::world_coord> world_coords,
		               game::Chunk_tile_layer& tile_layer) const override;

		J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(placementOrientation orientation,
		                                                                    game::World_data& world_data,
		                                                                    std::pair<int, int> world_coords) const override;


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
