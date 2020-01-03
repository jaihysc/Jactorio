#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "jactorio.h"

#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"
#include "game/event/event.h"

#include "renderer/gui/imgui_manager.h"
#include "data/prototype/item/item.h"
#include "data/data_manager.h"

void logic_loop() {
	using namespace jactorio::game;

	// Do things every logic loop tick
	input_manager::raise();

	// Generate chunks
	world_generator::gen_chunk();

	// jactorio::data::item_stack* ptr;
	// if ((ptr = player_manager::get_selected_item()) != nullptr) {
		// mouse_selection::draw_tile_at_cursor(ptr->first->sprite->name);
	// }
	// else
		mouse_selection::draw_selection_box();
}


bool logic_loop_should_terminate = false;

const float move_speed = 0.9f;

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
