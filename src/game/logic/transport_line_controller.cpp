#include "game/logic/transport_line_controller.h"

#include "core/float_math.h"
#include "game/world/world_manager.h"

void jactorio::game::transport_line_c::logic_update(std::queue<chunk_transition_item_queue>& chunk_transition_items, Logic_chunk* l_chunk) {
	auto& layer = const_cast<std::vector<Chunk_object_layer>&>(
		l_chunk->chunk->objects[static_cast<int>(Chunk::object_layer::item_entity)]
	);

	// Containers for holding chunk items awaiting chunk transition into any of the 4 adjacent chunks
	auto pos = l_chunk->chunk->get_position();
	chunk_transition_item_queue queue_up{
		&game::world_manager::get_chunk(pos.first, pos.second - 1)->get_object(Chunk::object_layer::item_entity),
		{}
	};
	chunk_transition_item_queue queue_right{
		&game::world_manager::get_chunk(pos.first + 1, pos.second)->get_object(Chunk::object_layer::item_entity),
		{}
	};
	chunk_transition_item_queue queue_down{
		&game::world_manager::get_chunk(pos.first, pos.second + 1)->get_object(Chunk::object_layer::item_entity),
		{}
	};
	chunk_transition_item_queue queue_left{
		&game::world_manager::get_chunk(pos.first - 1, pos.second)->get_object(Chunk::object_layer::item_entity),
		{}
	};


	auto& line_updates = l_chunk->transport_line_updates;

	// Update each item in chunk
	for (int i = 0; i < layer.size(); ++i) {
		auto& line_item = layer[i];

		float lower_y = line_item.position_y - core::epsilon;
		float upper_y = line_item.position_y + core::epsilon;

		auto lower_iterator = line_updates.lower_bound(
			{
				line_item.position_x - core::epsilon,
				lower_y
			});
		auto upper_iterator = line_updates.upper_bound(
			{
				line_item.position_x + core::epsilon,
				upper_y
			});

		// lower, upper x iterators found X values in range, now find iterator with y within epsilon range
		bool match = false;
		// BUG the iterater search appears to be broken
//		do {
//			if (lower_iterator == line_updates.end())
//				break;
//
//			const float y_val = lower_iterator->first.second;
//			if (y_val >= lower_y && y_val < upper_y) {
//				match = true;
//				break;
//			}
//			++lower_iterator;
//		}
		while (lower_iterator != upper_iterator) {
			const float y_val = lower_iterator->first.second;
			if (y_val >= lower_y && y_val < upper_y) {
				match = true;
				break;
			}
			++lower_iterator;
		}

		auto* line_item_data = static_cast<data::Transport_line_item_data*>(line_item.unique_data);
		assert(line_item_data);  // Should never be nullptr
		if (match) {
			// Found update location, update the item direction
			line_item_data->direction = lower_iterator->second;
		}

		switch (line_item_data->direction) {
			case data::Transport_line_item_data::move_dir::up:
				line_item.position_y -= 0.01;
				break;
			case data::Transport_line_item_data::move_dir::right:
				line_item.position_x += 0.01;
				break;
			case data::Transport_line_item_data::move_dir::down:
				line_item.position_y += 0.01;
				break;
			case data::Transport_line_item_data::move_dir::left:
				line_item.position_x -= 0.01;
				break;

			case data::Transport_line_item_data::move_dir::stop:
				break;

			default:;
				assert(false);  // Invalid direction
		}


		// =============================================================================
		// Chunk transitions

		// Move item to another chunk if it has moved into another chunk
		if (line_item.position_y < 0.f - core::epsilon) {  // To Up
			line_item.position_y = 32.f + line_item.position_y;

			queue_up.second.push_back(std::move(line_item));
			layer.erase(layer.begin() + i);
		}
		else if (line_item.position_x > 32.f + core::epsilon) {  // Transfer to right
			line_item.position_x = line_item.position_x - 32.f;

			queue_right.second.push_back(std::move(line_item));
			layer.erase(layer.begin() + i);
		}
		else if (line_item.position_y > 32.f + core::epsilon) {  // To Down
			line_item.position_y = line_item.position_y - 32.f;

			queue_down.second.push_back(std::move(line_item));
			layer.erase(layer.begin() + i);
		}
		else if (line_item.position_x < 0.f - core::epsilon) {  // Transfer to Left
			line_item.position_x = 32.f + line_item.position_x;

			queue_left.second.push_back(std::move(line_item));
			layer.erase(layer.begin() + i);
		}

	}

	// Add to the chunk transition item queue if it the local queue has items
	if (!queue_up.second.empty())
		chunk_transition_items.push(std::move(queue_up));
	if (!queue_right.second.empty())
		chunk_transition_items.push(std::move(queue_right));
	if (!queue_down.second.empty())
		chunk_transition_items.push(std::move(queue_down));
	if (!queue_left.second.empty())
		chunk_transition_items.push(std::move(queue_left));
}

void jactorio::game::transport_line_c::logic_process_queued_items(std::queue<chunk_transition_item_queue>& chunk_transition_items) {
	while (!chunk_transition_items.empty()) {
		auto& pair = chunk_transition_items.front();

		// Add everything in pair.second into the vector at pair.first
		for (game::Chunk_object_layer& chunk_object_layer : pair.second) {
			pair.first->push_back(std::move(chunk_object_layer));
		}

		chunk_transition_items.pop();
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

void jactorio::game::transport_line_c::chunk_insert_item(Chunk* chunk, float position_x, float position_y,
														 data::Item* item) {
	auto& added_object =
		chunk->get_object(Chunk::object_layer::item_entity)
			.emplace_back(item, position_x, position_y,
						  item_width, item_width);

	auto* item_data = new data::Transport_line_item_data();
	added_object.unique_data = item_data;

	item_data->direction = data::Transport_line_item_data::move_dir::left;

	// Add current chunk to logic_chunks array if not in it
	// world_manager::logic_add_chunk(chunk);
}
