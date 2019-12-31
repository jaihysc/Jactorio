#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "core/debug/execution_timer.h"
#include "core/logger.h"

#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"
#include "game/event/event.h"

#include "renderer/gui/imgui_manager.h"
#include "data/prototype/item/item.h"
#include "data/data_manager.h"

// Register this to the game_tick event
void logic_loop() {
	using namespace jactorio::game;
	
	// Do things every logic loop tick
	input_manager::dispatch_input_callbacks();

	// Generate chunks
	world_generator::gen_chunk();

	mouse_selection::draw_cursor_selected_tile();
}


bool logic_loop_should_terminate = false;

const float move_speed = 0.9f;
void jactorio::game::init_logic_loop() {
	// Logic initialization here...
	logic_loop_should_terminate = false;
	
	// Movement controls
	input_manager::register_input_callback([]() {
		player_manager::move_player_y(move_speed * -1);
	}, GLFW_KEY_W, GLFW_PRESS);
	input_manager::register_input_callback([]() {
        player_manager::move_player_y(move_speed * -1);
    }, GLFW_KEY_W, GLFW_REPEAT);

	
	input_manager::register_input_callback([]() {
        player_manager::move_player_y(move_speed);
    }, GLFW_KEY_S, GLFW_PRESS);
	input_manager::register_input_callback([]() {
        player_manager::move_player_y(move_speed);
    }, GLFW_KEY_S, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
        player_manager::move_player_x(move_speed * -1);
    }, GLFW_KEY_A, GLFW_PRESS);
	input_manager::register_input_callback([]() {
        player_manager::move_player_x(move_speed * -1);
    }, GLFW_KEY_A, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
        player_manager::move_player_x(move_speed);
    }, GLFW_KEY_D, GLFW_PRESS);
	input_manager::register_input_callback([]() {
        player_manager::move_player_x(move_speed);
    }, GLFW_KEY_D, GLFW_REPEAT);


	// Menus
	input_manager::register_input_callback([]() {
		renderer::imgui_manager::show_debug_menu = !renderer::imgui_manager::show_debug_menu;
	}, GLFW_KEY_GRAVE_ACCENT, GLFW_RELEASE);
	input_manager::register_input_callback([]() {
		renderer::imgui_manager::show_inventory_menu = !renderer::imgui_manager::show_inventory_menu;
	}, GLFW_KEY_TAB, GLFW_RELEASE);


	// TODO REMOVE
	input_manager::register_input_callback([]() {
		data::item_stack* inventory = player_manager::player_inventory;

		// TODO REMOVE | Test data
		{
			using namespace data;
			auto x = data_manager::data_raw_get_all<Item>(data_category::item);
			inventory[0] = std::pair(x[0], 10);
			inventory[1] = std::pair(x[0], 8);
			inventory[4] = std::pair(x[0], 100);
			inventory[5] = std::pair(x[1], 2000);
		};
	}, GLFW_KEY_0, GLFW_RELEASE);

	Event::subscribe(event_type::logic_tick, logic_loop);
	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	while (!logic_loop_should_terminate) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		Event::raise(event_type::logic_tick);
		
		next_frame += std::chrono::nanoseconds(16666666);
		std::this_thread::sleep_until(next_frame);
	}
}

void jactorio::game::terminate_logic_loop() {
	logic_loop_should_terminate = true;
}
