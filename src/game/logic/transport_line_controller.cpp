#include <data/prototype/entity/transport/transport_line.h>
#include "game/logic/transport_line_controller.h"

#include "core/float_math.h"
#include "game/world/world_manager.h"
#include "game/logic/transport_line_structure.h"

void apply_termination_deduction_l(jactorio::game::Transport_line_segment::terminationType termination_type, float& offset) {
	switch (termination_type) {
		case jactorio::game::Transport_line_segment::terminationType::bend_left:
			offset -= jactorio::game::Transport_line_segment::bend_left_l_reduction;
			break;
		case jactorio::game::Transport_line_segment::terminationType::bend_right:
			offset -= jactorio::game::Transport_line_segment::bend_right_l_reduction;
			break;

		case jactorio::game::Transport_line_segment::terminationType::straight:
			break;
	}
}

void apply_termination_deduction_r(jactorio::game::Transport_line_segment::terminationType termination_type, float& offset) {
	switch (termination_type) {
		case jactorio::game::Transport_line_segment::terminationType::bend_left:
			offset -= jactorio::game::Transport_line_segment::bend_left_r_reduction;
			break;
		case jactorio::game::Transport_line_segment::terminationType::bend_right:
			offset -= jactorio::game::Transport_line_segment::bend_right_r_reduction;
			break;

		case jactorio::game::Transport_line_segment::terminationType::straight:
			break;
	}
}

void update_side(const float tiles_moved, jactorio::game::Transport_line_segment* segment, bool is_left) {
	std::deque<jactorio::game::transport_line_item>& line_side = is_left ? segment->left : segment->right;
	uint16_t& index = is_left ? segment->l_index : segment->r_index;

	auto& offset = line_side[index].first;

	// Front item if index is 0
	if (index != 0)
		goto trailing_item;


	if (offset < 0.f - jactorio::core::transport_line_epsilon) {
		if (segment->target_segment) {
			float target_offset = static_cast<float>(segment->target_segment->segment_length) - fabs(offset);  // From start of line
			float target_offset_tile;  // From previous item

			// Faster version of iterating through all existing items:
			// This can be mitigated by saving the distance to end of the back item
			// Avoids having to iterate through and count

			// Account for the termination type of the line segments for the offset from start to insert into
			if (is_left) {
				apply_termination_deduction_l(segment->termination_type, target_offset);
				apply_termination_deduction_l(segment->target_segment->termination_type, target_offset);

				// Offset from end of transport line only calculated if line is empty, otherwise it maintains distance to previous item
				target_offset_tile = target_offset;
				// TODO see todo above for a more efficient approach (This only passes the test)
				for (const auto& item : segment->target_segment->left) {
					target_offset_tile -= item.first;
				}
			}
			else {
				apply_termination_deduction_r(segment->termination_type, target_offset);
				apply_termination_deduction_r(segment->target_segment->termination_type, target_offset);

				target_offset_tile = target_offset;
				for (const auto& item : segment->target_segment->right) {
					target_offset_tile -= item.first;
				}
			}


			if (segment->target_segment->can_insert(is_left, target_offset)) {
				// Item goes on left side if offset is negative
				jactorio::game::transport_line_c::belt_insert_item(is_left, segment->target_segment,
																   target_offset_tile, line_side[index].second);

				line_side.pop_front();  // Remove item in current segment now moved away

				// Move the next item forwards to preserve spacing
				// BUG depending on the order which transitions are processed, this below may not have ran, causing a belt to be disabled
				if (!line_side.empty())
					line_side.front().first -= tiles_moved;
				return;
			}
		}
		// No target segment or cannot move to the target segment

		// Set the item back to 0, the distance will be made up for by decreasing the distance of the next item
		offset = 0;
		goto move_next_item;
	}
	// Front item does not need to be moved
	return;


	// ================================================
	// Items behind another item in transport line
trailing_item:

	// Items following the first item will leave a gap of item_width
	if (offset > jactorio::game::transport_line_c::item_spacing - jactorio::core::transport_line_epsilon)
		return;

	// Item has reached its end, set the offset to item_spacing since it was decremented 1 too many times
	offset = jactorio::game::transport_line_c::item_spacing;
	goto move_next_item;


	// ========================================================
	// Find the next index with a distance greater than item_width
move_next_item:

	// Set index to the next item with a distance greater than item_width and decrement it
	for (int i = index + 1; i < line_side.size(); ++i) {
		auto& i_item_offset = line_side[i].first;
		if (i_item_offset > jactorio::game::transport_line_c::item_spacing + jactorio::core::transport_line_epsilon) {
			// Found a valid item to decrement
			index = i;
			i_item_offset -= tiles_moved;
			return;
		}
	}

	// Failed to find another item, set to 1 past the index of the last item
	index = line_side.size();
}

void jactorio::game::transport_line_c::logic_update_move_items(jactorio::game::Logic_chunk* l_chunk) {
	auto& layers = l_chunk->get_struct(Logic_chunk::structLayer::transport_line);

	// Each object layer holds a transport line segment
	for (auto& object_layer : layers) {
		auto* line_proto = static_cast<data::Transport_line*>(object_layer.prototype_data);
		auto* line_segment = static_cast<game::Transport_line_segment*>(object_layer.unique_data);

		// Left
		{
			auto& left = line_segment->left;
			auto index = line_segment->l_index;
			// Empty or index indicates nothing should be moved
			if (line_segment->is_active_left())
				left[index].first -= line_proto->speed;
		}

		// Right
		{
			auto& right = line_segment->right;
			auto index = line_segment->r_index;
			// Empty or index indicates nothing should be moved
			if (line_segment->is_active_right())
				right[index].first -= line_proto->speed;
		}
	}
}

void jactorio::game::transport_line_c::logic_update_transition_items(jactorio::game::Logic_chunk* l_chunk) {
	auto& layers = l_chunk->get_struct(Logic_chunk::structLayer::transport_line);

	// Each object layer holds a transport line segment
	for (auto& object_layer : layers) {
		auto* line_proto = static_cast<data::Transport_line*>(object_layer.prototype_data);
		auto* line_segment = static_cast<game::Transport_line_segment*>(object_layer.unique_data);

		auto tiles_moved = line_proto->speed;

		if (line_segment->is_active_left())
			update_side(tiles_moved, line_segment, true);

		if (line_segment->is_active_right())
			update_side(tiles_moved, line_segment, false);
	}
}

void jactorio::game::transport_line_c::transport_line_logic_update() {
	// The logic update of transport line items occur in 2 stages:
	// 		1. Move items on their transport lines
	//		2. Check if any items have reached the end of their lines, and need to be moved to another one

	for (auto& logic_chunk_pair : world_manager::logic_get_all_chunks()) {
		logic_update_move_items(&logic_chunk_pair.second);
	}

	for (auto& logic_chunk_pair : world_manager::logic_get_all_chunks()) {
		logic_update_transition_items(&logic_chunk_pair.second);
	}
}


// Item insertion

void jactorio::game::transport_line_c::belt_insert_item(bool insert_left,
														game::Transport_line_segment* belt, float offset, data::Item* item) {
	std::deque<std::pair<float, data::Item*>>& target_queue = insert_left ? belt->left : belt->right;

	// A minimum distance of transport_line_c::item_spacing is maintained between items (AFTER the initial item)
	if (offset < item_spacing && !target_queue.empty())
		offset = item_spacing;

//	LOG_MESSAGE_f(debug, "Transport line %s: Inserting at offset %f", insert_left ? "LEFT" : "RIGHT", offset);

	target_queue.emplace_back(offset, item);
}
