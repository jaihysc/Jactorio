// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "data/prototype/entity/transport/transport_line.h"

#include <cmath>

#include "game/logic/transport_line_controller.h"

#include "game/logic/transport_line_structure.h"
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

void UpdateSide(const jactorio::game::TransportLineOffset& tiles_moved, jactorio::game::TransportSegment* segment,
                const bool is_left) {
	auto& side = segment->GetSide(is_left);
	uint16_t& index = side.index;

	jactorio::game::TransportLineOffset& offset             = side.lane[index].first;
	jactorio::game::TransportLineOffset& back_item_distance = side.backItemDistance;

	// Front item if index is 0
	if (index == 0) {
		// Front item does not need to be moved
		if (offset >= dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(0))
			return;

		if (segment->targetSegment) {
			jactorio::game::TransportLineOffset target_offset =
				dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(
					static_cast<double>(segment->targetSegment->length) - fabs(
						offset.getAsDouble()));  // From start of line

			jactorio::game::TransportLineOffset target_offset_tile;  // From previous item


			// Account for the termination type of the line segments for the offset from start to insert into
			if (is_left) {
				ApplyTerminationDeductionL(segment->terminationType, target_offset);
				ApplyTerminationDeductionL(segment->targetSegment->terminationType, target_offset);

				// Offset from end of transport line only calculated if line is empty,
				// otherwise it maintains distance to previous item
				target_offset_tile = target_offset - segment->targetSegment->left.backItemDistance;
			}
			else {
				ApplyTerminationDeductionR(segment->terminationType, target_offset);
				ApplyTerminationDeductionR(segment->targetSegment->terminationType, target_offset);

				target_offset_tile = target_offset - segment->targetSegment->right.backItemDistance;
			}


			bool added_item = false;
			// Decides how the items will be fed into the target segment (if at all)
			switch (segment->terminationType) {
			default:
				{
					auto& target_segment = *segment->targetSegment;
					if (target_segment.CanInsert(is_left, target_offset)) {
						added_item = true;

						// Reenable the segment if it was disabled
						if (is_left) {
							if (!target_segment.left.IsActive())
								target_segment.left.index = target_segment.left.lane.size();
						}
						else {
							if (!target_segment.right.IsActive())
								target_segment.right.index = target_segment.right.lane.size();
						}

						segment->targetSegment->AppendItem(
							is_left,
							target_offset_tile.getAsDouble(), side.lane[index].second);
					}
				}
				break;

				// Side insertion
			case jactorio::game::TransportSegment::TerminationType::left_only:
				if (segment->targetSegment->CanInsert(true, target_offset)) {
					segment->targetSegment->InsertItem(true,
					                                   target_offset.getAsDouble(),
					                                   side.lane[index].second);
					added_item = true;
				}

				break;
			case jactorio::game::TransportSegment::TerminationType::right_only:
				if (segment->targetSegment->CanInsert(false, target_offset)) {
					segment->targetSegment->InsertItem(false,
					                                   target_offset.getAsDouble(),
					                                   side.lane[index].second);
					added_item = true;

				}

				break;
			}


			// Handle transition if the item has been added to another transport line
			if (added_item) {
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

		if (MoveNextItem(tiles_moved, side.lane, index, segment->targetSegment != nullptr)) {
			back_item_distance -= tiles_moved;
		}
			// Disable transport line since it does not feed anywhere
		else {
			switch (segment->terminationType) {
				// Due to how items feeding onto the sides of transport segments behave, they cannot be disabled unless empty
			case jactorio::game::TransportSegment::TerminationType::left_only:
			case jactorio::game::TransportSegment::TerminationType::right_only:
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
		if (offset > dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(
			jactorio::game::kItemSpacing))
			return;

		// Item has reached its end, set the offset to item_spacing since it was decremented 1 too many times
		offset = jactorio::game::kItemSpacing;
		if (MoveNextItem(tiles_moved, side.lane, index,
		                 segment->targetSegment != nullptr)) {
			back_item_distance -= tiles_moved;
		}
	}

}

void jactorio::game::LogicUpdateMoveItems(LogicChunk* l_chunk) {
	auto& layers = l_chunk->GetStruct(LogicChunk::StructLayer::transport_line);

	// Each object layer holds a transport line segment
	for (auto& object_layer : layers) {
		const auto* line_proto = static_cast<const data::TransportLine*>(object_layer.prototypeData);
		auto* line_segment     = static_cast<TransportSegment*>(object_layer.uniqueData);

		// Left
		{
			auto& left       = line_segment->left;
			const auto index = line_segment->left.index;
			// Empty or index indicates nothing should be moved
			if (line_segment->left.IsActive()) {
				left.lane[index].first -= line_proto->speed;
				line_segment->left.backItemDistance -= line_proto->speed;
			}
		}

		// Right
		{
			auto& right      = line_segment->right;
			const auto index = line_segment->right.index;
			// Empty or index indicates nothing should be moved
			if (line_segment->right.IsActive()) {
				right.lane[index].first -= line_proto->speed;
				line_segment->right.backItemDistance -= line_proto->speed;
			}
		}
	}
}

void jactorio::game::LogicUpdateTransitionItems(LogicChunk* l_chunk) {
	auto& layers = l_chunk->GetStruct(LogicChunk::StructLayer::transport_line);

	// Each object layer holds a transport line segment
	for (auto& object_layer : layers) {
		const auto* line_proto = static_cast<const data::TransportLine*>(object_layer.prototypeData);
		auto* line_segment     = static_cast<TransportSegment*>(object_layer.uniqueData);


		auto tiles_moved = line_proto->speed;

		if (line_segment->left.IsActive())
			UpdateSide(tiles_moved, line_segment, true);

		if (line_segment->right.IsActive())
			UpdateSide(tiles_moved, line_segment, false);
	}
}

void jactorio::game::TransportLineLogicUpdate(WorldData& world_data) {
	// The logic update of transport line items occur in 2 stages:
	// 		1. Move items on their transport lines
	//		2. Check if any items have reached the end of their lines, and need to be moved to another one

	for (auto& logic_chunk_pair : world_data.LogicGetAllChunks()) {
		LogicUpdateMoveItems(&logic_chunk_pair.second);
	}

	for (auto& logic_chunk_pair : world_data.LogicGetAllChunks()) {
		LogicUpdateTransitionItems(&logic_chunk_pair.second);
	}
}
