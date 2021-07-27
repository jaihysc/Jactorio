// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_controller.h"

#include <cmath>

#include "game/logic/conveyor_struct.h"
#include "game/logic/conveyor_utility.h"
#include "game/world/world.h"
#include "proto/abstract/conveyor.h"
#include "proto/splitter.h"

using namespace jactorio;

/// Sets index to the next item with a distance greater than item_width and decrement it
/// If there is no item AND has_target_segment == false, index is set as size of conveyor
/// \return true if an item was decremented
J_NODISCARD static bool MoveNextItem(const proto::LineDistT& tiles_moved,
                                     std::deque<game::ConveyorItem>& line_side,
                                     uint16_t& index,
                                     const bool has_target_segment) {
    for (size_t i = SafeCast<size_t>(index) + 1; i < line_side.size(); ++i) {
        auto& i_item_offset = line_side[i].dist;
        if (i_item_offset > proto::LineDistT(game::ConveyorProp::kItemSpacing)) {
            // Found a valid item to decrement
            if (!has_target_segment) {
                // Always check every item from index 0 if there is a target segment as the previous item may have moved
                index = SafeCast<uint16_t>(i);
            }
            i_item_offset -= tiles_moved;

            return true;
        }
    }

    // Failed to find another item
    index = 0;

    /*
    // set to 1 past the index of the last item (stop the conveyor permanently) if there is no target segment
    if (!has_target_segment)
        index = line_side.size();
    else
        index = 0;
    */

    return false;
}

template <bool IsLeft>
void UpdateSide(const proto::LineDistT& tiles_moved, game::ConveyorStruct& segment) {
    using namespace jactorio;
    auto& side      = segment.GetSide(IsLeft);
    uint16_t& index = side.index;

    proto::LineDistT& offset             = side.lane[index].dist;
    proto::LineDistT& back_item_distance = side.backItemDistance;

    // Front item if index is 0
    if (index == 0) {
        // Front item does not need to be moved
        if (offset >= proto::LineDistT(0))
            return;

        if (segment.target) {
            game::ConveyorStruct& target_segment = *segment.target;
            // Offset to insert at target segment from head
            proto::LineDistT target_offset;
            {
                double length;
                switch (segment.terminationType) {
                    // Since segments terminating with side only can target the middle of a grouped segment, it uses
                    // its own member for head offset
                case game::ConveyorStruct::TerminationType::left_only:
                case game::ConveyorStruct::TerminationType::right_only:
                    // |   |   |   |
                    // 3   2   1   0
                    // targetOffset of 0: Length is 1
                    length = SafeCast<double>(1) + segment.sideInsertIndex;
                    break;

                default:
                    length = target_segment.length;
                    break;
                }
                target_offset = proto::LineDistT(length - fabs(offset.getAsDouble()));
            }

            game::ConveyorStruct::ApplyTerminationDeduction<IsLeft>(
                segment.terminationType, target_segment.terminationType, target_offset);

            bool moved_item;
            // Decides how the items will be fed into the target segment (if at all)
            switch (segment.terminationType) {
            default:
                moved_item = target_segment.TryInsertItem(IsLeft, target_offset.getAsDouble(), *side.lane[index].item);
                break;

                // Side insertion
            case game::ConveyorStruct::TerminationType::left_only:
                moved_item = target_segment.left.TryInsertItem(
                    target_offset.getAsDouble(), *side.lane[index].item, target_segment.headOffset);
                break;
            case game::ConveyorStruct::TerminationType::right_only:
                moved_item = target_segment.right.TryInsertItem(
                    target_offset.getAsDouble(), *side.lane[index].item, target_segment.headOffset);
                break;
            }


            // Handle transition if the item has been added to another conveyor
            if (moved_item) {
                side.lane.pop_front(); // Remove item in current segment now moved away

                // Move the next item forwards to preserve spacing & update back_item_distance
                if (!side.lane.empty()) { // This will not work with speeds greater than item_spacing
                    // Offset is always negative
                    side.lane.front().dist += offset;
                }
                else {
                    // No items left in segment
                    back_item_distance = 0;
                }
                return;
            }
        }
        // No target segment or cannot move to the target segment

        // Adjust for the extra movement forwards
        // Set the item back to 0, the distance will be made up for by decreasing the distance of the next item
        offset = 0;
        back_item_distance += tiles_moved;

        if (MoveNextItem(tiles_moved, side.lane, index, segment.target != nullptr)) {
            back_item_distance -= tiles_moved;
        }
        /*
            // Disable conveyor since it does not feed anywhere
        else {
            switch (segment.terminationType) {
                // Due to how items feeding onto the sides of conveyor segments behave, they cannot be disabled unless
        empty case game::ConveyorSegment::TerminationType::left_only: case
        game::ConveyorSegment::TerminationType::right_only: break;

            default:
                index = UINT16_MAX;
                break;
            }
        }
        */
    }
    else {
        // ================================================
        // Items behind another item in conveyor

        // Items following the first item will leave a gap of item_width
        if (offset > proto::LineDistT(game::ConveyorProp::kItemSpacing))
            return;

        // Item has reached its end, set the offset to item_spacing since it was decremented 1 too many times
        offset = game::ConveyorProp::kItemSpacing;
        if (MoveNextItem(tiles_moved, side.lane, index, segment.target != nullptr)) {
            back_item_distance -= tiles_moved;
        }
    }
}

