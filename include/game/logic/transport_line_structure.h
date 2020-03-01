//
// Created on 2/21/2020.
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
//

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H

#include "data/prototype/item/item.h"

#include <deque>

namespace jactorio::game
{
	/**
	 * Item on a transport line
	 * Tile distance from next item or end of transport line, pointer to item
	 */
	using transport_line_item = std::pair<float, data::Item*>;

	/**
	 * 	Stores a collection of items heading in one direction
	 * 	When stored in a chunk, this is associated with a transport_line prototype
	 */
	struct Transport_line_segment
	{
		enum class moveDir
		{
			up,
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
		static constexpr float bend_left_l_reduction = 0.7;
		static constexpr float bend_left_r_reduction = 0.3;

		static constexpr float bend_right_l_reduction = 0.3;
		static constexpr float bend_right_r_reduction = 0.7;

		enum class terminationType
		{
			straight,
			// Left length -0.7
			// Right length -0.3
			bend_left,
			// Left length -0.3
			// Right length -0.7
			bend_right
		};

		Transport_line_segment(moveDir direction, terminationType termination_type, uint8_t segment_length)
			: direction(direction), termination_type(termination_type), segment_length(segment_length) {
		}

		Transport_line_segment(moveDir direction, terminationType termination_type,
							   Transport_line_segment* target_segment, uint8_t segment_length)
			: direction(direction), termination_type(termination_type), target_segment(target_segment), segment_length(segment_length) {
		}

		// Each item's distance (in tiles) to the next item or the end of this transport line segment
		// Items closest to the end are stored at the front
		// See FFF 176 https://factorio.com/blog/post/fff-176

		// <distance, spritemap_id for item>
		std::deque<transport_line_item> left;
		std::deque<transport_line_item> right;

		// Direction items in this segment travel in
		moveDir direction;
		// How the belt terminates (bends left, right, straight)
		terminationType termination_type;

		// Segment this transport line feeds into
		Transport_line_segment* target_segment = nullptr;

		// Index to the next item still with space to move
		uint16_t l_index = 0;
		uint16_t r_index = 0;

		// Length of this segment in tiles
		uint8_t segment_length;


		/**
		 * Returns true if an item can be inserted into this transport line
		 * @param start_offset Item offset from the start of transport line in tiles
		 * @return
		 */
		J_NODISCARD bool can_insert(bool left_side, float start_offset);
	};
}


#endif //JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
