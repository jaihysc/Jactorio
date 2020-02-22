#include <data/prototype/entity/transport/transport_line.h>
#include "game/logic/transport_line_controller.h"

#include "core/float_math.h"
#include "game/world/world_manager.h"
#include "game/logic/transport_line_structure.h"

void jactorio::game::transport_line_c::logic_update(std::queue<segment_transition_item>& queue, Logic_chunk* l_chunk) {
	auto& layers = l_chunk->chunk->get_object(Chunk::object_layer::transport_line);

	// Each object_layer holds a transport line segment
	for (auto& object_layer : layers) {
		std::vector<transport_line_item> transition_items;

		auto* line_proto = static_cast<data::Transport_line*>(object_layer.prototype_data);
		auto* line_segment = static_cast<game::Transport_line_segment*>(object_layer.unique_data);

		// Left and right sides of the belt
		auto& left = line_segment->left;
		if (!left.empty()) {
			auto& offset = left.front().first;
			if ((offset -= line_proto->speed) < 0.f - jactorio::core::transport_line_epsilon) {
				switch (line_segment->termination_type) {
					case Transport_line_segment::termination_type::bend_left:
						offset -= Transport_line_segment::bend_left_l_reduction;
						break;
					case Transport_line_segment::termination_type::bend_right:
						offset -= Transport_line_segment::bend_right_l_reduction;
						break;
				}

				transition_items.emplace_back(std::move(left.front()));
				left.pop_front();  // Remove item now moved away
			}
		}

		auto& right = line_segment->right;
		if (!right.empty()) {
			auto& offset = right.front().first;
			if ((offset -= line_proto->speed) < 0.f - jactorio::core::transport_line_epsilon) {
				// Account for termination type by increasing the current offset
				switch (line_segment->termination_type) {
					case Transport_line_segment::termination_type::bend_left:
						offset -= Transport_line_segment::bend_left_r_reduction;
						break;
					case Transport_line_segment::termination_type::bend_right:
						offset -= Transport_line_segment::bend_right_r_reduction;
						break;
				}

				right.front().first *= -1;  // Invert to positive to indicate it belongs on the right side
				transition_items.emplace_back(std::move(right.front()));
				right.pop_front();  // Remove item now moved away
			}
		}


		// Add to the chunk transition item queue if it the local queue has items
		if (!transition_items.empty()) {
			queue.push({line_segment->target_segment, std::move(transition_items)});
		}

	}
}

void jactorio::game::transport_line_c::logic_process_queued_items(std::queue<segment_transition_item>& queue) {
	while (!queue.empty()) {
		auto& pair = queue.front();

		// Add everything in pair.second into the segment at pair.first
		for (transport_line_item& line_item : pair.second) {
			float offset = static_cast<float>(pair.first->segment_length) - fabs(line_item.first);
			const bool left = line_item.first < 0.f;

			// Account for the termination type of the new line segment
			if (left) {
				switch (pair.first->termination_type) {
					case Transport_line_segment::termination_type::bend_left:
						offset -= Transport_line_segment::bend_left_l_reduction;
						break;
					case Transport_line_segment::termination_type::bend_right:
						offset -= Transport_line_segment::bend_right_l_reduction;
						break;
				}
			}
			else {
				switch (pair.first->termination_type) {
					case Transport_line_segment::termination_type::bend_left:
						offset -= Transport_line_segment::bend_left_r_reduction;
						break;
					case Transport_line_segment::termination_type::bend_right:
						offset -= Transport_line_segment::bend_right_r_reduction;
						break;
				}
			}

			// Item goes on left side if offset is negative
			belt_insert_item(left, pair.first, offset, line_item.second);
		}

		queue.pop();
	}
}


// Item insertion

void jactorio::game::transport_line_c::belt_insert_item_l(const int tile_x, const int tile_y,
														  data::Item* item) {
	// auto* chunk = world_manager::get_chunk_world_coords(tile_x, tile_y);
	// chunk_insert_item(chunk,
	//                   static_cast<float>(tile_x) + 0.3f, 
	//                   static_cast<float>(tile_y) + 0.5f, 
	//                   item);
}

void jactorio::game::transport_line_c::belt_insert_item_r(int tile_x, int tile_y,
														  data::Item* item) {

}

void jactorio::game::transport_line_c::belt_insert_item(bool insert_left,
														game::Transport_line_segment* belt, float offset, data::Item* item) {
	std::deque<std::pair<float, data::Item*>>& target_queue = insert_left ? belt->left : belt->right;
	auto iterator = target_queue.begin();

	// Insert to the next location where [i].offset < [i + 1].offset
	while (iterator != target_queue.end()) {
		if (offset < iterator->first) {
			break;
		}
		++iterator;
	}
	target_queue.insert(iterator, {offset, item});
}