/// Moves items for conveyors
static void LogicUpdateMoveItems(const proto::Conveyor& line_proto, proto::ConveyorData& conveyor_data) {
    auto& line_segment = *conveyor_data.structure;

    // Left
    {
        auto& left       = line_segment.left;
        const auto index = line_segment.left.index;
        // Empty or index indicates nothing should be moved
        if (line_segment.left.IsActive()) {
            left.lane[index].dist -= line_proto.speed;
            line_segment.left.backItemDistance -= line_proto.speed;
        }
    }

    // Right
    {
        auto& right      = line_segment.right;
        const auto index = line_segment.right.index;
        // Empty or index indicates nothing should be moved
        if (line_segment.right.IsActive()) {
            right.lane[index].dist -= line_proto.speed;
            line_segment.right.backItemDistance -= line_proto.speed;
        }
    }
}

/// Transitions items on conveyors to other lines and modifies whether of not the line is active
static void LogicUpdateTransitionItems(const proto::Conveyor& line_proto, proto::ConveyorData& conveyor_data) {
    auto& line_segment = *conveyor_data.structure;

    const auto tiles_moved = line_proto.speed;

    if (line_segment.left.IsActive())
        UpdateSide<true>(tiles_moved, line_segment);

    if (line_segment.right.IsActive())
        UpdateSide<false>(tiles_moved, line_segment);
}

