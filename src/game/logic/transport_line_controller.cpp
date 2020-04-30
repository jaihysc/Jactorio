// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "data/prototype/entity/transport/transport_line.h"

#include <cmath>

#include "game/logic/transport_line_controller.h"

#include "game/logic/transport_line_structure.h"
#include "game/world/world_data.h"

void apply_termination_deduction_l(const jactorio::game::Transport_line_segment::TerminationType termination_type,
                                   jactorio::transport_line_offset& offset) {
	switch (termination_type) {
		// Feeding into another belt also needs to be deducted to feed at the right offset on the target belt
	case jactorio::game::Transport_line_segment::TerminationType::left_only:
	case jactorio::game::Transport_line_segment::TerminationType::bend_left:
		offset -= dec::decimal_cast<jactorio::transport_line_decimal_place>(
			jactorio::game::Transport_line_segment::bend_left_l_reduction);
		break;

	case jactorio::game::Transport_line_segment::TerminationType::right_only:
	case jactorio::game::Transport_line_segment::TerminationType::bend_right:
		offset -= dec::decimal_cast<jactorio::transport_line_decimal_place>(
			jactorio::game::Transport_line_segment::bend_right_l_reduction);
		break;

	case jactorio::game::Transport_line_segment::TerminationType::straight:
		break;
	}
}

void apply_termination_deduction_r(const jactorio::game::Transport_line_segment::TerminationType termination_type,
                                   jactorio::transport_line_offset& offset) {
	switch (termination_type) {
	case jactorio::game::Transport_line_segment::TerminationType::left_only:
	case jactorio::game::Transport_line_segment::TerminationType::bend_left:
		offset -= dec::decimal_cast<jactorio::transport_line_decimal_place>(
			jactorio::game::Transport_line_segment::bend_left_r_reduction);
		break;

	case jactorio::game::Transport_line_segment::TerminationType::right_only:
	case jactorio::game::Transport_line_segment::TerminationType::bend_right:
		offset -= dec::decimal_cast<jactorio::transport_line_decimal_place>(
			jactorio::game::Transport_line_segment::bend_right_r_reduction);
		break;

	case jactorio::game::Transport_line_segment::TerminationType::straight:
		break;
	}
}

