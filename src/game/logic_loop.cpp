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


#include "data/cereal/register_type.h"

#include <fstream>
#include <cereal/archives/portable_binary.hpp>

using namespace jactorio;

constexpr float kMoveSpeed = 0.8f;

void LogicLoop(LogicRenderLoopCommon& common) {
	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	while (!common.logicThreadShouldExit) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		// ======================================================================
		// LOGIC LOOP ======================================================================
		{
			EXECUTION_PROFILE_SCOPE(logic_update_timer, "Logic update");

			// ======================================================================	
			// World chunks			
			{
				std::lock_guard<std::mutex> guard{common.worldDataMutex};

				common.gameDataGlobal.logic.GameTickAdvance();
				common.gameDataGlobal.logic.DeferralUpdate(common.gameDataGlobal.world,
														   common.gameDataGlobal.logic.GameTick());

				// Retrieved mvp matrix may be invalid on startup
				common.gameDataGlobal.player.MouseCalculateSelectedTile(renderer::GetBaseRenderer()->GetMvpManager().GetMvpMatrix());

				common.gameDataGlobal.world.GenChunk(common.gameDataLocal.prototype);
				common.gameDataLocal.input.mouse.DrawCursorOverlay(common.gameDataGlobal.player,
				                                                   common.gameDataLocal.prototype);


				// Logistics logic
				{
					EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

					game::TransportLineLogicUpdate(common.gameDataGlobal.world);
				}
				{
					EXECUTION_PROFILE_SCOPE(inserter_timer, "Inserter update");

					game::InserterLogicUpdate(common.gameDataGlobal.world, common.gameDataGlobal.logic);
				}
			}

			// ======================================================================
			// Player logic
			{
				std::lock_guard<std::mutex> guard{common.playerDataMutex};

				common.gameDataGlobal.player.RecipeCraftTick(common.gameDataLocal.prototype);
			}

			// Lock all mutexes for events
			std::lock_guard<std::mutex> world_guard{common.worldDataMutex};
			std::lock_guard<std::mutex> gui_guard{common.playerDataMutex};
			common.gameDataLocal.event.Raise<game::LogicTickEvent>(
				game::EventType::logic_tick, common.gameDataGlobal.logic.GameTick() % kGameHertz);
			common.gameDataLocal.input.key.Raise();
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


void game::InitLogicLoop(LogicRenderLoopCommon& common) {
	core::CapturingGuard<void()> loop_termination_guard([&]() {
		common.renderThreadShouldExit = true;
		common.logicThreadShouldExit  = true;
	});

	// Initialize game data
	data::active_data_manager = &common.gameDataLocal.prototype;

	try {
		common.gameDataLocal.prototype.LoadData(core::ResolvePath("data"));
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
	common.prototypeLoadingComplete = true;

	// ======================================================================
	// Temporary Startup settings
	common.gameDataGlobal.player.SetPlayerWorldData(common.gameDataGlobal.world);  // Main world is player's world
	common.gameDataGlobal.player.SetPlayerLogicData(common.gameDataGlobal.logic);  // Should be same for every player 


	// ======================================================================

	// Movement controls
	common.gameDataLocal.input.key.Register([&]() {
		common.gameDataGlobal.player.MovePlayerY(kMoveSpeed * -1);
	}, SDLK_w, InputAction::key_held);


	common.gameDataLocal.input.key.Register([&]() {
		common.gameDataGlobal.player.MovePlayerY(kMoveSpeed);
	}, SDLK_s, InputAction::key_held);

	common.gameDataLocal.input.key.Register([&]() {
		common.gameDataGlobal.player.MovePlayerX(kMoveSpeed * -1);
	}, SDLK_a, InputAction::key_held);

	common.gameDataLocal.input.key.Register([&]() {
		common.gameDataGlobal.player.MovePlayerX(kMoveSpeed);
	}, SDLK_d, InputAction::key_held);


	// Menus
	common.gameDataLocal.input.key.Register([&]() {
		SetVisible(renderer::Menu::DebugMenu,
		           !IsVisible(renderer::Menu::DebugMenu));
	}, SDLK_BACKQUOTE, InputAction::key_up);

	common.gameDataLocal.input.key.Register([&]() {
		// If a layer is already activated, deactivate it, otherwise open the gui menu
		if (common.gameDataGlobal.player.GetActivatedLayer() != nullptr)
			common.gameDataGlobal.player.SetActivatedLayer(nullptr);
		else
			SetVisible(renderer::Menu::CharacterMenu,
			           !IsVisible(renderer::Menu::CharacterMenu));

	}, SDLK_TAB, InputAction::key_up);


	// Rotating orientation	
	common.gameDataLocal.input.key.Register([&]() {
		common.gameDataGlobal.player.RotatePlacementOrientation();
	}, SDLK_r, InputAction::key_up);
	common.gameDataLocal.input.key.Register([&]() {
		common.gameDataGlobal.player.CounterRotatePlacementOrientation();
	}, SDLK_r, InputAction::key_up, KMOD_LSHIFT);


	common.gameDataLocal.input.key.Register([&]() {
		common.gameDataGlobal.player.DeselectSelectedItem();
	}, SDLK_q, InputAction::key_down);

	// Place entities
	common.gameDataLocal.input.key.Register([&]() {
		if (renderer::input_mouse_captured || !common.gameDataGlobal.player.MouseSelectedTileInRange())
			return;

		const auto tile_selected = common.gameDataGlobal.player.GetMouseTileCoords();
		common.gameDataGlobal.player.TryPlaceEntity(common.gameDataGlobal.world, common.gameDataGlobal.logic,
		                                            tile_selected.x, tile_selected.y);
	}, MouseInput::left, InputAction::key_held);

	common.gameDataLocal.input.key.Register([&]() {
		if (renderer::input_mouse_captured || !common.gameDataGlobal.player.MouseSelectedTileInRange())
			return;

		common.gameDataGlobal.player.TryActivateLayer(common.gameDataGlobal.world, common.gameDataGlobal.player.GetMouseTileCoords());

	}, MouseInput::left, InputAction::key_down);

	// Remove entities or mine resource
	common.gameDataLocal.input.key.Register([&]() {
		if (renderer::input_mouse_captured || !common.gameDataGlobal.player.MouseSelectedTileInRange())
			return;

		const auto tile_selected = common.gameDataGlobal.player.GetMouseTileCoords();
		common.gameDataGlobal.player.TryPickup(common.gameDataGlobal.world, common.gameDataGlobal.logic,
		                                       tile_selected.x, tile_selected.y);
	}, MouseInput::right, InputAction::key_held);


	common.gameDataLocal.input.key.Register([&]() {
		data::SerializeGameData(common.gameDataGlobal);
	}, SDLK_l, InputAction::key_up);

	common.gameDataLocal.input.key.Register([&]() {
		data::DeserializeGameData(common.gameDataLocal, common.gameDataGlobal);

		printf("Break");
	}, SDLK_k, InputAction::key_up);


	LogicLoop(common);


	LOG_MESSAGE(info, "Logic thread exited");
}
