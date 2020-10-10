// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_controller.h"

#include <cmath>

#include "game/logic/conveyor_segment.h"
#include "game/world/world_data.h"
#include "proto/abstract/conveyor.h"

using namespace jactorio;

///
/// Sets index to the next item with a distance greater than item_width and decrement it
/// If there is no item AND has_target_segment == false, index is set as size of conveyor
/// \return true if an item was decremented
J_NODISCARD bool MoveNextItem(const proto::LineDistT& tiles_moved,
                              std::deque<game::ConveyorItem>& line_side,
                              uint16_t& index,
                              const bool has_target_segment) {
    for (size_t i = core::SafeCast<size_t>(index) + 1; i < line_side.size(); ++i) {
        auto& i_item_offset = line_side[i].dist;
        if (i_item_offset > proto::LineDistT(game::ConveyorProp::kItemSpacing)) {
            // Found a valid item to decrement
            if (!has_target_segment) {
                // Always check every item from index 0 if there is a target segment as the previous item may have moved
                index = core::SafeCast<uint16_t>(i);
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
void UpdateSide(const proto::LineDistT& tiles_moved, game::ConveyorSegment& segment) {
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

        if (segment.targetSegment) {
            game::ConveyorSegment& target_segment = *segment.targetSegment;
            // Offset to insert at target segment from head
            proto::LineDistT target_offset;
            {
                double length;
                switch (segment.terminationType) {
                    // Since segments terminating with side only can target the middle of a grouped segment, it uses
                    // its own member for head offset
                case game::ConveyorSegment::TerminationType::left_only:
                case game::ConveyorSegment::TerminationType::right_only:
                    // |   |   |   |
                    // 3   2   1   0
                    // targetOffset of 0: Length is 1
                    length = core::SafeCast<double>(1) + segment.targetInsertOffset;
                    break;

                default:
                    length = target_segment.length;
                    break;
                }
                target_offset = proto::LineDistT(length - fabs(offset.getAsDouble()));
            }

            game::ConveyorSegment::ApplyTerminationDeduction<IsLeft>(
                segment.terminationType, target_segment.terminationType, target_offset);

            bool moved_item;
            // Decides how the items will be fed into the target segment (if at all)
            switch (segment.terminationType) {
            default:
                moved_item = target_segment.TryInsertItem(IsLeft, target_offset.getAsDouble(), *side.lane[index].item);
                break;

                // Side insertion
            case game::ConveyorSegment::TerminationType::left_only:
                moved_item = target_segment.left.TryInsertItem(
                    target_offset.getAsDouble(), *side.lane[index].item, target_segment.itemOffset);
                break;
            case game::ConveyorSegment::TerminationType::right_only:
                moved_item = target_segment.right.TryInsertItem(
                    target_offset.getAsDouble(), *side.lane[index].item, target_segment.itemOffset);
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

        if (MoveNextItem(tiles_moved, side.lane, index, segment.targetSegment != nullptr)) {
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
        if (MoveNextItem(tiles_moved, side.lane, index, segment.targetSegment != nullptr)) {
            back_item_distance -= tiles_moved;
        }
    }
}

///
/// Moves items for conveyors
/// \param l_chunk Chunk to update
void LogicUpdateMoveItems(const game::Chunk& l_chunk) {
    const auto& layers = l_chunk.GetLogicGroup(game::Chunk::LogicGroup::conveyor);

    // Each object layer holds a conveyor segment
    for (const auto& tile_layer : layers) {
        const auto& line_proto = *static_cast<const proto::Conveyor*>(tile_layer->prototypeData.Get());
        auto& line_segment     = *tile_layer->GetUniqueData<proto::ConveyorData>()->lineSegment;

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
}

///
/// Transitions items on conveyors to other lines and modifies whether of not the line is active
/// \param l_chunk Chunk to update
void LogicUpdateTransitionItems(const game::Chunk& l_chunk) {
    const auto& layers = l_chunk.GetLogicGroup(game::Chunk::LogicGroup::conveyor);

    // Each object layer holds a conveyor segment
    for (const auto& tile_layer : layers) {
        const auto* line_proto = static_cast<const proto::Conveyor*>(tile_layer->prototypeData.Get());
        auto& line_segment     = *tile_layer->GetUniqueData<proto::ConveyorData>()->lineSegment;

        auto tiles_moved = line_proto->speed;

        if (line_segment.left.IsActive())
            UpdateSide<true>(tiles_moved, line_segment);

        if (line_segment.right.IsActive())
            UpdateSide<false>(tiles_moved, line_segment);
    }
}

void game::ConveyorLogicUpdate(WorldData& world_data) {
    // The logic update of conveyor items occur in 2 stages:
    // 		1. Move items on their conveyors
    //		2. Check if any items have reached the end of their lines, and need to be moved to another one

    for (const auto& chunk_pair : world_data.LogicGetChunks()) {
        LogicUpdateMoveItems(*chunk_pair);
    }

    for (const auto& chunk_pair : world_data.LogicGetChunks()) {
        LogicUpdateTransitionItems(*chunk_pair);
    }
}
