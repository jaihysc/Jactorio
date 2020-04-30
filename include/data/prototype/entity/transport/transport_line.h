// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

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
		enum class LineOrientation
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

		/// The logic chunk line_segment associated
		game::Transport_line_segment& line_segment;

		/// The distance to the head of the transport line
		/// \remark For rendering purposes, the length should never exceed ~2 chunks at most
		uint8_t line_segment_index = 0;

		LineOrientation orientation = LineOrientation::up;

		// ======================================================================
		// Methods

		///
		/// \brief Updates orientation and member set for rendering 
		void set_orientation(LineOrientation orientation) {
			this->orientation = orientation;
			this->set         = static_cast<uint16_t>(orientation);
		}

		///
		/// \brief Converts lineOrientation to placementOrientation
		static Orientation to_orientation(const LineOrientation line_orientation) {
			switch (line_orientation) {
			case LineOrientation::up:
			case LineOrientation::right_up:
			case LineOrientation::left_up:
				return Orientation::up;

			case LineOrientation::right:
			case LineOrientation::up_right:
			case LineOrientation::down_right:
				return Orientation::right;

			case LineOrientation::down:
			case LineOrientation::right_down:
			case LineOrientation::left_down:
				return Orientation::down;

			case LineOrientation::left:
			case LineOrientation::up_left:
			case LineOrientation::down_left:
				return Orientation::left;

			default:
				assert(false);  // Missing switch case
				return Orientation::up;
			}
		}
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
		PYTHON_PROP_I(Transport_line, double, speed_float, 0.01f);

		/// Number of tiles traveled by each item on the belt per tick
		transport_line_offset speed;


		// ======================================================================
		// Data access

		///
		/// \brief Attempts to retrieve transport line data at world coordinates on tile
		/// \return pointer to data or nullptr if non existent
		J_NODISCARD static Transport_line_data* get_line_data(const game::World_data& world_data,
		                                                      game::World_data::world_coord world_x,
		                                                      game::World_data::world_coord world_y);

		static Transport_line_data::LineOrientation get_line_orientation(Orientation orientation,
		                                                                 Transport_line_data* up,
		                                                                 Transport_line_data* right,
		                                                                 Transport_line_data* down,
		                                                                 Transport_line_data* left);

		///
		/// \brief Attempts to find transport line at world_x, world_y
		/// \param callback Called for each Chunk_struct_layer found matching Transport_line_data at world_x, world_y
		static void get_line_struct_layer(game::World_data& world_data,
		                                  game::World_data::world_coord world_x,
		                                  game::World_data::world_coord world_y,
		                                  const std::function<void(game::Chunk_struct_layer&)>& callback);
		// ======================================================================
		// Game events
	private:
		/// Up, right, down, left
		using line_data_4_way = Transport_line_data*[4];

		///
		///	\brief Updates the orientation of current and neighboring transport lines 
		static void update_neighboring_orientation(const game::World_data& world_data,
		                                           const game::World_data::world_pair& world_coords,
		                                           Transport_line_data* t_center,
		                                           Transport_line_data* c_right,
		                                           Transport_line_data* b_center,
		                                           Transport_line_data* c_left,
		                                           Transport_line_data* center);

		using update_func = std::function<
			void(game::World_data& world_data,
			     int world_x,
			     int world_y,
			     float world_offset_x,
			     float world_offset_y,
			     game::Transport_line_segment::TerminationType termination_type)>;

		using update_side_only_func = std::function<
			void(game::World_data& world_data,
			     int world_x,
			     int world_y,
			     float world_offset_x,
			     float world_offset_y,
			     Orientation direction,
			     game::Transport_line_segment::TerminationType termination_type)>;

		///
		/// \brief Calls func or side_only_func depending on the line_orientation, provides parameters on how neighboring lines
		/// should be modified.
		/// \remark This does not move across logic chunks and may make the position negative
		/// \param func Called when line orientation is bending for updating provided line segment
		/// \param side_only_func Called when line orientation is straight for updating provided line segment 
		static void update_neighbor_lines(game::World_data& world_data,
		                                  int32_t world_x,
		                                  int32_t world_y,
		                                  Transport_line_data::LineOrientation line_orientation,
		                                  const update_func& func,
		                                  const update_side_only_func& side_only_func);

		static void update_termination_type(game::World_data& world_data,
		                                    const game::World_data::world_pair& world_coords,
		                                    Orientation orientation,
											line_data_4_way& line_data,
		                                    game::Transport_line_segment& line_segment,
		                                    int32_t& line_segment_world_x, int32_t& line_segment_world_y);
		/*
		 * Transport line grouping rules:
		 *
		 * < < < [1, 2, 3] - Direction [order];
		 * Line ahead:
		 *		- Extends length of transport line segment
		 *
		 * < < < [3, 2, 1]
		 * Line behind:
		 *		- Moves head of transport segment, shift leading item 1 tile back
		 *		
		 * < < < [1, 3, 2]
		 * Line ahead and behind:
		 *		- Behaves as line ahead
		 */
		///
		/// \brief Groups transport segments
		/// Sets the transport segment grouped / newly created with in tile_layer and returns it
		static Transport_line_data* group_transport_segment(Orientation orientation,
															game::Chunk_tile_layer& tile_layer,
		                                                    line_data_4_way& line_data);

		///
		/// \brief Attempts to find and group with a transport ahead in the provided orientation 
		/// \param orientation
		/// \param line_data The 4 neighboring transport segments in N,E,S,W order
		/// \return -1 if failed to group, otherwise line segment index
		static int group_transport_segment_ahead(Orientation orientation,
		                                         line_data_4_way& line_data);
	public:
		void on_build(game::World_data& world_data,
		              const game::World_data::world_pair& world_coords,
		              game::Chunk_tile_layer& tile_layer,
		              uint16_t frame,
		              Orientation orientation) const override;

		void on_neighbor_update(game::World_data& world_data,
		                        const game::World_data::world_pair& emit_world_coords,
		                        const game::World_data::world_pair& receive_world_coords,
		                        Orientation emit_orientation) const override;

		void on_remove(game::World_data& world_data,
		               const game::World_data::world_pair& world_coords,
		               game::Chunk_tile_layer& tile_layer) const override;

		J_NODISCARD std::pair<uint16_t, uint16_t> map_placement_orientation(Orientation orientation,
		                                                                    game::World_data& world_data,
		                                                                    const game::World_data::world_pair& world_coords)
		const override;


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

		void on_r_show_gui(game::Player_data& /*player_data*/, game::Chunk_tile_layer* /*tile_layer*/) const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
