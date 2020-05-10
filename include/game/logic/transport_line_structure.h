// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
#pragma once

#include <deque>

#include "core/data_type.h"
#include "data/prototype/orientation.h"
#include "data/prototype/item/item.h"
#include "game/logic/transport_line_controller.h"

namespace jactorio::game
{
	///
	/// \brief Item on a transport line
	/// Tile distance from next item or end of transport line, pointer to item
	using TransportLineItem = std::pair<TransportLineOffset, const data::Item*>;

	///
	/// \brief Stores a collection of items heading in one direction
	struct TransportLineSegment : data::UniqueDataBase
	{
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
		static constexpr double kBendLeftLReduction = 0.7;
		static constexpr double kBendLeftRReduction = 0.3;

		static constexpr double kBendRightLReduction = 0.3;
		static constexpr double kBendRightRReduction = 0.7;

		enum class TerminationType
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


		TransportLineSegment(const data::Orientation direction, const TerminationType termination_type,
		                     const uint8_t segment_length)
			: direction(direction), terminationType(termination_type), length(segment_length) {
		}

		TransportLineSegment(const data::Orientation direction, const TerminationType termination_type,
		                     TransportLineSegment* target_segment, const uint8_t segment_length)
			: direction(direction), targetSegment(target_segment),
			  terminationType(termination_type),
			  length(segment_length) {
		}

		// Each item's distance (in tiles) to the next item or the end of this transport line segment
		// Items closest to the end are stored at the front
		// See FFF 176 https://factorio.com/blog/post/fff-176

		/// <distance, spritemap_id for item>
		std::deque<TransportLineItem> left;
		std::deque<TransportLineItem> right;

		/// Are the items on this transport line visible?
		bool itemVisible = true;

		/// Direction items in this segment travel in
		data::Orientation direction;

		/// Segment this transport line feeds into
		TransportLineSegment* targetSegment = nullptr;

		/// How the belt terminates (bends left, right, straight) (Single belt side)
		TerminationType terminationType;

		/// Index to the next item still with space to move
		uint16_t lIndex = 0;
		uint16_t rIndex = 0;

		/// Distance to the last item from beginning of transport line
		/// Avoids having to iterate through and count each time
		TransportLineOffset lBackItemDistance;
		TransportLineOffset rBackItemDistance;


		/// Length of this segment in tiles
		uint8_t length;

		// ======================================================================

		///
		/// Returns true if an item can be inserted into this transport line
		/// \param start_offset Item offset from the start of transport line in tiles
		/// \return
		J_NODISCARD bool CanInsert(bool left_side, const TransportLineOffset& start_offset);


		///
		/// \return true if left size is not empty and has a valid index
		J_NODISCARD bool IsActiveLeft() const {
			return !(left.empty() || lIndex >= left.size());
		}

		///
		/// \return  true if right side is not empty and has a valid index
		J_NODISCARD bool IsActiveRight() const {
			return !(right.empty() || rIndex >= right.size());
		}


		// Item insertion

		///
		/// \brief Appends item onto the specified side of a belt behind the last item
		/// \param insert_left True to insert item on left size of belt
		/// \param offset Number of tiles to offset from previous item or the end of the transport line segment when there are no items
		void AppendItem(bool insert_left, double offset, const data::Item* item);

		///
		/// \brief Inserts the item onto the specified belt side at the offset
		/// from the beginning of the transport line
		/// \param insert_left True to insert item on left size of belt
		/// \param offset Distance from beginning of transport line
		void InsertItem(bool insert_left, double offset, const data::Item* item);

		///
		/// \brief Attempts to insert the item onto the specified belt side at the offset
		/// from the beginning of the transport line
		/// \param insert_left True to insert item on left size of belt
		/// \param offset Distance from beginning of transport line
		/// \return false if unsuccessful
		bool TryInsertItem(bool insert_left, double offset, const data::Item* item);
	};
}


#endif //JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
