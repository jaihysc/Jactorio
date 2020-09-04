// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/transport_segment.h"

#include <decimal.h>

#include "game/logic/transport_line_controller.h"

using namespace jactorio;

bool game::TransportLane::IsActive() const {
    return !(lane.empty() || index >= lane.size());
}

bool game::TransportLane::CanInsert(data::LineDistT start_offset, const IntOffsetT item_offset) {
    start_offset += data::LineDistT(item_offset);
    assert(start_offset.getAsDouble() >= 0);

    data::LineDistT offset(0);

    // Check if start_offset already has an item
    for (const auto& item : lane) {
        // Item is not compressed with the previous item
        if (item.dist > data::LineDistT(kItemSpacing)) {
            //  OFFSET item_spacing               item_spacing   OFFSET + item.first
            //     | -------------- |   GAP FOR ITEM   | ------------ |
            if (data::LineDistT(kItemSpacing) + offset <= start_offset &&
                start_offset <= offset + item.dist - data::LineDistT(kItemSpacing)) {

                return true;
            }
        }

        offset += item.dist;

        // Offset past start_offset, not possible to be true past this
        if (offset > start_offset)
            return false;
    }

    // Account for the item width of the last item if not the firs item
    if (!lane.empty())
        offset += data::LineDistT(kItemSpacing);

    return offset <= start_offset;
}

void game::TransportLane::AppendItem(FloatOffsetT offset, const data::Item& item) {
    // A minimum distance of item_spacing is maintained between items (AFTER the initial item)
    if (offset < kItemSpacing && !lane.empty())
        offset = kItemSpacing;

    lane.emplace_back(data::LineDistT(offset), &item);
    backItemDistance += data::LineDistT{offset};
}

void game::TransportLane::InsertItem(FloatOffsetT offset, const data::Item& item, const IntOffsetT item_offset) {
    offset += item_offset;
    assert(offset >= 0);

    data::LineDistT target_offset{offset}; // Location where item will be inserted
    data::LineDistT counter_offset;        // Running tally of offset from beginning

    std::deque<TransportLineItem>::iterator it;
    for (auto i = 0u; i < lane.size(); ++i) {
        counter_offset += lane[i].dist;

        // Ends at location where item should be inserted
        // Target: 0.4
        // 0.3   0.2(0.5)
        //     ^ Ends here
        if (counter_offset > target_offset) {
            it = lane.begin() + i;

            // Modify offset of next item to be relative to what will be the newly inserted item
            counter_offset -= lane[i].dist; // Back to distance to previous item

            // Modify insert offset to be relative to previous item, and following item to be relative to newly inserted
            // item
            target_offset -= counter_offset;
            lane[i].dist -= target_offset;
            goto loop_exit;
        }
    }
    // Failed to find a greater item, insert at back
    backItemDistance = target_offset;

    it = lane.end();
    target_offset -= counter_offset;

loop_exit:
    assert(target_offset.getAsDouble() >= 0);
    lane.emplace(it, target_offset, &item);
}

bool game::TransportLane::TryInsertItem(const FloatOffsetT offset,
                                        const data::Item& item,
                                        const IntOffsetT item_offset) {
    if (!CanInsert(data::LineDistT(offset), item_offset))
        return false;

    // Reenable transport segment if disabled
    if (!IsActive())
        index = 0;

    InsertItem(offset, item, item_offset);
    return true;
}

std::pair<size_t, game::TransportLineItem> game::TransportLane::GetItem(const FloatOffsetT offset,
                                                                        const FloatOffsetT epsilon) const {
    const data::LineDistT lower_bound{offset - epsilon};
    const data::LineDistT upper_bound{offset + epsilon};

    data::LineDistT offset_counter{0};

    // Iterate past offset - epsilon
    size_t iteration = 0;
    for (const auto& item_pair : lane) {
        offset_counter += item_pair.dist;

        if (offset_counter >= lower_bound) {
            if (offset_counter <= upper_bound) {
                return {iteration, item_pair};
            }
            // Past upper  bounds
            return {0, {}};
        }

        ++iteration;
    }
    // Failed to meet lower bounds
    return {0, {}};
}

const data::Item* game::TransportLane::TryPopItem(const FloatOffsetT offset, const FloatOffsetT epsilon) {
    const auto result = GetItem(offset, epsilon);

    if (result.second.item == nullptr)
        return nullptr;

    const auto iteration = result.first;
    const auto item_pair = result.second;


    if (iteration + 1 < lane.size()) {
        lane[iteration + 1].dist += item_pair.dist;
    }

    lane.erase(lane.begin() + iteration);
    return item_pair.item.Get();
}

// ======================================================================

bool game::TransportSegment::CanInsert(const bool left_side, const data::LineDistT& start_offset) {
    return left_side ? left.CanInsert(start_offset, 0) : right.CanInsert(start_offset, 0);
}

bool game::TransportSegment::IsActive(const bool left_side) const {
    return left_side ? left.IsActive() : right.IsActive();
}


// Side only deductions are applied differently whether the segment is a target, or the being the source to a target
// E.g:  v Calculating from here
//       v
//      -->  -->
//       ^    ^
//       ^    Target
//       Segment


void ApplyTerminationDeductionL(const game::TransportSegment::TerminationType termination_type,
                                data::LineDistT& offset) {
    switch (termination_type) {
        // Feeding into another belt also needs to be deducted to feed at the right offset on the target belt
    case game::TransportSegment::TerminationType::left_only:
    case game::TransportSegment::TerminationType::bend_left:
        offset -= data::LineDistT(game::kBendLeftLReduction);
        break;

    case game::TransportSegment::TerminationType::right_only:
    case game::TransportSegment::TerminationType::bend_right:
        offset -= data::LineDistT(game::kBendRightLReduction);
        break;

    case game::TransportSegment::TerminationType::straight:
        break;
    }
}

