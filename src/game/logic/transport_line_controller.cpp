#include "game/logic/transport_line_controller.h"

#include "core/float_math.h"
#include "game/world/world_manager.h"

void jactorio::game::transport_line_c::logic_update(Logic_chunk* l_chunk) {
	auto& layer = const_cast<std::vector<Chunk_object_layer>&>(
		l_chunk->chunk->objects[static_cast<int>(Chunk::object_layer::item_entity)]
	);

	auto& line_updates = l_chunk->transport_line_updates;
	
	for (auto& object : layer) {
		float lower_y = object.position_y - core::epsilon;
		float upper_y = object.position_y + core::epsilon;
		
		auto lower_iterator = line_updates.lower_bound({
			object.position_x - core::epsilon, 
			lower_y
		});
		auto upper_iterator = line_updates.upper_bound({
			object.position_x + core::epsilon,
			upper_y
		});

		// lower, upper x iterators found X values in range, now find iterator with y within epsilon range
		bool match = false;
		while (lower_iterator != upper_iterator) {
			const float y_val = lower_iterator->first.second;
			if (y_val >= lower_y && y_val < upper_y) {
				match = true;
				break;
			}
			++lower_iterator;
		}

		
		auto* line_item_data = static_cast<data::Transport_line_item_data*>(object.unique_data);
		if (match) {
			// Found update location, update the item direction
			line_item_data->direction = lower_iterator->second;
		}

		switch (line_item_data->direction) {
		case data::Transport_line_item_data::move_dir::up:
			object.position_y -= 0.01;
			break;
		case data::Transport_line_item_data::move_dir::right:
			object.position_x += 0.01;
			break;
		case data::Transport_line_item_data::move_dir::down:
			object.position_y += 0.01;
			break;
		case data::Transport_line_item_data::move_dir::left:
			object.position_x -= 0.01;
			break;
		default:
			;
		}
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
