#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "core/logger.h"
#include "core/debug/execution_timer.h"

#include "game/input/input_manager.h"
#include "game/world/world_generator.h"
#include "renderer/rendering/world_renderer.h"
#include "renderer/gui/imgui_manager.h"

bool logic_loop_should_terminate = false;

const long long move_speed = 10;
void jactorio::game::logic_loop() {
	// Logic initialization here...
	logic_loop_should_terminate = false;
	
	// Movement controls
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_y -= move_speed;
	}, GLFW_KEY_W, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_y -= move_speed;
	}, GLFW_KEY_W, GLFW_REPEAT);

	
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_y += move_speed;
	}, GLFW_KEY_S, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_y += move_speed;
	}, GLFW_KEY_S, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_x -= move_speed;
	}, GLFW_KEY_A, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_x -= move_speed;
	}, GLFW_KEY_A, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_x += move_speed;
	}, GLFW_KEY_D, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::world_renderer::player_position_x += move_speed;
	}, GLFW_KEY_D, GLFW_REPEAT);


	// Debug menu
	input_manager::register_input_callback([]() {
		renderer::imgui_manager::show_debug_menu = !renderer::imgui_manager::show_debug_menu;
	}, GLFW_KEY_GRAVE_ACCENT, GLFW_RELEASE);

	
	auto next_frame = std::chrono::steady_clock::now();
	while (!logic_loop_should_terminate) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		// Do things every logic loop tick
		input_manager::dispatch_input_callbacks();

		// Generate chunks
		world_generator::gen_chunk();

		
		next_frame += std::chrono::nanoseconds(16666666);
		std::this_thread::sleep_until(next_frame);
	}
}

void jactorio::game::terminate_logic_loop() {
	logic_loop_should_terminate = true;
}
