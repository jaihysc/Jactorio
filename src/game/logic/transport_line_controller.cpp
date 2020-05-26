// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "data/prototype/entity/transport/transport_line.h"

#include <cmath>

#include "game/logic/transport_line_controller.h"

#include "game/logic/transport_segment.h"
#include "game/world/world_data.h"

void ApplyTerminationDeductionL(const jactorio::game::TransportSegment::TerminationType termination_type,
                                jactorio::game::TransportLineOffset& offset) {
	switch (termination_type) {
		// Feeding into another belt also needs to be deducted to feed at the right offset on the target belt
	case jactorio::game::TransportSegment::TerminationType::left_only:
	case jactorio::game::TransportSegment::TerminationType::bend_left:
		offset -= dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(
			jactorio::game::TransportSegment::kBendLeftLReduction);
		break;

	case jactorio::game::TransportSegment::TerminationType::right_only:
	case jactorio::game::TransportSegment::TerminationType::bend_right:
		offset -= dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(
			jactorio::game::TransportSegment::kBendRightLReduction);
		break;

	case jactorio::game::TransportSegment::TerminationType::straight:
		break;
	}
}

void ApplyTerminationDeductionR(const jactorio::game::TransportSegment::TerminationType termination_type,
                                jactorio::game::TransportLineOffset& offset) {
	switch (termination_type) {
	case jactorio::game::TransportSegment::TerminationType::left_only:
	case jactorio::game::TransportSegment::TerminationType::bend_left:
		offset -= dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(
			jactorio::game::TransportSegment::kBendLeftRReduction);
		break;

	case jactorio::game::TransportSegment::TerminationType::right_only:
	case jactorio::game::TransportSegment::TerminationType::bend_right:
		offset -= dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(
			jactorio::game::TransportSegment::kBendRightRReduction);
		break;

	case jactorio::game::TransportSegment::TerminationType::straight:
		break;
	}
}

///
/// \brief Sets index to the next item with a distance greater than item_width and decrement it
/// If there is no item AND has_target_segment == false, index is set as size of transport line
/// \return true if an item was decremented
J_NODISCARD bool MoveNextItem(const jactorio::game::TransportLineOffset& tiles_moved,
                              std::deque<jactorio::game::TransportLineItem>& line_side,
                              uint16_t& index, const bool has_target_segment) {
	for (decltype(line_side.size()) i = index + 1; i < line_side.size(); ++i) {
		auto& i_item_offset = line_side[i].first;
		if (i_item_offset > dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(
			jactorio::game::kItemSpacing)) {

			// Found a valid item to decrement
			index = i;
			i_item_offset -= tiles_moved;
			return true;
		}
	}

	// Failed to find another item
	// set to 1 past the index of the last item (stop the transport line permanently) if there is no target segment
	if (!has_target_segment)
		index = line_side.size();
	else
		index = 0;

	return false;
}

