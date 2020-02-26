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

void jactorio::game::transport_line_c::logic_update(std::queue<Segment_transition_item>& queue, Logic_chunk* l_chunk) {
	auto& layers = l_chunk->get_struct(Logic_chunk::structLayer::transport_line);

	// Each objectLayer holds a transport line segment
	for (auto& object_layer : layers) {
		std::vector<transport_line_item> transition_items;

		auto* line_proto = static_cast<data::Transport_line*>(object_layer.prototype_data);
		auto* line_segment = static_cast<game::Transport_line_segment*>(object_layer.unique_data);

		// Left and right sides of the belt
		auto& left = line_segment->left;
		if (!left.empty()) {
			auto& offset = left.front().first;
			if ((offset -= line_proto->speed) < 0.f - jactorio::core::transport_line_epsilon) {
				transition_items.emplace_back(std::move(left.front()));
				left.pop_front();  // Remove item now moved away

				// Move the next item forwards to preserve spacing
				if (!left.empty())
					left.front().first -= line_proto->speed;
			}
		}

		auto& right = line_segment->right;
		if (!right.empty()) {
			auto& offset = right.front().first;
			if ((offset -= line_proto->speed) < 0.f - jactorio::core::transport_line_epsilon) {
				right.front().first *= -1;  // Invert to positive to indicate it belongs on the right side
				transition_items.emplace_back(std::move(right.front()));
				right.pop_front();  // Remove item now moved away

				// Move the next item forwards to preserve spacing
				if (!right.empty())
					right.front().first -= line_proto->speed;
			}
		}


		// Add to the chunk transition item queue if it the local queue has items
		if (!transition_items.empty()) {
			queue.emplace(line_segment->target_segment, line_segment, std::move(transition_items));
		}

	}
}

void jactorio::game::transport_line_c::logic_process_queued_items(std::queue<Segment_transition_item>& queue) {
	while (!queue.empty()) {
		auto& transition_items = queue.front();

		// Add everything in pair.second into the segment at pair.first
		for (transport_line_item& line_item : transition_items.items) {
			// Faster version of iterating thorugh all existing items:
			// This can be mitigated by saving the distance to end of the back item
			// Avoids having to iterate through and count

			float offset = static_cast<float>(transition_items.target_segment->segment_length) - fabs(line_item.first);
			const bool is_left_side = line_item.first < 0.f;

			// Offset from end of transport line only calculated if line is empty, otherwise it maintains distance to previous item
			if (is_left_side) {
				// Account for the termination type of the new line segment
				apply_termination_deduction_l(transition_items.previous_segment->termination_type, offset);
				apply_termination_deduction_l(transition_items.target_segment->termination_type, offset);

				// The offset to the next item is the current calculated offset from end - last item's position
//				if (!transition_items.target_segment->left.empty()) {
				// TODO see todo above for a more efficient approach (This only passes the test)
				for (const auto& item : transition_items.target_segment->left) {
					offset -= item.first;
				}

//					offset -= transition_items.target_segment->left.back().first;
//				}
			}
			else {
				apply_termination_deduction_r(transition_items.previous_segment->termination_type, offset);
				apply_termination_deduction_r(transition_items.target_segment->termination_type, offset);

//				if (!transition_items.target_segment->right.empty()) {
				for (const auto& item : transition_items.target_segment->right) {
					offset -= item.first;
				}
//					offset -= transition_items.target_segment->right.back().first;
//				}
			}

			// Item goes on left side if offset is negative
			belt_insert_item(is_left_side, transition_items.target_segment, offset, line_item.second);
		}

		queue.pop();
	}
}


// Item insertion

void jactorio::game::transport_line_c::belt_insert_item(bool insert_left,
														game::Transport_line_segment* belt, float offset, data::Item* item) {
	std::deque<std::pair<float, data::Item*>>& target_queue = insert_left ? belt->left : belt->right;

	// A minimum distance of transport_line_c::item_spacing is maintained between items (AFTER the initial item)
	if (offset < item_spacing && !target_queue.empty())
		offset = item_spacing;

	target_queue.emplace_back(offset, item);
}