void ApplyTargetTerminationDeductionL(const game::TransportSegment::TerminationType termination_type,
                                      data::LineDistT& offset) {
    switch (termination_type) {
    case game::TransportSegment::TerminationType::bend_left:
        offset -= data::LineDistT(game::kBendLeftLReduction);
        break;

    case game::TransportSegment::TerminationType::bend_right:
        offset -= data::LineDistT(game::kBendRightLReduction);
        break;

    case game::TransportSegment::TerminationType::left_only:
    case game::TransportSegment::TerminationType::right_only:
        offset -= data::LineDistT(game::kTargetSideOnlyReduction);
        break;

    case game::TransportSegment::TerminationType::straight:
        break;
    }
}


void ApplyTerminationDeductionR(const game::TransportSegment::TerminationType termination_type,
                                data::LineDistT& offset) {
    switch (termination_type) {
    case game::TransportSegment::TerminationType::left_only:
    case game::TransportSegment::TerminationType::bend_left:
        offset -= data::LineDistT(game::kBendLeftRReduction);
        break;

    case game::TransportSegment::TerminationType::right_only:
    case game::TransportSegment::TerminationType::bend_right:
        offset -= data::LineDistT(game::kBendRightRReduction);
        break;

    case game::TransportSegment::TerminationType::straight:
        break;
    }
}

void ApplyTargetTerminationDeductionR(const game::TransportSegment::TerminationType termination_type,
                                      data::LineDistT& offset) {
    switch (termination_type) {
    case game::TransportSegment::TerminationType::bend_left:
        offset -= data::LineDistT(game::kBendLeftRReduction);
        break;

    case game::TransportSegment::TerminationType::bend_right:
        offset -= data::LineDistT(game::kBendRightRReduction);
        break;

    case game::TransportSegment::TerminationType::left_only:
    case game::TransportSegment::TerminationType::right_only:
        offset -= data::LineDistT(game::kTargetSideOnlyReduction);
        break;

    case game::TransportSegment::TerminationType::straight:
        break;
    }
}

void game::TransportSegment::ApplyTerminationDeduction(const bool is_left,
                                                       const TerminationType segment_ttype,
                                                       const TerminationType target_segment_ttype,
                                                       data::LineDistT& offset) {
    if (is_left)
        ApplyLeftTerminationDeduction(segment_ttype, target_segment_ttype, offset);
    else
        ApplyRightTerminationDeduction(segment_ttype, target_segment_ttype, offset);
}

void game::TransportSegment::ApplyLeftTerminationDeduction(const TerminationType segment_ttype,
                                                           const TerminationType target_segment_ttype,
                                                           data::LineDistT& offset) {
    ApplyTerminationDeductionL(segment_ttype, offset);

    // Transition into right lane
    if (segment_ttype == TerminationType::right_only)
        ApplyTargetTerminationDeductionR(target_segment_ttype, offset);
    else
        ApplyTargetTerminationDeductionL(target_segment_ttype, offset);
}

void game::TransportSegment::ApplyRightTerminationDeduction(const TerminationType segment_ttype,
                                                            const TerminationType target_segment_ttype,
                                                            data::LineDistT& offset) {
    ApplyTerminationDeductionR(segment_ttype, offset);


    // Transition into left lane
    if (segment_ttype == TerminationType::left_only)
        ApplyTargetTerminationDeductionL(target_segment_ttype, offset);
    else
        ApplyTargetTerminationDeductionR(target_segment_ttype, offset);
}

void game::TransportSegment::AppendItem(const bool left_side, const FloatOffsetT offset, const data::Item& item) {
    left_side ? left.AppendItem(offset, item) : right.AppendItem(offset, item);
}

void game::TransportSegment::InsertItem(const bool left_side, const FloatOffsetT offset, const data::Item& item) {
    left_side ? left.InsertItem(offset, item, 0) : right.InsertItem(offset, item, 0);
}

bool game::TransportSegment::TryInsertItem(const bool left_side, const FloatOffsetT offset, const data::Item& item) {
    return left_side ? left.TryInsertItem(offset, item, 0) : right.TryInsertItem(offset, item, 0);
}

std::pair<size_t, game::TransportLineItem> game::TransportSegment::GetItem(const bool left_side,
                                                                           const FloatOffsetT offset,
                                                                           const FloatOffsetT epsilon) const {
    return left_side ? left.GetItem(offset, epsilon) : right.GetItem(offset, epsilon);
}

const data::Item* game::TransportSegment::TryPopItem(const bool left_side,
                                                     const FloatOffsetT offset,
                                                     const FloatOffsetT epsilon) {
    return left_side ? left.TryPopItem(offset, epsilon) : right.TryPopItem(offset, epsilon);
}

// With itemOffset applied

bool game::TransportSegment::CanInsertAbs(const bool left_side, const data::LineDistT& start_offset) {
    return left_side ? left.CanInsert(start_offset, itemOffset) : right.CanInsert(start_offset, itemOffset);
}

void game::TransportSegment::InsertItemAbs(const bool left_side, const FloatOffsetT offset, const data::Item& item) {
    left_side ? left.InsertItem(offset, item, itemOffset) : right.InsertItem(offset, item, itemOffset);
}

bool game::TransportSegment::TryInsertItemAbs(const bool left_side, const FloatOffsetT offset, const data::Item& item) {
    return left_side ? left.TryInsertItem(offset, item, itemOffset) : right.TryInsertItem(offset, item, itemOffset);
}

void game::TransportSegment::GetOffsetAbs(IntOffsetT& val) const {
    val -= itemOffset;
}

void game::TransportSegment::GetOffsetAbs(FloatOffsetT& val) const {
    val -= itemOffset;
}