template <bool IsLeft>
void UpdateSide(const jactorio::game::TransportLineOffset& tiles_moved, jactorio::game::TransportSegment& segment) {
	using namespace jactorio;
	auto& side      = segment.GetSide(IsLeft);
	uint16_t& index = side.index;

	game::TransportLineOffset& offset             = side.lane[index].first;
	game::TransportLineOffset& back_item_distance = side.backItemDistance;

	// Front item if index is 0
	if (index == 0) {
		// Front item does not need to be moved
		if (offset >= dec::decimal_cast<game::kTransportLineDecimalPlace>(0))
			return;

		if (segment.targetSegment) {
			game::TransportSegment& target_segment = *segment.targetSegment;
			// Offset to insert at target segment from head 
			game::TransportLineOffset target_offset;
			{
				double length;
				switch (segment.terminationType) {
					// Since segments terminating with side only can target the middle of a grouped segment, it uses
					// its own member for head offset
				case game::TransportSegment::TerminationType::left_only:
				case game::TransportSegment::TerminationType::right_only:
					// |   |   |   |
					// 3   2   1   0
					// targetOffset of 0: Length is 1
					length = segment.targetInsertOffset + 1.f;
					break;

				default:
					length = target_segment.length;
					break;
				}
				target_offset = dec::decimal_cast<game::kTransportLineDecimalPlace>(
					length - fabs(offset.getAsDouble())
				);
			}

			// Account for the termination type of the line segments for the offset from start to insert into
			if constexpr (IsLeft) {
				ApplyTerminationDeductionL(segment.terminationType, target_offset);
				ApplyTerminationDeductionL(target_segment.terminationType, target_offset);
			}
			else {
				ApplyTerminationDeductionR(segment.terminationType, target_offset);
				ApplyTerminationDeductionR(target_segment.terminationType, target_offset);
			}

			bool moved_item;
			// Decides how the items will be fed into the target segment (if at all)
			switch (segment.terminationType) {
			default:
				moved_item = target_segment.TryInsertItem(IsLeft,
				                                          target_offset.getAsDouble(),
				                                          side.lane[index].second);
				break;

				// Side insertion
			case game::TransportSegment::TerminationType::left_only:
				moved_item = target_segment.left.TryInsertItem(target_offset.getAsDouble(),
				                                               side.lane[index].second,
				                                               target_segment.itemOffset);
				break;
			case game::TransportSegment::TerminationType::right_only:
				moved_item = target_segment.right.TryInsertItem(target_offset.getAsDouble(),
				                                                side.lane[index].second,
				                                                target_segment.itemOffset);
				break;
			}


			// Handle transition if the item has been added to another transport line
			if (moved_item) {
				side.lane.pop_front();  // Remove item in current segment now moved away

				// Move the next item forwards to preserve spacing & update back_item_distance
				if (!side.lane.empty()) {  // This will not work with speeds greater than item_spacing
					// Offset is always negative
					side.lane.front().first += offset;
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
			// Disable transport line since it does not feed anywhere
		else {
			switch (segment.terminationType) {
				// Due to how items feeding onto the sides of transport segments behave, they cannot be disabled unless empty
			case game::TransportSegment::TerminationType::left_only:
			case game::TransportSegment::TerminationType::right_only:
				break;

			default:
				index = UINT16_MAX;
				break;
			}
		}
	}
	else {
		// ================================================
		// Items behind another item in transport line

		// Items following the first item will leave a gap of item_width
		if (offset > dec::decimal_cast<game::kTransportLineDecimalPlace>(
			game::kItemSpacing))
			return;

		// Item has reached its end, set the offset to item_spacing since it was decremented 1 too many times
		offset = game::kItemSpacing;
		if (MoveNextItem(tiles_moved, side.lane, index,
		                 segment.targetSegment != nullptr)) {
			back_item_distance -= tiles_moved;
		}
	}

}

///
/// \brief Moves items for transport lines
/// \param l_chunk Chunk to update
void LogicUpdateMoveItems(const jactorio::game::Chunk& l_chunk) {
	using namespace jactorio;
	const auto& layers = l_chunk.GetLogicGroup(game::Chunk::LogicGroup::transport_line);

	// Each object layer holds a transport line segment
	for (const auto& tile_layer : layers) {
		const auto& line_proto = *static_cast<const data::TransportLine*>(tile_layer->prototypeData);
		auto& line_segment     = static_cast<const data::TransportLineData*>(tile_layer->uniqueData)->lineSegment.get();

		// Left
		{
			auto& left       = line_segment.left;
			const auto index = line_segment.left.index;
			// Empty or index indicates nothing should be moved
			if (line_segment.left.IsActive()) {
				left.lane[index].first -= line_proto.speed;
				line_segment.left.backItemDistance -= line_proto.speed;
			}
		}

		// Right
		{
			auto& right      = line_segment.right;
			const auto index = line_segment.right.index;
			// Empty or index indicates nothing should be moved
			if (line_segment.right.IsActive()) {
				right.lane[index].first -= line_proto.speed;
				line_segment.right.backItemDistance -= line_proto.speed;
			}
		}
	}
}

///
/// \brief Transitions items on transport lines to other lines and modifies whether of not the line is active
/// \param l_chunk Chunk to update
void LogicUpdateTransitionItems(const jactorio::game::Chunk& l_chunk) {
	using namespace jactorio;
	const auto& layers = l_chunk.GetLogicGroup(game::Chunk::LogicGroup::transport_line);

	// Each object layer holds a transport line segment
	for (const auto& tile_layer : layers) {
		const auto* line_proto = static_cast<const data::TransportLine*>(tile_layer->prototypeData);
		auto& line_segment     = static_cast<const data::TransportLineData*>(tile_layer->uniqueData)->lineSegment.get();

		auto tiles_moved = line_proto->speed;

		if (line_segment.left.IsActive())
			UpdateSide<true>(tiles_moved, line_segment);

		if (line_segment.right.IsActive())
			UpdateSide<false>(tiles_moved, line_segment);
	}
}

void jactorio::game::TransportLineLogicUpdate(WorldData& world_data) {
	// The logic update of transport line items occur in 2 stages:
	// 		1. Move items on their transport lines
	//		2. Check if any items have reached the end of their lines, and need to be moved to another one

	for (const auto& chunk_pair : world_data.LogicGetChunks()) {
		LogicUpdateMoveItems(*chunk_pair);
	}

	for (const auto& chunk_pair : world_data.LogicGetChunks()) {
		LogicUpdateTransitionItems(*chunk_pair);
	}
}