///
/// \brief Sets index to the next item with a distance greater than item_width and decrement it
/// If there is no item AND has_target_segment == false, index is set as size of transport line
/// \return true if an item was decremented
J_NODISCARD bool move_next_item(const jactorio::transport_line_offset& tiles_moved,
                                std::deque<jactorio::game::transport_line_item>& line_side,
                                uint16_t& index, const bool has_target_segment) {
	for (decltype(line_side.size()) i = index + 1; i < line_side.size(); ++i) {
		auto& i_item_offset = line_side[i].first;
		if (i_item_offset > dec::decimal_cast<jactorio::transport_line_decimal_place>(
			jactorio::game::item_spacing)) {

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

void update_side(const jactorio::transport_line_offset& tiles_moved, jactorio::game::Transport_line_segment* segment,
                 const bool is_left) {
	auto& line_side = is_left ? segment->left : segment->right;
	uint16_t& index = is_left ? segment->l_index : segment->r_index;

	jactorio::transport_line_offset& offset = line_side[index].first;
	jactorio::transport_line_offset& back_item_distance = is_left
		                                                      ? segment->l_back_item_distance
		                                                      : segment->r_back_item_distance;

	// Front item if index is 0
	if (index == 0) {
		// Front item does not need to be moved
		if (offset >= dec::decimal_cast<jactorio::transport_line_decimal_place>(0))
			return;

		if (segment->target_segment) {
			jactorio::transport_line_offset target_offset =
				dec::decimal_cast<jactorio::transport_line_decimal_place>(
					static_cast<double>(segment->target_segment->length) - fabs(
						offset.getAsDouble()));  // From start of line

			jactorio::transport_line_offset target_offset_tile;  // From previous item


			// Account for the termination type of the line segments for the offset from start to insert into
			if (is_left) {
				apply_termination_deduction_l(segment->termination_type, target_offset);
				apply_termination_deduction_l(segment->target_segment->termination_type, target_offset);

				// Offset from end of transport line only calculated if line is empty,
				// otherwise it maintains distance to previous item
				target_offset_tile = target_offset - segment->target_segment->l_back_item_distance;
			}
			else {
				apply_termination_deduction_r(segment->termination_type, target_offset);
				apply_termination_deduction_r(segment->target_segment->termination_type, target_offset);

				target_offset_tile = target_offset - segment->target_segment->r_back_item_distance;
			}


			bool added_item = false;
			// Decides how the items will be fed into the target segment (if at all)
			switch (segment->termination_type) {
			default:
				{
					auto& target_segment = *segment->target_segment;
					if (target_segment.can_insert(is_left, target_offset)) {
						added_item = true;

						// Reenable the segment if it was disabled
						if (is_left) {
							if (!target_segment.is_active_left())
								target_segment.l_index = target_segment.left.size();
						}
						else {
							if (!target_segment.is_active_right())
								target_segment.r_index = target_segment.right.size();
						}

						segment->target_segment->append_item(
							is_left,
							target_offset_tile.getAsDouble(), line_side[index].second);
					}
				}
				break;

				// Side insertion
			case jactorio::game::Transport_line_segment::TerminationType::left_only:
				if (segment->target_segment->can_insert(true, target_offset)) {
					segment->target_segment->insert_item(true,
					                                     target_offset.getAsDouble(),
					                                     line_side[index].second);
					added_item = true;
				}

				break;
			case jactorio::game::Transport_line_segment::TerminationType::right_only:
				if (segment->target_segment->can_insert(false, target_offset)) {
					segment->target_segment->insert_item(false,
					                                     target_offset.getAsDouble(),
					                                     line_side[index].second);
					added_item = true;

				}

				break;
			}


			// Handle transition if the item has been added to another transport line
			if (added_item) {
				line_side.pop_front();  // Remove item in current segment now moved away

				// Move the next item forwards to preserve spacing & update back_item_distance
				if (!line_side.empty()) {  // This will not work with speeds greater than item_spacing
					// Offset is always negative
					line_side.front().first += offset;
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

		if (move_next_item(tiles_moved, line_side, index, segment->target_segment != nullptr)) {
			back_item_distance -= tiles_moved;
		}
			// Disable transport line since it does not feed anywhere
		else {
			switch (segment->termination_type) {
				// Due to how items feeding onto the sides of transport segments behave, they cannot be disabled unless empty
			case jactorio::game::Transport_line_segment::TerminationType::left_only:
			case jactorio::game::Transport_line_segment::TerminationType::right_only:
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
		if (offset > dec::decimal_cast<jactorio::transport_line_decimal_place>(
			jactorio::game::item_spacing))
			return;

		// Item has reached its end, set the offset to item_spacing since it was decremented 1 too many times
		offset = jactorio::game::item_spacing;
		if (move_next_item(tiles_moved, line_side, index,
		                   segment->target_segment != nullptr)) {
			back_item_distance -= tiles_moved;
		}
	}

}

void jactorio::game::logic_update_move_items(Logic_chunk* l_chunk) {
	auto& layers = l_chunk->get_struct(Logic_chunk::structLayer::transport_line);

	// Each object layer holds a transport line segment
	for (auto& object_layer : layers) {
		const auto* line_proto = static_cast<const data::Transport_line*>(object_layer.prototype_data);
		auto* line_segment = static_cast<Transport_line_segment*>(object_layer.unique_data);

		// Left
		{
			auto& left = line_segment->left;
			const auto index = line_segment->l_index;
			// Empty or index indicates nothing should be moved
			if (line_segment->is_active_left()) {
				left[index].first -= line_proto->speed;
				line_segment->l_back_item_distance -= line_proto->speed;
			}
		}

		// Right
		{
			auto& right = line_segment->right;
			const auto index = line_segment->r_index;
			// Empty or index indicates nothing should be moved
			if (line_segment->is_active_right()) {
				right[index].first -= line_proto->speed;
				line_segment->r_back_item_distance -= line_proto->speed;
			}
		}
	}
}

void jactorio::game::logic_update_transition_items(Logic_chunk* l_chunk) {
	auto& layers = l_chunk->get_struct(Logic_chunk::structLayer::transport_line);

	// Each object layer holds a transport line segment
	for (auto& object_layer : layers) {
		const auto* line_proto = static_cast<const data::Transport_line*>(object_layer.prototype_data);
		auto* line_segment = static_cast<Transport_line_segment*>(object_layer.unique_data);


		auto tiles_moved = line_proto->speed;

		if (line_segment->is_active_left())
			update_side(tiles_moved, line_segment, true);

		if (line_segment->is_active_right())
			update_side(tiles_moved, line_segment, false);
	}
}

void jactorio::game::transport_line_logic_update(World_data& world_data) {
	// The logic update of transport line items occur in 2 stages:
	// 		1. Move items on their transport lines
	//		2. Check if any items have reached the end of their lines, and need to be moved to another one

	for (auto& logic_chunk_pair : world_data.logic_get_all_chunks()) {
		logic_update_move_items(&logic_chunk_pair.second);
	}

	for (auto& logic_chunk_pair : world_data.logic_get_all_chunks()) {
		logic_update_transition_items(&logic_chunk_pair.second);
	}
}
