// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic_loop.h"

#include <chrono>
#include <filesystem>
#include <thread>

#include "jactorio.h"
#include "core/filesystem.h"

#include "data/prototype_manager.h"
#include "data/prototype/inserter.h"

#include "game/game_data.h"
#include "game/logic/transport_line_controller.h"

#include "renderer/render_loop.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/gui/imgui_manager.h"




#include <cereal/archives/portable_binary.hpp>
#include <fstream>

using namespace jactorio;

constexpr float kMoveSpeed = 0.8f;

void LogicLoop() {
	auto& game_data = *game::game_data;

	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	while (!game::logic_thread_should_exit) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		// ======================================================================
		// LOGIC LOOP ======================================================================
		{
			EXECUTION_PROFILE_SCOPE(logic_update_timer, "Logic update");

			// ======================================================================	
			// World chunks			
			{
				std::lock_guard<std::mutex> guard{game_data.world.worldDataMutex};

				game_data.logic.GameTickAdvance();
				game_data.logic.deferralTimer.DeferralUpdate(game_data.world, game_data.logic.GameTick());

				// Retrieved mvp matrix may be invalid on startup
				game_data.player.MouseCalculateSelectedTile(renderer::GetBaseRenderer()->GetMvpManager().GetMvpMatrix());

				game_data.world.GenChunk(game_data.prototype);
				game_data.input.mouse.DrawCursorOverlay(game_data.player, game_data.prototype);


				// Logistics logic
				{
					EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

					game::TransportLineLogicUpdate(game_data.world);
				}
				{
					EXECUTION_PROFILE_SCOPE(inserter_timer, "Inserter update");

					game::InserterLogicUpdate(game_data.world, game_data.logic);
				}
			}

			// ======================================================================
			// Player logic
			{
				std::lock_guard<std::mutex> guard{game_data.player.mutex};

				game_data.player.RecipeCraftTick(game_data.prototype);
			}

			// Lock all mutexes for events
			std::lock_guard<std::mutex> world_guard{game_data.world.worldDataMutex};
			std::lock_guard<std::mutex> gui_guard{game_data.player.mutex};
			game_data.event.Raise<game::LogicTickEvent>(game::EventType::logic_tick, game_data.logic.GameTick() % kGameHertz);
			game_data.input.key.Raise();
		}
		// ======================================================================
		// ======================================================================

		auto time_end = std::chrono::steady_clock::now();
		while (time_end > next_frame) {
			next_frame += std::chrono::nanoseconds(16666666);
		}
		std::this_thread::sleep_until(next_frame);
	}

}


void game::InitLogicLoop() {
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
		LOG_MESSAGE_F(error, "data/ folder not found at %s", core::ResolvePath("data").c_str());
		return;
	}

	LOG_MESSAGE(info, "Prototype loading complete");
	prototype_loading_complete = true;

	// ======================================================================
	// Temporary Startup settings
	game_data->player.SetPlayerWorldData(game_data->world);  // Main world is player's world
	game_data->player.SetPlayerLogicData(game_data->logic);  // Should be same for every player 


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
		if (renderer::input_mouse_captured || !game_data->player.MouseSelectedTileInRange())
			return;

		const auto tile_selected = game_data->player.GetMouseTileCoords();
		game_data->player.TryPlaceEntity(game_data->world, game_data->logic,
		                                 tile_selected.x, tile_selected.y);
	}, MouseInput::left, InputAction::key_held);

	game_data->input.key.Register([]() {
		if (renderer::input_mouse_captured || !game_data->player.MouseSelectedTileInRange())
			return;

		game_data->player.TryActivateLayer(game_data->world, game_data->player.GetMouseTileCoords());

	}, MouseInput::left, InputAction::key_down);

	// Remove entities or mine resource
	game_data->input.key.Register([]() {
		if (renderer::input_mouse_captured || !game_data->player.MouseSelectedTileInRange())
			return;

		const auto tile_selected = game_data->player.GetMouseTileCoords();
		game_data->player.TryPickup(game_data->world, game_data->logic,
		                            tile_selected.x, tile_selected.y);
	}, MouseInput::right, InputAction::key_held);


	game_data->input.key.Register([]() {
		std::ofstream out_cereal_stream("savegame.bin", std::ios_base::binary);
		cereal::PortableBinaryOutputArchive output_archive(out_cereal_stream);
		output_archive(game_data->world);
	}, SDLK_l, InputAction::key_up);


	LogicLoop();


	LOG_MESSAGE(info, "Logic thread exited");
}
