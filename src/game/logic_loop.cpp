#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "core/debug/execution_timer.h"
#include "core/logger.h"

#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"
#include "game/world/world_manager.h"

#include "renderer/gui/imgui_manager.h"
#include "renderer/rendering/renderer.h"
#include "renderer/opengl/shader_manager.h"

bool logic_loop_should_terminate = false;

const float move_speed = 4.1f;
void jactorio::game::logic_loop() {
	// Logic initialization here...
	logic_loop_should_terminate = false;
	
	// Movement controls
	input_manager::register_input_callback([]() {
		player_manager::player_position_y -= move_speed;
	}, GLFW_KEY_W, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		player_manager::player_position_y -= move_speed;
	}, GLFW_KEY_W, GLFW_REPEAT);

	
	input_manager::register_input_callback([]() {
		player_manager::player_position_y += move_speed;
	}, GLFW_KEY_S, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		player_manager::player_position_y += move_speed;
	}, GLFW_KEY_S, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
		player_manager::player_position_x -= move_speed;
	}, GLFW_KEY_A, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		player_manager::player_position_x -= move_speed;
	}, GLFW_KEY_A, GLFW_REPEAT);
	
	input_manager::register_input_callback([]() {
		player_manager::player_position_x += move_speed;
	}, GLFW_KEY_D, GLFW_PRESS);
	input_manager::register_input_callback([]() {
		player_manager::player_position_x += move_speed;
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

		// Mouse selection
		{
			const int mouse_x_center = renderer::Renderer::get_window_width() / 2;
			const int mouse_y_center = renderer::Renderer::get_window_height() / 2;

			// Calculate number of pixels from center
			double pixels_from_center_x = mouse_selection::get_position_x() - mouse_x_center;
			double pixels_from_center_y = mouse_selection::get_position_y() - mouse_y_center;

			// TODO, account for MVP matrices - This does not work
			// pixels_from_center_x = (renderer::get_mvp_matrix() * glm::vec4(pixels_from_center_x)).x;
			// pixels_from_center_y = (renderer::get_mvp_matrix() * glm::vec4(pixels_from_center_y)).x;

			float world_x = player_manager::player_position_x;
			float world_y = player_manager::player_position_y;
			
			// Calculate tile position
			world_x += pixels_from_center_x / renderer::Renderer::tile_width;
			world_y += pixels_from_center_y / renderer::Renderer::tile_width;

			float chunk_index_x = world_x / 32;
			float chunk_index_y = world_y / 32;

			// There is no 0, 0 in negative chunks, thus subtract 1 if negative
			if (chunk_index_x < 0) {
				chunk_index_x -= 1;
				world_x -= 1;
			}
			if (chunk_index_y < 0) {
				chunk_index_y -= 1;
				world_y -= 1;
			}

			auto* chunk = world_manager::get_chunk(
				static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));
			
			if (chunk != nullptr) {
				int tile_index_x = static_cast<int>(world_x) % 32;
				int tile_index_y = static_cast<int>(world_y) % 32;

				// Chunk is 32 tiles
				if (tile_index_x < 0) {
					tile_index_x = 32 - tile_index_x * -1;
				}
				if (tile_index_y < 0) {
					tile_index_y = 32 - tile_index_y * -1;
				}

				// LOG_MESSAGE_f(debug, "%d %d", chunk_index_y, tile_index_y);

			
				auto& tile = chunk->tiles_ptr()[32 * tile_index_y + tile_index_x];
				tile.set_tile_prototype(Chunk_tile::prototype_category::base, nullptr);
			}
		}
		
		next_frame += std::chrono::nanoseconds(16666666);
		std::this_thread::sleep_until(next_frame);
	}
}

void jactorio::game::terminate_logic_loop() {
	logic_loop_should_terminate = true;
}
