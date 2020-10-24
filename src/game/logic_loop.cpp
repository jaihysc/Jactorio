// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic_loop.h"

#include <chrono>
#include <filesystem>
#include <thread>

#include "jactorio.h"

#include "core/execution_timer.h"
#include "core/filesystem.h"
#include "core/loop_common.h"

#include "data/save_game_manager.h"
#include "proto/inserter.h"

#include "game/event/game_events.h"
#include "game/logic/conveyor_controller.h"

#include "gui/gui_menus.h"
#include "gui/imgui_manager.h"
#include "render/render_loop.h"
#include "render/renderer.h"

using namespace jactorio;

constexpr float kMoveSpeed = 0.8f;

void LogicLoop(ThreadedLoopCommon& common) {
    // Runtime

    auto& worlds = common.GetDataGlobal().worlds;
    auto& logic  = common.GetDataGlobal().logic;
    auto& player = common.GetDataGlobal().player;

    auto& event = common.gameDataLocal.event;
    auto& input = common.gameDataLocal.input;
    auto& proto = common.gameDataLocal.prototype;

    auto next_frame = std::chrono::steady_clock::now();
    while (common.gameState != ThreadedLoopCommon::GameState::quit) {
        EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

        // ======================================================================
        // LOGIC LOOP ======================================================================
        if (common.gameState == ThreadedLoopCommon::GameState::in_world) {
            EXECUTION_PROFILE_SCOPE(logic_update_timer, "Logic update");

            // ======================================================================
            // World chunks
            for (auto& world : worlds) {
                std::lock_guard<std::mutex> guard{common.worldDataMutex};

                logic.GameTickAdvance();
                logic.DeferralUpdate(world, logic.GameTick());


                world.GenChunk(proto);


                // Logistics logic
                {
                    EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

                    ConveyorLogicUpdate(world);
                }
                {
                    EXECUTION_PROFILE_SCOPE(inserter_timer, "Inserter update");

                    InserterLogicUpdate(world, logic);
                }
            }

            // ======================================================================
            // Player logic
            std::lock_guard<std::mutex> world_guard{common.worldDataMutex};

            // Retrieved mvp matrix may be invalid on startup
            player.world.CalculateMouseSelectedTile(render::GetBaseRenderer()->GetMvpManager().GetMvpMatrix());
            input.mouse.DrawCursorOverlay(worlds, player, proto);


            std::lock_guard<std::mutex> gui_guard{common.playerDataMutex};

            player.crafting.RecipeCraftTick(proto);


            event.Raise<game::LogicTickEvent>(game::EventType::logic_tick, logic.GameTick() % kGameHertz);
            input.key.Raise();
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


void game::InitLogicLoop(ThreadedLoopCommon& common) {
    // Initialize game data
    data::active_prototype_manager   = &common.gameDataLocal.prototype;
    data::active_unique_data_manager = &common.gameDataLocal.unique;

    try {
        common.gameDataLocal.prototype.LoadData(core::ResolvePath("data"));
    }
    catch (proto::ProtoError&) {
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

    // World controls
    common.gameDataLocal.input.key.Register(
        [&]() { common.GetDataGlobal().player.world.MovePlayerY(kMoveSpeed * -1); }, SDLK_w, InputAction::key_held);


    common.gameDataLocal.input.key.Register(
        [&]() { common.GetDataGlobal().player.world.MovePlayerY(kMoveSpeed); }, SDLK_s, InputAction::key_held);

    common.gameDataLocal.input.key.Register(
        [&]() { common.GetDataGlobal().player.world.MovePlayerX(kMoveSpeed * -1); }, SDLK_a, InputAction::key_held);

    common.gameDataLocal.input.key.Register(
        [&]() { common.GetDataGlobal().player.world.MovePlayerX(kMoveSpeed); }, SDLK_d, InputAction::key_held);


    // Menus
    common.gameDataLocal.input.key.Register(
        [&]() { SetVisible(gui::Menu::DebugMenu, !IsVisible(gui::Menu::DebugMenu)); },
        SDLK_BACKQUOTE,
        InputAction::key_up);

    common.gameDataLocal.input.key.Register(
        [&]() {
            // If a layer is already activated, deactivate it, otherwise open the gui menu
            if (common.GetDataGlobal().player.placement.GetActivatedLayer() != nullptr)
                common.GetDataGlobal().player.placement.SetActivatedLayer(nullptr);
            else
                SetVisible(gui::Menu::CharacterMenu, !IsVisible(gui::Menu::CharacterMenu));
        },
        SDLK_TAB,
        InputAction::key_up);


    // Rotating orientation
    common.gameDataLocal.input.key.Register(
        [&]() { common.GetDataGlobal().player.placement.RotateOrientation(); }, SDLK_r, InputAction::key_up);
    common.gameDataLocal.input.key.Register(
        [&]() { common.GetDataGlobal().player.placement.CounterRotateOrientation(); },
        SDLK_r,
        InputAction::key_up,
        KMOD_LSHIFT);


    common.gameDataLocal.input.key.Register(
        [&]() { common.GetDataGlobal().player.inventory.DeselectSelectedItem(); }, SDLK_q, InputAction::key_down);

    // Place entities
    common.gameDataLocal.input.key.Register(
        [&]() {
            if (gui::input_mouse_captured || !common.GetDataGlobal().player.world.MouseSelectedTileInRange())
                return;

            const auto tile_selected = common.GetDataGlobal().player.world.GetMouseTileCoords();

            auto& player = common.GetDataGlobal().player;
            auto& world  = common.GetDataGlobal().worlds[player.world.GetId()];

            player.placement.TryPlaceEntity(world, common.GetDataGlobal().logic, tile_selected.x, tile_selected.y);
        },
        MouseInput::left,
        InputAction::key_held);

    common.gameDataLocal.input.key.Register(
        [&]() {
            if (gui::input_mouse_captured || !common.GetDataGlobal().player.world.MouseSelectedTileInRange())
                return;

            auto& player = common.GetDataGlobal().player;
            auto& world  = common.GetDataGlobal().worlds[player.world.GetId()];

            player.placement.TryActivateLayer(world, player.world.GetMouseTileCoords());
        },
        MouseInput::left,
        InputAction::key_down);

    // Remove entities or mine resource
    common.gameDataLocal.input.key.Register(
        [&]() {
            if (gui::input_mouse_captured || !common.GetDataGlobal().player.world.MouseSelectedTileInRange())
                return;

            const auto tile_selected = common.GetDataGlobal().player.world.GetMouseTileCoords();

            auto& player = common.GetDataGlobal().player;
            auto& world  = common.GetDataGlobal().worlds[player.world.GetId()];

            player.placement.TryPickup(world, common.GetDataGlobal().logic, tile_selected.x, tile_selected.y);
        },
        MouseInput::right,
        InputAction::key_held);


    common.gameDataLocal.input.key.Register(
        [&]() { SetVisible(gui::Menu::MainMenu, !IsVisible(gui::Menu::MainMenu)); }, SDLK_ESCAPE, InputAction::key_up);

    LogicLoop(common);


    LOG_MESSAGE(info, "Logic thread exited");
}
