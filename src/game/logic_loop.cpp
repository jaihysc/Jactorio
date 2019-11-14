#include "game/logic_loop.h"

#include "core/loop_manager.h"
#include "game/input/input_manager.h"
#include "core/logger.h"
#include "renderer/rendering/mvp_manager.h"

namespace jactorio::game
{
	bool run_logic_loop = false;
}

void jactorio::game::logic_loop() {
	// Logic initialization here...

	// Temporary camera controls
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->y += 10;
	}, GLFW_KEY_W, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->y += 10;
	}, GLFW_KEY_W, GLFW_REPEAT);

	
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->y -= 10;
	}, GLFW_KEY_S, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->y -= 10;
	}, GLFW_KEY_S, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->x += 10;
	}, GLFW_KEY_A, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->x += 10;
	}, GLFW_KEY_A, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->x -= 10;
	}, GLFW_KEY_D, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		renderer::mvp_manager::get_view_transform()->x -= 10;
	}, GLFW_KEY_D, GLFW_REPEAT);

	
	core::loop_manager::logic_loop_ready([] {
		run_logic_loop = true;
	});

	while (!core::loop_manager::loop_manager_terminated()) {
		if (run_logic_loop) {
			run_logic_loop = false;

			// Do things every logic loop tick
			input_manager::dispatch_input_callbacks();

			core::loop_manager::logic_loop_complete();
		}
	}
}
