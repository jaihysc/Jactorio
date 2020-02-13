#include "game/logic/transport_line_controller.h"

#include "core/float_math.h"

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
