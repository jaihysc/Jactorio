#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "jactorio.h"

#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/entity_place_controller.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"
#include "game/world/world_manager.h"

#include "data/data_manager.h"
#include "data/prototype/item/item.h"
#include "renderer/gui/imgui_manager.h"

void logic_loop() {
	using namespace jactorio::game;

	// Do things every logic loop tick
	input_manager::raise();

	// Generate chunks
	world_generator::gen_chunk();

	mouse_selection::draw_cursor_overlay();
}


bool logic_loop_should_terminate = false;

const float move_speed = 0.1f;

int test_rm_counter = 0;

void jactorio::game::init_logic_loop() {
	// Logic initialization here...
	logic_loop_should_terminate = false;

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
			set_window_visibility(gui_window::character, !get_window_visibility(gui_window::character));
		}, GLFW_KEY_TAB, GLFW_RELEASE);
	}

	// TODO REMOVE | Test data
	Event::subscribe(event_type::game_gui_open, []() {
		data::item_stack* inventory = player_manager::player_inventory;

		using namespace data;
		auto x = data_manager::data_raw_get_all<Item>(data_category::item);
		inventory[0] = std::pair(x[0], 10);
		inventory[1] = std::pair(x[0], 8);
		inventory[4] = std::pair(x[0], 100);
		inventory[5] = std::pair(x[1], 2000);
	});

	// TODO MOVE, test entity placement
	{
		input_manager::subscribe([]() {
			// Place
			data::item_stack* ptr;
			if ((ptr = player_manager::get_selected_item()) != nullptr) {
				auto* entity_ptr = static_cast<data::Entity*>(ptr->first->entity_prototype);

				// Entities only
				const auto tile_selected = mouse_selection::get_mouse_selected_tile();
				if (entity_ptr != nullptr) {
					logic::place_entity_at_coords(entity_ptr, tile_selected.first, tile_selected.second);
				}
			}
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS);
		input_manager::subscribe([]() {
			// Remove
			// Entities only
			const auto tile_selected = mouse_selection::get_mouse_selected_tile();
			const auto entity_ptr = world_manager::get_tile_world_coords(
				tile_selected.first, tile_selected.second)->entity;
			
			if (entity_ptr != nullptr) {
				test_rm_counter++;
				if (test_rm_counter == 60) {
					test_rm_counter = 0;
					logic::place_entity_at_coords(nullptr, tile_selected.first, tile_selected.second);
				}
			}
		}, GLFW_MOUSE_BUTTON_2, GLFW_PRESS);
	}

	
	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	while (!logic_loop_should_terminate) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		logic_loop();

		next_frame += std::chrono::nanoseconds(16666666);
		std::this_thread::sleep_until(next_frame);
	}
}

void jactorio::game::terminate_logic_loop() {
	logic_loop_should_terminate = true;
}
