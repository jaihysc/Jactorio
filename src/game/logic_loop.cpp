#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "jactorio.h"

#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"

#include "data/data_manager.h"

#include "renderer/gui/imgui_manager.h"
#include "game/world/world_manager.h"

bool logic_loop_should_terminate = false;

constexpr float move_speed = 0.8f;

// TODO move this elsewhere
// This doesn't work with large amounts of items, having to copy and move them (2/11/2020)
// std::list<uint32_t> up;
// std::list<uint32_t> right;
// std::list<uint32_t> down;
// std::list<uint32_t> left;

// Stores the item direction
/**
 * Directions:
 * 0 = none,
 * 1 = up,
 * 2 = right,
 * 3 = down,
 * 4 = left
 */
// std::map<uint32_t, uint8_t> item_direction;

bool float_same(float a, float b) {
	return fabs(a - b) < 0.00005;  // Epsilon may need to be adjusted
}

void jactorio::game::init_logic_loop() {
	// Movement controls
	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed * -1);
	}, GLFW_KEY_W, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed * -1);
	}, GLFW_KEY_W, GLFW_REPEAT);


	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed);
	}, GLFW_KEY_S, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed);
	}, GLFW_KEY_S, GLFW_REPEAT);

	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed * -1);
	}, GLFW_KEY_A, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed * -1);
	}, GLFW_KEY_A, GLFW_REPEAT);

	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed);
	}, GLFW_KEY_D, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed);
	}, GLFW_KEY_D, GLFW_REPEAT);


	{
		using namespace renderer::imgui_manager;
		// Menus
		input_manager::subscribe([]() {
			set_window_visibility(gui_window::debug, !get_window_visibility(gui_window::debug));
		}, GLFW_KEY_GRAVE_ACCENT, GLFW_RELEASE);
		input_manager::subscribe([]() {
			// If a layer is already activated, deactivate it, otherwise open the gui menu
			if (player_manager::get_activated_layer() != nullptr)
				player_manager::set_activated_layer(nullptr);
			else
				set_window_visibility(gui_window::character, !get_window_visibility(gui_window::character));
		}, GLFW_KEY_TAB, GLFW_RELEASE);
	}

	{
		// Place entities
		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured)
				return;

			const auto tile_selected = mouse_selection::get_mouse_tile_coords();
			player_manager::try_place(tile_selected.first, tile_selected.second);
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS);

		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured)
				return;

			const auto tile_selected = mouse_selection::get_mouse_tile_coords();
			player_manager::try_place(tile_selected.first, tile_selected.second, true);
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS_FIRST);

		// Remove entities or mine resource
		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured)
				return;

			const auto tile_selected = mouse_selection::get_mouse_tile_coords();
			player_manager::try_pickup(tile_selected.first, tile_selected.second);
		}, GLFW_MOUSE_BUTTON_2, GLFW_PRESS);
	}


	// TODO remove this
	input_manager::subscribe([]() {
		Event::subscribe_once(event_type::logic_tick, []() {
			auto* chunk = world_manager::get_chunk(0, 0);

			auto& logic_chunks = world_manager::logic_get_all_chunks();
			// Check if chunk has already been added
			Logic_chunk* logic_chunk;

			bool found = false;
			for (auto& clunk : logic_chunks) {
				if (clunk.chunk == chunk) {
					found = true;
					logic_chunk = &clunk;
					break;
				}
			}
			if (!found) {
				logic_chunk = &world_manager::logic_add_chunk(chunk);
			}


			auto& oof = chunk->objects[0];
			auto proto = data::data_manager::data_raw_get<data::Entity>(data::data_category::container_entity,
			                                                            "__base__/steel-chest");
			// BUG everything which modifies chunk data is not thread safe. Merely luck that it managed
			// to complete before the renderer reads incomplete memory or object

			for (int i = 0; i < 1; ++i) {
				auto* added_item = &oof.emplace_back(proto, 3.2f, 5.2f, 0.3, 0.3);
				logic_chunk->item_direction[oof.size() - 1] = 4;

				// BUG emplacing back destroys left?!?!?1 (OOOH! IT IS BECAUES THE VECTOR RESIZED!!)

				// TODO just store move direction with the object layer?
				// no, keep the same idea but with indices (2/11/20)
			}
		});
	}, GLFW_KEY_Q, GLFW_RELEASE);


	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	unsigned short logic_tick = 1;
	while (!logic_loop_should_terminate) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		{
			mouse_selection::calculate_mouse_tile_coords();

			// Do things every logic loop tick
			Event::raise<Logic_tick_event>(event_type::logic_tick, logic_tick);

			input_manager::raise();

			// Generate chunks
			world_generator::gen_chunk();

			mouse_selection::draw_cursor_overlay();

			player_manager::recipe_craft_tick();

			// TODO test only with belt logic
			{
				EXECUTION_PROFILE_SCOPE(belt_timer, "belt update");

				/**
				 * Measured performance (Debug config):
				 * Copying 120 000 objects: 7ms
				 *		This makes it feasible to store items for transport belts based on the chunk(or tile)
				 *		they reside in
				 * 
				 */

				// Each item on belt gets a SINGLE float of the distance to the next item / end of belt
				// 
				// 4 vectors exist for items moving in the 4 directions
				// Upon passing a update tile (where the belt bends), the logic will move the belt into a
				// new vector matching the new direction of travel
				// * (std::list appears to be a better data type for this)
				//
				// Update tiles are iterated over by the update logic, since items are stored per chunk. It
				// iterates through all items of a chunk, checking if any of the conditions are met
				// with a 32 x 32 chunk and 9 items per chunk: (9216 iterations max) (~120 chunks average)
				//
				// 
				//
				// After exiting a chunk, it needs to be moved to the next chunk
				// auto* chunk_boi = world_manager::get_chunk(-3, 0);
				// if (chunk_boi) {
				// 	auto& x = chunk_boi->objects[0];
				// 	int i = 0;
				// 	for (auto object : x) {
				// 		object.position_x -= 0.1;
				// 		x[i] = object;
				// 		++i;
				// 	}
				// }

				for (auto& logic_chunk : world_manager::logic_get_all_chunks()) {
					auto& layer = const_cast<std::vector<Chunk_object_layer>&>(logic_chunk.chunk->objects[0]);

					for (auto i = 0; i < layer.size(); ++i) {
						auto& object = layer[i];
						auto& direction = logic_chunk.item_direction[i];

						// left to up
						// LOG_MESSAGE_f(debug, "%f %f", object.position_x, object.position_y);
						if (float_same(object.position_x, 0.8f) && float_same(object.position_y, 5.2f)) {
							// LOG_MESSAGE(debug, "Match 1");
							direction = 1;
						}

							// up to right
						else if (float_same(object.position_x, 0.8f) && float_same(object.position_y, 0.8f)) {
							// LOG_MESSAGE(debug, "Match 2");
							direction = 2;
						}

							// right to down
						else if (float_same(object.position_x, 4.2f) && float_same(object.position_y, 0.8f)) {
							// LOG_MESSAGE(debug, "Match 3");
							direction = 3;
						}

							// down to left
						else if (float_same(object.position_x, 4.2f) && float_same(object.position_y, 5.2f)) {
							// LOG_MESSAGE(debug, "Match 4");
							direction = 4;
						}

						switch (direction) {
						case 1:
							object.position_y -= 0.01f;
							break;
						case 2:
							object.position_x += 0.01f;
							break;
						case 3:
							object.position_y += 0.01f;
							break;
						case 4:
							object.position_x -= 0.01f;
							break;
						default:
							;
						}
					}
				}
				// auto* chunk = world_manager::get_chunk(0, 0);
				// if (chunk) {
				// auto& layer = chunk->objects[0];
				// Handle event tiles
				// for (auto i = 0; i < layer.size(); ++i) {
				// 	auto& object = layer[i];
				// 	// left to up
				// 	// LOG_MESSAGE_f(debug, "%f %f", object.position_x, object.position_y);
				// 	if (float_same(object.position_x, 0.8f) && float_same(object.position_y, 5.2f)) {
				// 		LOG_MESSAGE(debug, "Match 1");
				// 		left.remove(i);
				// 		up.push_back(i);
				// 	}
				//
				// 	// up to right
				// 	if (float_same(object.position_x, 0.8f) && float_same(object.position_y, 0.8f)) {
				// 		LOG_MESSAGE(debug, "Match 2");
				//
				// 		up.remove(i);
				// 		right.push_back(i);
				// 	}
				// 	
				// 	// right to down
				// 	if (float_same(object.position_x, 4.2f) && float_same(object.position_y, 0.8f)) {
				// 		LOG_MESSAGE(debug, "Match 3");
				//
				// 		right.remove(i);
				// 		down.push_back(i);
				// 	}
				// 	
				// 	// down to left
				// 	if (float_same(object.position_x, 4.2f) && float_same(object.position_y, 5.2f)) {
				// 		LOG_MESSAGE(debug, "Match 4");
				// 		// LOG_MESSAGE_f(debug, "%f %f", object.position_x, object.position_y);
				//
				// 		down.remove(i);
				// 		left.push_back(i);
				// 	}
				// }
				//
				// // Move items
				// for (auto& index : up) {
				// 	layer[index].position_y -= 0.01f;
				// }
				// for (auto& index : right) {
				// 	layer[index].position_x += 0.01f;
				// }
				// for (auto& index : down) {
				// 	layer[index].position_y += 0.01f;
				// }
				// for (auto& index : left) {
				// 	layer[index].position_x -= 0.01f;
				// }
				//
				// LOG_MESSAGE_f(debug, "%d", layer.size());
				// }
			}
		}

		if (++logic_tick > 60)
			logic_tick = 1;

		next_frame += std::chrono::nanoseconds(16666666);
		std::this_thread::sleep_until(next_frame);
	}
}

void jactorio::game::terminate_logic_loop() {
	logic_loop_should_terminate = true;
}
