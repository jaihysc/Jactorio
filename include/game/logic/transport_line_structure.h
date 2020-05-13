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
	// Each item's distance (in tiles) to the next item or the end of this transport line segment
	// Items closest to the end are stored at the front
	// See FFF 176 https://factorio.com/blog/post/fff-176

	///
	/// \brief Item on a transport line
	/// Tile distance from next item or end of transport line, pointer to item
	using TransportLineItem = std::pair<TransportLineOffset, const data::Item*>;

	///
	/// \brief One side of a transport line
	struct TransportLane
	{
		using ItemOffsetT = int16_t;
		using InsertOffsetT = double;

		///
		/// \return true if left size is not empty and has a valid index
		J_NODISCARD bool IsActive() const;

		///
		/// \param start_offset Item offset from the start of transport line in tiles
		/// \return true if an item can be inserted into this transport line
		J_NODISCARD bool CanInsert(TransportLineOffset start_offset, ItemOffsetT item_offset);

		// Item insertion 

		///
		/// \brief Appends item onto the specified side of a belt behind the last item
		/// \param offset Number of tiles to offset from previous item or the end of the transport line segment when there are no items
		void AppendItem(InsertOffsetT offset, const data::Item* item);

		///
		/// \brief Inserts the item onto the specified belt side at the offset
		/// from the beginning of the transport line
		/// \param offset Distance from beginning of transport line
		void InsertItem(InsertOffsetT offset, const data::Item* item, ItemOffsetT item_offset);

		///
		/// \brief Attempts to insert the item onto the specified belt side at the offset
		/// from the beginning of the transport line
		/// \param offset Distance from beginning of transport line
		/// \return false if unsuccessful
		bool TryInsertItem(InsertOffsetT offset, const data::Item* item, ItemOffsetT item_offset);

		// ======================================================================

		/// <distance, spritemap_id for item>
		std::deque<TransportLineItem> lane;

		/// Index of active item within lane
		uint16_t index = 0;

		/// Distance to the last item from beginning of transport line
		/// Avoids having to iterate through and count each time
		TransportLineOffset backItemDistance;

		/// Visibility of items on lane
		bool visible = true;
	};

	///
	/// \brief Stores a collection of items heading in one direction
	struct TransportSegment final : data::UniqueDataBase
	{
	private:
		using SegmentLengthT = uint8_t;
		using ItemOffsetT = TransportLane::ItemOffsetT;

	public:
		using InsertOffsetT = TransportLane::InsertOffsetT;

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


		TransportSegment(const data::Orientation direction, const TerminationType termination_type,
		                 const uint8_t segment_length)
			: direction(direction), terminationType(termination_type), length(segment_length) {
		}

		TransportSegment(const data::Orientation direction, const TerminationType termination_type,
		                 TransportSegment* target_segment, const uint8_t segment_length)
			: direction(direction), targetSegment(target_segment),
			  terminationType(termination_type),
			  length(segment_length) {
		}

		TransportLane left;
		TransportLane right;

		/// Direction items in this segment travel in
		data::Orientation direction;

		/// Segment this transport line feeds into
		TransportSegment* targetSegment = nullptr;

		/// How the belt terminates (bends left, right, straight) (Single belt side)
		TerminationType terminationType;

		/// Length of this segment in tiles
		SegmentLengthT length;

		// TODO remove this if a use does not come up
		/// \brief Currently Unused. Offset applied for TransportLineData::lineSegmentIndex
		///
		/// When this segment is extended from the head, offset increments 1.
		/// When the segment is shortened from the head, offset decrements 1.
		/// No effect when extended or shortened from the tail.
		/// <br>
		/// The offset ensures that all entities which stores a segment index at a tile inserts at the correct location
		/// when the segment is extended or shortened
		ItemOffsetT itemOffset = 0;

		// ======================================================================

		///
		/// \param start_offset Item offset from the start of transport line in tiles
		/// \return true if an item can be inserted into this transport line
		J_NODISCARD bool CanInsert(bool left_side, const TransportLineOffset& start_offset);

		///
		/// \return true if left size is not empty and has a valid index
		J_NODISCARD bool IsActive(bool left_side) const;

		// Item insertion 

		///
		/// \brief Appends item onto the specified side of a belt behind the last item
		/// \param offset Number of tiles to offset from previous item or the end of the transport line segment when there are no items
		void AppendItem(bool left_side, InsertOffsetT offset, const data::Item* item);

		///
		/// \brief Inserts the item onto the specified belt side at the offset from the beginning of the transport line
		/// \param offset Distance from beginning of transport line
		void InsertItem(bool left_side, InsertOffsetT offset, const data::Item* item);

		///
		/// \brief Attempts to insert the item onto the specified belt side at the offset from the beginning of the transport line
		/// \param offset Distance from beginning of transport line
		/// \return false if unsuccessful
		bool TryInsertItem(bool left_side, InsertOffsetT offset, const data::Item* item);


		J_NODISCARD TransportLane& GetSide(const bool left_side) {
			return left_side ? left : right;
		}

		///
		/// \brief Adjusts provided value such that InsertItem(, offset, ) is at the correct location
		///
		/// This is because segments are numbered from 0 to n from the head of the line, storing the segmentIndex
		/// directly leads to insertion at the incorrect locations when the segment length is changed.
		/// Thus, an adjustment is applied to all offsets inserting into the segment such that the same offset
		/// results in the same location regardless of the segment length.
		/// \param val Distance from beginning of transport segment
		void AdjustInsertionOffset(InsertOffsetT& val) const;
	};
}


#endif //JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