static void LogicUpdateSplitterSwap(const proto::Splitter& splitter, proto::SplitterData& splitter_data) {
    // 1. Find the candidates for swapping (both sides)
    // 2. Swapping logic

    struct SwapCandidate
    {
        /// Index in lane
        std::size_t index;
        proto::LineDistT distFromFront;
        game::ConveyorItem cItem;
    };

    auto find_swap_candidates = [&splitter](game::ConveyorLane& lane,
                                            const double lane_length) -> std::pair<bool, SwapCandidate> {
        // Distance from front of item to front of splitter conveyor
        proto::LineDistT dist_from_front;

        for (std::size_t i = 0; i < lane.lane.size(); ++i) {
            auto [dist, item] = lane.lane[i];
            dist_from_front += dist;

            const auto dist_from_rear = lane_length - dist_from_front.getAsDouble();

            // TODO maybe raise to *1.9 to reduce change of clogging side of conveyor

            // Swapping is only allowed for a short region, otherwise it swaps items back and fourth
            // * 1.5 for a margin, so items which was previously right at the threshold can also swap
            if (dist_from_rear > game::ConveyorProp::kSplitterThreshold &&
                dist_from_rear < game::ConveyorProp::kSplitterThreshold + 1.5 * splitter.speed.getAsDouble()) {
                return {true, {i, dist_from_front, lane.lane[i]}};
            }
        }
        return {false, {}};
    };

    // TODO length is not 1 for different termination types

    // ll = Left side, left lane
    const auto [ll_has, ll_candidate] = find_swap_candidates(splitter_data.structure->left, 1.);
    const auto [lr_has, lr_candidate] = find_swap_candidates(splitter_data.structure->right, 1.);
    const auto [rl_has, rl_candidate] = find_swap_candidates(splitter_data.right.structure->left, 1.);
    const auto [rr_has, rr_candidate] = find_swap_candidates(splitter_data.right.structure->right, 1.);

    const auto left_has  = ll_has || lr_has;
    const auto right_has = rl_has || rr_has;

    /// Swaps item from lane "from" to lane "to"
    /// \param candidiate Candidate from lane "from" to swap
    auto swap_to = [](game::ConveyorLane& from, game::ConveyorLane& to, const SwapCandidate& candidate) {
        from.RemoveItem(candidate.index);
        to.InsertItem(candidate.distFromFront.getAsDouble(), *candidate.cItem.item, 0);
    };

    // One of the sides has items
    if (left_has != right_has) {
        if (!splitter_data.swap) {
            splitter_data.swap = true;
            // Let the item pass through, don't swap this time
            return;
        }

        // No need to check for space on other side, since other side has no items at swap threshold region
        if (left_has) {
            if (ll_has) {
                swap_to(splitter_data.structure->left, splitter_data.right.structure->left, ll_candidate);
            }
            if (lr_has) {
                swap_to(splitter_data.structure->right, splitter_data.right.structure->right, lr_candidate);
            }
        }
        else {
            if (rl_has) {
                swap_to(splitter_data.right.structure->left, splitter_data.structure->left, rl_candidate);
            }
            if (rr_has) {
                swap_to(splitter_data.right.structure->right, splitter_data.structure->right, rr_candidate);
            }
        }

        splitter_data.swap = false;
    }
    else if (left_has && right_has) {
        // TODO what if both sides has items
        splitter_data.swap = false;
    }
}


void game::ConveyorLogicUpdate(World& world) {
    // The logic update of conveyor items occur in 2 stages:
    // 		1. Move items on their conveyors
    //		2. Check if any items have reached the end of their lines, and need to be moved to another one

    for (auto [prototype, unique_data, coord] : world.LogicGet(LogicGroup::conveyor)) {
        const auto* line_proto = SafeCast<const proto::Conveyor*>(prototype.Get());
        auto* con_data         = SafeCast<proto::ConveyorData*>(unique_data.Get());

        assert(line_proto != nullptr);
        assert(con_data != nullptr);

        LogicUpdateMoveItems(*line_proto, *con_data);
    }

    for (auto [prototype, unique_data, coord] : world.LogicGet(LogicGroup::conveyor)) {
        const auto* line_proto = SafeCast<const proto::Conveyor*>(prototype.Get());
        auto* con_data         = SafeCast<proto::ConveyorData*>(unique_data.Get());

        assert(line_proto != nullptr);
        assert(con_data != nullptr);

        LogicUpdateTransitionItems(*line_proto, *con_data);
    }
}

void game::SplitterLogicUpdate(World& world) {
    // Similar to conveyors
    // 		1. Move items on their conveyors
    //		2. Check if any items have reached the end of their lines, and need to be moved to another one

    for (auto [prototype, unique_data, coord] : world.LogicGet(LogicGroup::splitter)) {
        auto* splitter_proto = SafeCast<const proto::Splitter*>(prototype.Get());
        auto* splitter_data  = SafeCast<proto::SplitterData*>(unique_data.Get());

        assert(splitter_proto != nullptr);
        assert(splitter_data != nullptr);

        LogicUpdateMoveItems(*splitter_proto, *splitter_data);
        LogicUpdateMoveItems(*splitter_proto, splitter_data->right);
    }

    for (auto [prototype, unique_data, coord] : world.LogicGet(LogicGroup::splitter)) {
        auto* splitter_proto = SafeCast<const proto::Splitter*>(prototype.Get());
        auto* splitter_data  = SafeCast<proto::SplitterData*>(unique_data.Get());

        assert(splitter_proto != nullptr);
        assert(splitter_data != nullptr);

        LogicUpdateSplitterSwap(*splitter_proto, *splitter_data);

        LogicUpdateTransitionItems(*splitter_proto, *splitter_data);
        LogicUpdateTransitionItems(*splitter_proto, splitter_data->right);
    }
}
