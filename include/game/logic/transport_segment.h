// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_SEGMENT_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_SEGMENT_H
#pragma once

#include <deque>

#include "data/cereal/support/decimal.h"
#include "game/logic/transport_line_controller.h"
#include "proto/item.h"
#include "proto/type.h"

#include <cereal/types/deque.hpp>

namespace jactorio::game
{
    // Each item's distance (in tiles) to the next item or the end of this transport line segment
    // Items closest to the end are stored at the front
    // See FFF 176 https://factorio.com/blog/post/fff-176

    ///
    /// Item on a transport line
    /// Tile distance from next item or end of transport line, pointer to item
    struct TransportLineItem
    {
        TransportLineItem() = default;

        TransportLineItem(const data::LineDistT& line_dist, const data::Item* item_ptr)
            : dist(line_dist), item(item_ptr) {}

        data::LineDistT dist;
        data::SerialProtoPtr<const data::Item> item;


        CEREAL_SERIALIZE(archive) {
            archive(dist, item);
        }
    };

    ///
    /// One side of a transport line
    struct TransportLane
    {
        using IntOffsetT   = int16_t;
        using FloatOffsetT = double;

        ///
        /// \return true if left size is not empty and has a valid index
        J_NODISCARD bool IsActive() const;

        ///
        /// \param start_offset Item offset from the start of transport line in tiles
        /// \return true if an item can be inserted into this transport line
        J_NODISCARD bool CanInsert(data::LineDistT start_offset, IntOffsetT item_offset);

        // Item insertion
        // See TransportSegment for function documentation

        void AppendItem(FloatOffsetT offset, const data::Item& item);

        void InsertItem(FloatOffsetT offset, const data::Item& item, IntOffsetT item_offset);

        bool TryInsertItem(FloatOffsetT offset, const data::Item& item, IntOffsetT item_offset);

        J_NODISCARD std::pair<size_t, TransportLineItem> GetItem(FloatOffsetT offset,
                                                                 FloatOffsetT epsilon = kItemWidth / 2) const;
        const data::Item* TryPopItem(FloatOffsetT offset, FloatOffsetT epsilon = kItemWidth / 2);

        // ======================================================================

        std::deque<TransportLineItem> lane;

        /// Index of active item within lane
        uint16_t index = 0;

        /// Distance to the last item from beginning of transport line
        /// Avoids having to iterate through and count each time
        data::LineDistT backItemDistance;

        /// Visibility of items on lane
        bool visible = true;


        CEREAL_SERIALIZE(archive) {
            archive(lane, index, backItemDistance, visible);
        }
    };

    ///
    /// Stores a collection of items heading in one direction
    class TransportSegment
    {
        using SegmentLengthT = uint8_t;

    public:
        using IntOffsetT   = TransportLane::IntOffsetT;
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


        TransportSegment(const data::Orientation direction,
                         const TerminationType termination_type,
                         const uint8_t segment_length)
            : direction(direction), terminationType(termination_type), length(segment_length) {}

        TransportSegment(const data::Orientation direction,
                         const TerminationType termination_type,
                         TransportSegment* target_segment,
                         const uint8_t segment_length)
            : direction(direction),
              terminationType(termination_type),
              length(segment_length),
              targetSegment(target_segment) {}


        // ======================================================================


        J_NODISCARD TransportLane& GetSide(const bool left_side) {
            return left_side ? left : right;
        }

        ///
        /// \param start_offset Item offset from the start of transport line in tiles
        /// \return true if an item can be inserted into this transport line
        J_NODISCARD bool CanInsert(bool left_side, const data::LineDistT& start_offset);

        ///
        /// \return true if left size is not empty and has a valid index
        J_NODISCARD bool IsActive(bool left_side) const;

        ///
        /// Deducts tile length offset to get offset based on lane length
        /// \param target_segment_ttype If deducting termination of current segment's target is unnecessary, use
        /// straight
        ///
        /// Since the length of segments is the tile length, it must be deducted to get the lane length to accurately
        /// insert, remove and any other operation on segments
        static void ApplyTerminationDeduction(bool is_left,
                                              TerminationType segment_ttype,
                                              TerminationType target_segment_ttype,
                                              data::LineDistT& offset);
        template <bool IsLeftLane>
        static void ApplyTerminationDeduction(TerminationType segment_ttype,
                                              TerminationType target_segment_ttype,
                                              data::LineDistT& offset);

        static void ApplyLeftTerminationDeduction(TerminationType segment_ttype,
                                                  TerminationType target_segment_ttype,
                                                  data::LineDistT& offset);
        static void ApplyRightTerminationDeduction(TerminationType segment_ttype,
                                                   TerminationType target_segment_ttype,
                                                   data::LineDistT& offset);

