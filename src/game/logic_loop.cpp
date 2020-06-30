// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include "game/logic_loop.h"

#include <chrono>
#include <filesystem>
#include <thread>

#include "jactorio.h"
#include "core/filesystem.h"
#include "data/data_manager.h"
#include "data/prototype/entity/inserter.h"
#include "game/game_data.h"
#include "game/logic/transport_line_controller.h"
#include "renderer/render_main.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/window/window_manager.h"

constexpr float kMoveSpeed = 0.8f;

void jactorio::game::InitLogicLoop() {
	core::ResourceGuard<void> loop_termination_guard([]() {
		renderer::render_thread_should_exit = true;
		logic_thread_should_exit            = true;
	});

	// Initialize game data
	core::ResourceGuard<void> game_data_guard([]() { delete game_data; });
	game_data                 = new GameData();
	data::active_data_manager = &game_data->prototype;

	try {
		game_data->prototype.LoadData(core::ResolvePath("data"));
	}
	catch (data::DataException&) {
		// Prototype loading error
		return;
	}
	catch (std::filesystem::filesystem_error&) {
		// Data folder not found error
		LOG_MESSAGE_f(error, "data/ folder not found at %s", core::ResolvePath("data").c_str());
		return;
	}

	LOG_MESSAGE(info, "Prototype loading complete");
	prototype_loading_complete = true;

	// ======================================================================
	// Temporary Startup settings
	game_data->player.SetPlayerWorld(&game_data->world);  // Main world is player's world


	// ======================================================================

	// Movement controls
	game_data->input.key.Register([]() {
		game_data->player.MovePlayerY(kMoveSpeed * -1);
	}, SDLK_w, InputAction::key_held);


	game_data->input.key.Register([]() {
		game_data->player.MovePlayerY(kMoveSpeed);
	}, SDLK_s, InputAction::key_held);

	game_data->input.key.Register([]() {
		game_data->player.MovePlayerX(kMoveSpeed * -1);
	}, SDLK_a, InputAction::key_held);

	game_data->input.key.Register([]() {
		game_data->player.MovePlayerX(kMoveSpeed);
	}, SDLK_d, InputAction::key_held);


	// Menus
	game_data->input.key.Register([]() {
		SetVisible(renderer::Menu::DebugMenu,
		           !IsVisible(renderer::Menu::DebugMenu));
	}, SDLK_BACKQUOTE, InputAction::key_up);

	game_data->input.key.Register([]() {
		// If a layer is already activated, deactivate it, otherwise open the gui menu
		if (game_data->player.GetActivatedLayer() != nullptr)
			game_data->player.SetActivatedLayer(nullptr);
		else
			SetVisible(renderer::Menu::CharacterMenu,
			           !IsVisible(renderer::Menu::CharacterMenu));

	}, SDLK_TAB, InputAction::key_up);


	// Rotating orientation	
	game_data->input.key.Register([]() {
		game_data->player.RotatePlacementOrientation();
	}, SDLK_r, InputAction::key_up);
	game_data->input.key.Register([]() {
		game_data->player.CounterRotatePlacementOrientation();
	}, SDLK_r, InputAction::key_up, KMOD_LSHIFT);


	game_data->input.key.Register([]() {
		game_data->player.DeselectSelectedItem();
	}, SDLK_q, InputAction::key_down);

	// Place entities
	game_data->input.key.Register([]() {
		if (renderer::input_captured || !game_data->player.MouseSelectedTileInRange())
			return;

		const auto tile_selected = game_data->player.GetMouseTileCoords();
		game_data->player.TryPlaceEntity(game_data->world,
		                                 tile_selected.first, tile_selected.second);
	}, MouseInput::left, InputAction::key_held);

	game_data->input.key.Register([]() {
		if (renderer::input_captured || !game_data->player.MouseSelectedTileInRange())
			return;

		const auto tile_selected = game_data->player.GetMouseTileCoords();
		game_data->player.TryPlaceEntity(game_data->world,
		                                 tile_selected.first, tile_selected.second, true);
	}, MouseInput::left, InputAction::key_down);

	// Remove entities or mine resource
	game_data->input.key.Register([]() {
		if (renderer::input_captured || !game_data->player.MouseSelectedTileInRange())
			return;

		const auto tile_selected = game_data->player.GetMouseTileCoords();
		game_data->player.TryPickup(game_data->world,
		                            tile_selected.first, tile_selected.second);
	}, MouseInput::right, InputAction::key_held);


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
				std::lock_guard<std::mutex> guard{game_data->world.worldDataMutex};

				game_data->world.OnTickAdvance();
				game_data->player.MouseCalculateSelectedTile();

				game_data->world.GenChunk(game_data->prototype);
				game_data->input.mouse.DrawCursorOverlay(game_data->player, game_data->prototype);


				// Logistics logic
				{
					EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

					TransportLineLogicUpdate(game_data->world);
				}
				{
					EXECUTION_PROFILE_SCOPE(inserter_timer, "Inserter update");

					InserterLogicUpdate(game_data->world);
				}
			}

			// ======================================================================
			// Player logic
			{
				std::lock_guard<std::mutex> guard{game_data->player.mutex};

				game_data->player.RecipeCraftTick(game_data->prototype);
			}

			// Lock all mutexes for events
			std::lock_guard<std::mutex> world_guard{game_data->world.worldDataMutex};
			std::lock_guard<std::mutex> gui_guard{game_data->player.mutex};
			game_data->event.Raise<LogicTickEvent>(EventType::logic_tick, game_data->world.GameTick() % JC_GAME_HERTZ);
			game_data->input.key.Raise();
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
