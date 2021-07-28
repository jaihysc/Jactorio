// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_struct.h"

#include <decimal.h>

using namespace jactorio;

bool game::ConveyorLane::IsActive() const {
    return !(lane.empty() || index >= lane.size());
}

bool game::ConveyorLane::CanInsert(proto::LineDistT start_offset, const IntOffsetT item_offset) {
    start_offset += proto::LineDistT(item_offset);
    assert(start_offset.getAsDouble() >= 0);

    proto::LineDistT offset(0);

    // Check if start_offset already has an item
    for (const auto& item : lane) {
        // Item is not compressed with the previous item
        if (item.dist > proto::LineDistT(ConveyorProp::kItemSpacing)) {
            //  OFFSET item_spacing               item_spacing   OFFSET + item.first
            //     | -------------- |   GAP FOR ITEM   | ------------ |
            if (proto::LineDistT(ConveyorProp::kItemSpacing) + offset <= start_offset &&
                start_offset <= offset + item.dist - proto::LineDistT(ConveyorProp::kItemSpacing)) {

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
        offset += proto::LineDistT(ConveyorProp::kItemSpacing);

    return offset <= start_offset;
}

void game::ConveyorLane::AppendItem(FloatOffsetT offset, const proto::Item& item) {
    // A minimum distance of item_spacing is maintained between items (AFTER the initial item)
    if (offset < ConveyorProp::kItemSpacing && !lane.empty())
        offset = ConveyorProp::kItemSpacing;

    lane.emplace_back(proto::LineDistT(offset), &item);
    backItemDistance += proto::LineDistT{offset};
}

void game::ConveyorLane::InsertItem(FloatOffsetT offset, const proto::Item& item, const IntOffsetT item_offset) {
    // TODO Why have item offset if it is just added to offset
    offset += item_offset;
    assert(offset >= 0);

    proto::LineDistT target_offset{offset}; // Location where item will be inserted
    proto::LineDistT counter_offset;        // Running tally of offset from beginning

    std::deque<ConveyorItem>::iterator it;
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

bool game::ConveyorLane::TryInsertItem(const FloatOffsetT offset,
                                       const proto::Item& item,
                                       const IntOffsetT item_offset) {
    if (!CanInsert(proto::LineDistT(offset), item_offset))
        return false;

    // Reenable conveyor segment if disabled
    if (!IsActive())
        index = 0;

    InsertItem(offset, item, item_offset);
    return true;
}

std::pair<size_t, game::ConveyorItem> game::ConveyorLane::GetItem(const FloatOffsetT offset,
                                                                  const FloatOffsetT epsilon) const {
    const proto::LineDistT lower_bound{offset - epsilon};
    const proto::LineDistT upper_bound{offset + epsilon};

    proto::LineDistT offset_counter{0};

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

void game::ConveyorLane::RemoveItem(const std::size_t index) {
    assert(index < lane.size());

    // Adjust distance if has item behind
    if (lane.size() > index + 1) {
        lane[index + 1].dist += lane[index].dist;
    }
    lane.erase(lane.begin() + index);
}

const proto::Item* game::ConveyorLane::TryPopItem(const FloatOffsetT offset, const FloatOffsetT epsilon) {
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

bool game::ConveyorStruct::CanInsert(const bool left_side, const proto::LineDistT& start_offset) {
    return left_side ? left.CanInsert(start_offset, 0) : right.CanInsert(start_offset, 0);
}

bool game::ConveyorStruct::IsActive(const bool left_side) const {
    return left_side ? left.IsActive() : right.IsActive();
}


// Side only deductions are applied differently whether the segment is a target, or the being the source to a target
// E.g:  v Calculating from here
//       v
//      -->  -->
//       ^    ^
//       ^    Target
//       Segment


proto::LineDistT GetTerminationDeductionL(const game::ConveyorStruct::TerminationType termination_type) {
    switch (termination_type) {
        // Feeding into another belt also needs to be deducted to feed at the right offset on the target belt
    case game::ConveyorStruct::TerminationType::left_only:
    case game::ConveyorStruct::TerminationType::bend_left:
        return proto::LineDistT(game::ConveyorProp::kBendLeftLReduction);

    case game::ConveyorStruct::TerminationType::right_only:
    case game::ConveyorStruct::TerminationType::bend_right:
        return proto::LineDistT(game::ConveyorProp::kBendRightLReduction);

    default:
        return proto::LineDistT(0);
    }
}

proto::LineDistT GetTargetDeductionDeductionL(const game::ConveyorStruct::TerminationType termination_type) {
    switch (termination_type) {
    case game::ConveyorStruct::TerminationType::bend_left:
        return proto::LineDistT(game::ConveyorProp::kBendLeftLReduction);

    case game::ConveyorStruct::TerminationType::bend_right:
        return proto::LineDistT(game::ConveyorProp::kBendRightLReduction);

    case game::ConveyorStruct::TerminationType::left_only:
    case game::ConveyorStruct::TerminationType::right_only:
        return proto::LineDistT(game::ConveyorProp::kTargetSideOnlyReduction);

    default:
        return proto::LineDistT(0);
    }
}


proto::LineDistT GetTerminationDeductionR(const game::ConveyorStruct::TerminationType termination_type) {
    switch (termination_type) {
    case game::ConveyorStruct::TerminationType::left_only:
    case game::ConveyorStruct::TerminationType::bend_left:
        return proto::LineDistT(game::ConveyorProp::kBendLeftRReduction);

    case game::ConveyorStruct::TerminationType::right_only:
    case game::ConveyorStruct::TerminationType::bend_right:
        return proto::LineDistT(game::ConveyorProp::kBendRightRReduction);

    default:
        return proto::LineDistT(0);
    }
}

proto::LineDistT GetTargetDeductionDeductionR(const game::ConveyorStruct::TerminationType termination_type) {
    switch (termination_type) {
    case game::ConveyorStruct::TerminationType::bend_left:
        return proto::LineDistT(game::ConveyorProp::kBendLeftRReduction);

    case game::ConveyorStruct::TerminationType::bend_right:
        return proto::LineDistT(game::ConveyorProp::kBendRightRReduction);

    case game::ConveyorStruct::TerminationType::left_only:
    case game::ConveyorStruct::TerminationType::right_only:
        return proto::LineDistT(game::ConveyorProp::kTargetSideOnlyReduction);

    default:
        return proto::LineDistT(0);
    }
}

proto::LineDistT game::ConveyorStruct::GetTerminationDeduction(const bool is_left,
                                                               const TerminationType segment_ttype,
                                                               const TerminationType target_segment_ttype) {
    if (is_left)
        return GetLeftTerminationDeduction(segment_ttype, target_segment_ttype);
    return GetRightTerminationDeduction(segment_ttype, target_segment_ttype);
}

proto::LineDistT game::ConveyorStruct::GetLeftTerminationDeduction(const TerminationType segment_ttype,
                                                                   const TerminationType target_segment_ttype) {
    const auto deduction = GetTerminationDeductionL(segment_ttype);

    // Transition into right lane
    if (segment_ttype == TerminationType::right_only)
        return deduction + GetTargetDeductionDeductionR(target_segment_ttype);
    return deduction + GetTargetDeductionDeductionL(target_segment_ttype);
}

proto::LineDistT game::ConveyorStruct::GetRightTerminationDeduction(const TerminationType segment_ttype,
                                                                    const TerminationType target_segment_ttype) {
    const auto deduction = GetTerminationDeductionR(segment_ttype);

    // Transition into left lane
    if (segment_ttype == TerminationType::left_only)
        return deduction + GetTargetDeductionDeductionL(target_segment_ttype);
    return deduction + GetTargetDeductionDeductionR(target_segment_ttype);
}

void game::ConveyorStruct::AppendItem(const bool left_side, const FloatOffsetT offset, const proto::Item& item) {
    left_side ? left.AppendItem(offset, item) : right.AppendItem(offset, item);
}

void game::ConveyorStruct::InsertItem(const bool left_side, const FloatOffsetT offset, const proto::Item& item) {
    left_side ? left.InsertItem(offset, item, 0) : right.InsertItem(offset, item, 0);
}

bool game::ConveyorStruct::TryInsertItem(const bool left_side, const FloatOffsetT offset, const proto::Item& item) {
    return left_side ? left.TryInsertItem(offset, item, 0) : right.TryInsertItem(offset, item, 0);
}

std::pair<size_t, game::ConveyorItem> game::ConveyorStruct::GetItem(const bool left_side,
                                                                    const FloatOffsetT offset,
                                                                    const FloatOffsetT epsilon) const {
    return left_side ? left.GetItem(offset, epsilon) : right.GetItem(offset, epsilon);
}

void game::ConveyorStruct::RemoveItem(const bool left_side, const std::size_t index) {
    left_side ? left.RemoveItem(index) : right.RemoveItem(index);
}

const proto::Item* game::ConveyorStruct::TryPopItem(const bool left_side,
                                                    const FloatOffsetT offset,
                                                    const FloatOffsetT epsilon) {
    return left_side ? left.TryPopItem(offset, epsilon) : right.TryPopItem(offset, epsilon);
}

// With itemOffset applied

bool game::ConveyorStruct::CanInsertAbs(const bool left_side, const proto::LineDistT& start_offset) {
    return left_side ? left.CanInsert(start_offset, headOffset) : right.CanInsert(start_offset, headOffset);
}

void game::ConveyorStruct::InsertItemAbs(const bool left_side, const FloatOffsetT offset, const proto::Item& item) {
    left_side ? left.InsertItem(offset, item, headOffset) : right.InsertItem(offset, item, headOffset);
}

bool game::ConveyorStruct::TryInsertItemAbs(const bool left_side, const FloatOffsetT offset, const proto::Item& item) {
    return left_side ? left.TryInsertItem(offset, item, headOffset) : right.TryInsertItem(offset, item, headOffset);
}

void game::ConveyorStruct::GetOffsetAbs(IntOffsetT& val) const {
    val -= headOffset;
}

void game::ConveyorStruct::GetOffsetAbs(FloatOffsetT& val) const {
    val -= headOffset;
}
