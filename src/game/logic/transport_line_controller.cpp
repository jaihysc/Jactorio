#include "game/logic/transport_line_controller.h"

#include "core/float_math.h"
#include "game/world/world_manager.h"

// TODO Instead of terating through a list of positions in order to determine an item direction change is valid,
// store them behind a std::map, with a custom float comparison function using core::f_eq.
// This gives a faster O(log n) speed over O(n)

void jactorio::game::transport_line_c::logic_update(Logic_chunk* l_chunk) {
	auto& layer = const_cast<std::vector<Chunk_object_layer>&>(
		l_chunk->chunk->objects[static_cast<int>(Chunk::object_layer::item_entity)]
	);

	for (unsigned long long i = 0; i < layer.size(); ++i) {
		auto& object = layer[i];
		auto& direction = l_chunk->item_direction[i];

		// vvvv TODO make modular
		
		// left to up
		// LOG_MESSAGE_f(debug, "%f %f", object.position_x, object.position_y);
		if (core::f_eq(object.position_x, 0.5f) && core::f_eq(object.position_y, 5.1f)) {
			// LOG_MESSAGE(debug, "Match 1");
			direction = 1;
		}

			// up to right
		else if (core::f_eq(object.position_x, 0.5f) && core::f_eq(object.position_y, 0.5f)) {
			// LOG_MESSAGE(debug, "Match 2");
			direction = 2;
		}

			// right to down
		else if (core::f_eq(object.position_x, 4.1f) && core::f_eq(object.position_y, 0.5f)) {
			// LOG_MESSAGE(debug, "Match 3");
			direction = 3;
		}

			// down to left
		else if (core::f_eq(object.position_x, 4.1f) && core::f_eq(object.position_y, 5.1f)) {
			// LOG_MESSAGE(debug, "Match 4");
			direction = 4;
		}

		// ^^^^ TODO make modular

		switch (direction) {
		case 1:
			object.position_y -= 0.01;
			break;
		case 2:
			object.position_x += 0.01;
			break;
		case 3:
			object.position_y += 0.01;
			break;
		case 4:
			object.position_x -= 0.01;
			break;
		default:
			;
		}
	}
}


// Item insertion

void jactorio::game::transport_line_c::belt_insert_item_l(const int tile_x, const int tile_y,
                                                          data::Sprite* item_sprite) {
	world_manager::get_chunk_world_coords(tile_x, tile_y)
		->get_object(Chunk::object_layer::item_entity)
		.emplace_back(item_sprite, 
		              static_cast<float>(tile_x) + 0.3f, static_cast<float>(tile_y) + 0.1f,
		              item_width, item_width);

	// TODO need to add current chunk to logic_chunks array if no tin it
	// TODO need to give the item a direction

	// TODO logic chunks turns into map
}

void jactorio::game::transport_line_c::belt_insert_item_r(int tile_x, int tile_y, data::Sprite* item) {
	
}