        // Item insertion

        ///
        /// Appends item onto the specified side of a belt behind the last item
        /// \param offset Number of tiles to offset from previous item or the end of the transport line segment when
        /// there are no items
        void AppendItem(bool left_side, FloatOffsetT offset, const data::Item& item);

        ///
        /// Inserts the item onto the specified belt side at the offset from the beginning of the transport line
        /// \param offset Distance from beginning of transport line
        void InsertItem(bool left_side, FloatOffsetT offset, const data::Item& item);

        ///
        /// Attempts to insert the item onto the specified belt side at the offset from the beginning of the
        /// transport line \param offset Distance from beginning of transport line \return false if unsuccessful
        bool TryInsertItem(bool left_side, FloatOffsetT offset, const data::Item& item);

        ///
        /// Finds item at offset within epsilon upper and lower bounds inclusive, <deque index,
        /// TransportLineItem> \return .second.second is nullptr if no items were found
        J_NODISCARD std::pair<size_t, TransportLineItem> GetItem(bool left_side,
                                                                 FloatOffsetT offset,
                                                                 FloatOffsetT epsilon = kItemWidth / 2) const;

        ///
        /// Finds and removes item at offset within epsilon inclusive
        /// \return nullptr if no items were found
        const data::Item* TryPopItem(bool left_side, FloatOffsetT offset, FloatOffsetT epsilon = kItemWidth / 2);


        // Item insertion with itemOffset

        // Methods with Abs suffix is "absolute"
        // meaning the same offset from beginning of transport line will reference in the same world location
        // regardless of variable segment length or head position

        J_NODISCARD bool CanInsertAbs(bool left_side, const data::LineDistT& start_offset);

        void InsertItemAbs(bool left_side, FloatOffsetT offset, const data::Item& item);

        bool TryInsertItemAbs(bool left_side, FloatOffsetT offset, const data::Item& item);


        ///
        /// Adjusts provided value such that InsertItem(, offset, ) is at the correct location
        ///
        /// This is because segments are numbered from 0 to n from the head of the line, storing the segmentIndex
        /// directly leads to insertion at the incorrect locations when the segment length is changed.
        /// Thus, an adjustment is applied to all offsets inserting into the segment such that the same offset
        /// results in the same location regardless of the segment length.
        /// \param val Distance from beginning of transport segment
        void GetOffsetAbs(IntOffsetT& val) const;
        void GetOffsetAbs(FloatOffsetT& val) const;


        // ======================================================================


        /// Direction items in this segment travel in
        data::Orientation direction;

        /// How the belt terminates (bends left, right, straight) (Single belt side)
        TerminationType terminationType;

        /// Length of this segment in tiles
        SegmentLengthT length;


        TransportLane left;
        TransportLane right;


        /// Offset applied for TransportBeltData::lineSegmentIndex
        ///
        /// When this segment is extended from the head, offset increments 1.
        /// When the segment is shortened from the head, offset decrements 1.
        /// No effect when extended or shortened from the tail.
        ///
        /// The offset ensures that all entities which stores a segment index tile inserts at the same location
        /// when the segment is extended or shortened, used in methods with a Abs suffix
        IntOffsetT itemOffset = 0;

        /// If this segment terminates side only, this is the offset from the beginning of target segment to insert at
        IntOffsetT targetInsertOffset = 0;

        /// Segment this transport line feeds into
        TransportSegment* targetSegment = nullptr;


        CEREAL_SERIALIZE(archive) {
            archive(direction, terminationType, length, left, right, itemOffset, targetInsertOffset);
        }

        CEREAL_LOAD_CONSTRUCT(archive, construct, TransportSegment) {
            data::Orientation line_dir;
            TerminationType term_type;
            SegmentLengthT seg_length;

            archive(line_dir, term_type, seg_length);
            construct(line_dir, term_type, seg_length);

            archive(construct->left, construct->right, construct->itemOffset, construct->targetInsertOffset);
        }
    };

    template <bool IsLeftLane>
    void TransportSegment::ApplyTerminationDeduction(const TerminationType segment_ttype,
                                                     const TerminationType target_segment_ttype,
                                                     data::LineDistT& offset) {
        if constexpr (IsLeftLane) {
            ApplyLeftTerminationDeduction(segment_ttype, target_segment_ttype, offset);
        }
        else {
            ApplyRightTerminationDeduction(segment_ttype, target_segment_ttype, offset);
        }
    }
} // namespace jactorio::game


#endif // JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_SEGMENT_H
