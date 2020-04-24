// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include "game/logic_loop.h"

#include <chrono>
#include <filesystem>
#include <thread>
#include <GLFW/glfw3.h>

#include "jactorio.h"
#include "core/filesystem.h"
#include "data/data_manager.h"
#include "game/game_data.h"
#include "game/logic/transport_line_controller.h"
#include "renderer/render_main.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/window/window_manager.h"

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
	core::Resource_guard data_manager_guard(&data::clear_data);
	try {
		data::load_data(core::resolve_path("~/data"));
	}
	catch (data::Data_exception&) {
		// Prototype loading error
		return;
	}
	catch (std::filesystem::filesystem_error&) {
		// Data folder not found error
		LOG_MESSAGE_f(error, "data/ folder not found at %s", core::resolve_path("~/data").c_str());
		return;
	}

	// ======================================================================
	// Temporary Startup settings
	game_data->player.set_player_world(&game_data->world);  // Main world is player's world


	// ======================================================================

	// Movement controls
	game_data->input.key.subscribe([]() {
		game_data->player.move_player_y(move_speed * -1);
	}, inputKey::w, inputAction::key_held);


	game_data->input.key.subscribe([]() {
		game_data->player.move_player_y(move_speed);
	}, inputKey::s, inputAction::key_held);

	game_data->input.key.subscribe([]() {
		game_data->player.move_player_x(move_speed * -1);
	}, inputKey::a, inputAction::key_held);

	game_data->input.key.subscribe([]() {
		game_data->player.move_player_x(move_speed);
	}, inputKey::d, inputAction::key_held);


	{
		using namespace renderer::imgui_manager;
		// Menus
		game_data->input.key.subscribe([]() {
			set_window_visibility(game_data->event, guiWindow::debug, !get_window_visibility(guiWindow::debug));
		}, inputKey::grave, inputAction::key_up);

		game_data->input.key.subscribe([]() {
			// If a layer is already activated, deactivate it, otherwise open the gui menu
			if (game_data->player.get_activated_layer() != nullptr)
				game_data->player.set_activated_layer(nullptr);
			else
				set_window_visibility(game_data->event, guiWindow::character, !get_window_visibility(guiWindow::character));
		}, inputKey::tab, inputAction::key_up);
	}

	{
		// Rotating orientation	
		game_data->input.key.subscribe([]() {
			game_data->player.rotate_placement_orientation();
		}, inputKey::r, inputAction::key_up);
		game_data->input.key.subscribe([]() {
			game_data->player.counter_rotate_placement_orientation();
		}, inputKey::r, inputAction::key_up, inputMod::shift);


		game_data->input.key.subscribe([]() {
			game_data->player.deselect_selected_item();
		}, inputKey::q, inputAction::key_down);

		// Place entities
		game_data->input.key.subscribe([]() {
			if (renderer::imgui_manager::input_captured || !game_data->player.mouse_selected_tile_in_range())
				return;

			const auto tile_selected = game_data->player.get_mouse_tile_coords();
			game_data->player.try_place_entity(game_data->world,
			                                   tile_selected.first, tile_selected.second);
		}, inputKey::mouse1, inputAction::key_held);

		game_data->input.key.subscribe([]() {
			if (renderer::imgui_manager::input_captured || !game_data->player.mouse_selected_tile_in_range())
				return;

			const auto tile_selected = game_data->player.get_mouse_tile_coords();
			game_data->player.try_place_entity(game_data->world,
			                                   tile_selected.first, tile_selected.second, true);
		}, inputKey::mouse1, inputAction::key_down);

		// Remove entities or mine resource
		game_data->input.key.subscribe([]() {
			if (renderer::imgui_manager::input_captured || !game_data->player.mouse_selected_tile_in_range())
				return;

			const auto tile_selected = game_data->player.get_mouse_tile_coords();
			game_data->player.try_pickup(game_data->world,
			                             tile_selected.first, tile_selected.second);
		}, inputKey::mouse2, inputAction::key_held);
	}

	game_data->input.key.subscribe([]() {
		glfwSetWindowShouldClose(renderer::window_manager::get_window(), GL_TRUE);

	}, inputKey::escape, inputAction::key_up);


	game_data->input.key.subscribe([]() {
		game_data->event.subscribe_once(eventType::renderer_tick, []() {
			game_data->world = World_data();
		});
	}, inputKey::p, inputAction::key_up);

	//

	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
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

				transport_line_logic_update(game_data->world);
			}

			// ======================================================================
			// Player logic
			{
				std::lock_guard<std::mutex> guard{game_data->player.mutex};

				game_data->player.recipe_craft_tick();
			}

			// Events are responsible for resource locking themselves
			game_data->event.raise<Logic_tick_event>(eventType::logic_tick, game_data->world.game_tick() % JC_GAME_HERTZ);
			game_data->input.key.raise();
		}
		// ======================================================================
		// ======================================================================

		auto time_end = std::chrono::steady_clock::now();
		while (time_end > next_frame) {
			next_frame += std::chrono::nanoseconds(16666666);
		}
		std::this_thread::sleep_until(next_frame);
	}

	LOG_MESSAGE(info, "Logic thread exited");
}
