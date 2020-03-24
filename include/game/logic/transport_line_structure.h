// 
// transport_line_structure.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/21/2020
// Last modified: 03/24/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
#pragma once

#include "data/prototype/item/item.h"
#include "core/data_type.h"

#include <deque>

namespace jactorio::game
{
	/**
	 * Item on a transport line
	 * Tile distance from next item or end of transport line, pointer to item
	 */
	using transport_line_item = std::pair<transport_line_offset, data::Item*>;

	///
	/// \brief Stores a collection of items heading in one direction
	struct Transport_line_segment : data::Unique_data_base
	{
		enum class moveDir
		{
			up = 0,
			right,
			down,
			left,
		};

		// When bending, the amounts below are reduced from the distance to the end of the next segment (see diagram below)
		/*
		 * === 0.7 ===
		 * =0.3=
		 *     ------------------------->
		 *     ^         *
		 *     |    -------------------->
		 *     |    ^    *
		 *     |    |    *
		 *     |    |    *
		 */
		static constexpr double bend_left_l_reduction = 0.7;
		static constexpr double bend_left_r_reduction = 0.3;

		static constexpr double bend_right_l_reduction = 0.3;
		static constexpr double bend_right_r_reduction = 0.7;

		enum class terminationType
		{
			straight = 0,
			// Left length -0.7
			// Right length -0.3
			bend_left,
			// Left length -0.3
			// Right length -0.7
			bend_right,

			// Left + Right lane -> Left lane
			left_only,
			// Left + Right Lane -> Right lane
			right_only
		};


		Transport_line_segment(const moveDir direction, const terminationType termination_type,
		                       const uint8_t segment_length)
			: direction(direction), termination_type(termination_type), segment_length(segment_length) {
		}

		Transport_line_segment(const moveDir direction, const terminationType termination_type,
		                       Transport_line_segment* target_segment, const uint8_t segment_length)
			: direction(direction), target_segment(target_segment),
			  termination_type(termination_type),
			  segment_length(segment_length) {
		}

		// Each item's distance (in tiles) to the next item or the end of this transport line segment
		// Items closest to the end are stored at the front
		// See FFF 176 https://factorio.com/blog/post/fff-176

		/// <distance, spritemap_id for item>
		std::deque<transport_line_item> left;
		std::deque<transport_line_item> right;

		/// Are the items on this transport line visible?
		bool item_visible = true;

		/// Direction items in this segment travel in
		moveDir direction;

		/// Segment this transport line feeds into
		Transport_line_segment* target_segment = nullptr;

		/// How the belt terminates (bends left, right, straight) (Single belt side)
		terminationType termination_type;

		/// Index to the next item still with space to move
		uint16_t l_index = 0;
		uint16_t r_index = 0;

		/// Distance to the last item from beginning of transport line
		/// Avoids having to iterate through and count each time
		transport_line_offset l_back_item_distance;
		transport_line_offset r_back_item_distance;


		/// Length of this segment in tiles
		uint8_t segment_length;

		///
		/// Returns true if an item can be inserted into this transport line
		/// \param start_offset Item offset from the start of transport line in tiles
		/// \return
		J_NODISCARD bool can_insert(bool left_side, const transport_line_offset& start_offset);


		///
		/// \return true if left size is not empty and has a valid index
		J_NODISCARD bool is_active_left() const {
			return !(left.empty() || l_index >= left.size());
		}

		///
		/// \return  true if right side is not empty and has a valid index
		J_NODISCARD bool is_active_right() const {
			return !(right.empty() || r_index >= right.size());
		}


		// Item insertion

		///
		/// \brief Appends item onto the specified side of a belt behind the last item
		/// \param insert_left True to insert item on left size of belt
		/// \param offset Number of tiles to offset from previous item or the end of the transport line segment when there are no items
		void append_item(bool insert_left, double offset, data::Item* item);

		///
		/// \brief Inserts the item onto the specified belt side at the offset
		/// from the beginning of the transport line
		/// \param insert_left True to insert item on left size of belt
		/// \param offset Distance from beginning of transport line
		void insert_item(bool insert_left, double offset, data::Item* item);
	};
}


#endif //JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
