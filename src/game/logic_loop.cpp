#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "jactorio.h"

#include "renderer/gui/imgui_manager.h"

#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"
#include "game/world/world_manager.h"
#include "game/logic/transport_line_controller.h"

#include "data/data_manager.h"
#include "data/prototype/entity/transport/transport_line_item.h"

bool logic_loop_should_terminate = false;

constexpr float move_speed = 0.8f;

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

			auto& object_layer = chunk->objects[static_cast<int>(Chunk::object_layer::item_entity)];
			auto proto =
				data::data_manager::data_raw_get<data::Transport_line_item>(data::data_category::item,
				                                                            "__base__/transport-belt-basic-item");
			for (int i = 0; i < 1; ++i) {
				auto* added_item = &object_layer.emplace_back(proto, 3.f, 5.1f, 0.4, 0.4);
				logic_chunk->item_direction[object_layer.size() - 1] = 4;  // Moving left
			}
		});
	}, GLFW_KEY_Q, GLFW_RELEASE);

	
	// BUG everything which modifies chunk data is not thread safe. Merely luck that it managed
	// to complete before the renderer reads incomplete memory or object

	
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

			// Logistics logic
			{
				EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

				for (auto& logic_chunk : world_manager::logic_get_all_chunks()) {
					transport_line_c::logic_update(&logic_chunk);
				}
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
