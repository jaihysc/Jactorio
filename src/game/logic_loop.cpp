// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include "game/logic_loop.h"

#include <chrono>
#include <filesystem>
#include <thread>

#include "jactorio.h"

#include "core/filesystem.h"
#include "data/data_manager.h"
#include "data/prototype/entity/transport/transport_line.h"
#include "game/event/event.h"
#include "game/game_data.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_line_structure.h"
#include "game/player/player_data.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/render_main.h"

constexpr float move_speed = 0.8f;

void jactorio::game::init_logic_loop() {
	core::Resource_guard<void> loop_termination_guard([]() {
		renderer::render_thread_should_exit = true;
		logic_thread_should_exit = true;
	});

	// Initialize game data
	core::Resource_guard<void> game_data_guard([]() { delete game_data; });
	game_data = new Game_data();

	// Load prototype data
	core::Resource_guard data_manager_guard(&data::data_manager::clear_data);
	try {
		data::data_manager::load_data(core::filesystem::resolve_path("~/data"));
	}
	catch (data::Data_exception&) {
		// Prototype loading error
		return;
	}
	catch (std::filesystem::filesystem_error&) {
		// Data folder not found error
		LOG_MESSAGE_f(error, "data/ folder not found at %s", core::filesystem::resolve_path("~/data").c_str());
		return;
	}

	// ======================================================================
	// Temporary Startup settings
	game_data->player.set_player_world(&game_data->world);  // Main world is player's world


	// ======================================================================

	// Movement controls
	input_manager::subscribe([]() {
		game_data->player.move_player_y(move_speed * -1);
	}, GLFW_KEY_W, GLFW_PRESS);
	input_manager::subscribe([]() {
		game_data->player.move_player_y(move_speed * -1);
	}, GLFW_KEY_W, GLFW_REPEAT);


	input_manager::subscribe([]() {
		game_data->player.move_player_y(move_speed);
	}, GLFW_KEY_S, GLFW_PRESS);
	input_manager::subscribe([]() {
		game_data->player.move_player_y(move_speed);
	}, GLFW_KEY_S, GLFW_REPEAT);

	input_manager::subscribe([]() {
		game_data->player.move_player_x(move_speed * -1);
	}, GLFW_KEY_A, GLFW_PRESS);
	input_manager::subscribe([]() {
		game_data->player.move_player_x(move_speed * -1);
	}, GLFW_KEY_A, GLFW_REPEAT);

	input_manager::subscribe([]() {
		game_data->player.move_player_x(move_speed);
	}, GLFW_KEY_D, GLFW_PRESS);
	input_manager::subscribe([]() {
		game_data->player.move_player_x(move_speed);
	}, GLFW_KEY_D, GLFW_REPEAT);


	{
		using namespace renderer::imgui_manager;
		// Menus
		input_manager::subscribe([]() {
			set_window_visibility(guiWindow::debug, !get_window_visibility(guiWindow::debug));
		}, GLFW_KEY_GRAVE_ACCENT, GLFW_RELEASE);
		input_manager::subscribe([]() {
			// If a layer is already activated, deactivate it, otherwise open the gui menu
			if (game_data->player.get_activated_layer() != nullptr)
				game_data->player.set_activated_layer(nullptr);
			else
				set_window_visibility(guiWindow::character, !get_window_visibility(guiWindow::character));
		}, GLFW_KEY_TAB, GLFW_RELEASE);
	}

	{
		// Rotating orientation	
		input_manager::subscribe([]() {
			game_data->player.rotate_placement_orientation();
		}, GLFW_KEY_R, GLFW_RELEASE);
		input_manager::subscribe([]() {
			game_data->player.counter_rotate_placement_orientation();
		}, GLFW_KEY_R, GLFW_RELEASE, GLFW_MOD_SHIFT);


		// Place entities
		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured || !game_data->player.mouse_selected_tile_in_range())
				return;

			const auto tile_selected = game_data->player.get_mouse_tile_coords();
			game_data->player.try_place_entity(game_data->world,
			                                   tile_selected.first, tile_selected.second);
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS);

		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured || !game_data->player.mouse_selected_tile_in_range())
				return;

			const auto tile_selected = game_data->player.get_mouse_tile_coords();
			game_data->player.try_place_entity(game_data->world,
			                                   tile_selected.first, tile_selected.second, true);
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS_FIRST);

		// Remove entities or mine resource
		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured || !game_data->player.mouse_selected_tile_in_range())
				return;

			const auto tile_selected = game_data->player.get_mouse_tile_coords();
			game_data->player.try_pickup(game_data->world,
			                             tile_selected.first, tile_selected.second);
		}, GLFW_MOUSE_BUTTON_2, GLFW_PRESS);
	}

	//

	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	unsigned short logic_tick = 1;
	while (!logic_thread_should_exit) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		// ======================================================================
		// LOGIC LOOP ======================================================================
		{
			EXECUTION_PROFILE_SCOPE(logic_update_timer, "Logic update");

			// ======================================================================	
			// World chunks			
			{
				std::lock_guard<std::mutex> guard{game_data->world.world_data_mutex};

				game_data->world.on_tick_advance();
				game_data->player.mouse_calculate_selected_tile();

				game_data->world.gen_chunk();
				game_data->input.mouse.draw_cursor_overlay(game_data->player);


				// Logistics logic
				EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

				transport_line_c::transport_line_logic_update(game_data->world);
			}

			// ======================================================================
			// Player logic
			{
				std::lock_guard<std::mutex> guard{game_data->player.mutex};

				game_data->player.recipe_craft_tick();
			}

			// Events are responsible for resource locking themselves
			Event::raise<Logic_tick_event>(event_type::logic_tick, logic_tick);
			input_manager::raise();
		}
		// ======================================================================
		// ======================================================================

		if (++logic_tick > 60)
			logic_tick = 1;

		auto time_end = std::chrono::steady_clock::now();
		while (time_end > next_frame) {
			next_frame += std::chrono::nanoseconds(16666666);
		}
		std::this_thread::sleep_until(next_frame);
	}

	LOG_MESSAGE(info, "Logic thread exited");
}
