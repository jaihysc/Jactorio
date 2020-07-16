// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
#pragma once

#include <deque>

#include "data/prototype/type.h"
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
		using IntOffsetT = int16_t;
		using FloatOffsetT = double;

		///
		/// \return true if left size is not empty and has a valid index
		J_NODISCARD bool IsActive() const;

		///
		/// \param start_offset Item offset from the start of transport line in tiles
		/// \return true if an item can be inserted into this transport line
		J_NODISCARD bool CanInsert(TransportLineOffset start_offset, IntOffsetT item_offset);

		// Item insertion 
		// See TransportSegment for function documentation

		void AppendItem(FloatOffsetT offset, const data::Item* item);

		void InsertItem(FloatOffsetT offset, const data::Item* item, IntOffsetT item_offset);

		bool TryInsertItem(FloatOffsetT offset, const data::Item* item, IntOffsetT item_offset);

		const data::Item* TryPopItem(FloatOffsetT offset, FloatOffsetT epsilon = kItemWidth / 2);

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
	struct TransportSegment
	{
	private:
		using SegmentLengthT = uint8_t;

	public:
		using IntOffsetT = TransportLane::IntOffsetT;
		using FloatOffsetT = TransportLane::FloatOffsetT;

		enum class TerminationType
		{
			straight = 0,
			bend_left,
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

		/// \brief Offset applied for TransportLineData::lineSegmentIndex
		///
		/// When this segment is extended from the head, offset increments 1.
		/// When the segment is shortened from the head, offset decrements 1.
		/// No effect when extended or shortened from the tail.
		/// <br>
		/// The offset ensures that all entities which stores a segment index tile inserts at the same location
		/// when the segment is extended or shortened, used in methods with a Abs suffix
		IntOffsetT itemOffset = 0;

		/// If this segment terminates side only, this is the offset from the beginning of target segment to insert at
		IntOffsetT targetInsertOffset = 0;

		// ======================================================================

		J_NODISCARD TransportLane& GetSide(const bool left_side) {
			return left_side ? left : right;
		}

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
		void AppendItem(bool left_side, FloatOffsetT offset, const data::Item* item);

		///
		/// \brief Inserts the item onto the specified belt side at the offset from the beginning of the transport line
		/// \param offset Distance from beginning of transport line
		void InsertItem(bool left_side, FloatOffsetT offset, const data::Item* item);

		///
		/// \brief Attempts to insert the item onto the specified belt side at the offset from the beginning of the transport line
		/// \param offset Distance from beginning of transport line
		/// \return false if unsuccessful
		bool TryInsertItem(bool left_side, FloatOffsetT offset, const data::Item* item);


		// Item insertion with itemOffset

		// Methods with Abs suffix is "absolute"
		// meaning the same offset from beginning of transport line will reference in the same world location
		// regardless of variable segment length or head position

		J_NODISCARD bool CanInsertAbs(bool left_side, const TransportLineOffset& start_offset);

		void InsertItemAbs(bool left_side, FloatOffsetT offset, const data::Item* item);

		bool TryInsertItemAbs(bool left_side, FloatOffsetT offset, const data::Item* item);

		///
		/// \brief Finds item at offset within epsilon inclusive
		/// \return nullptr if no items were found
		const data::Item* TryPopItemAbs(bool left_side, FloatOffsetT offset, FloatOffsetT epsilon = kItemWidth / 2);

		///
		/// \brief Adjusts provided value such that InsertItem(, offset, ) is at the correct location
		///
		/// This is because segments are numbered from 0 to n from the head of the line, storing the segmentIndex
		/// directly leads to insertion at the incorrect locations when the segment length is changed.
		/// Thus, an adjustment is applied to all offsets inserting into the segment such that the same offset
		/// results in the same location regardless of the segment length.
		/// \param val Distance from beginning of transport segment
		void GetOffsetAbs(IntOffsetT& val) const;
		void GetOffsetAbs(FloatOffsetT& val) const;
	};
}


#endif //JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_STRUCTURE_H
