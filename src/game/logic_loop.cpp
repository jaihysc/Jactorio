// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic_loop.h"

#include <chrono>
#include <filesystem>
#include <thread>

#include "jactorio.h"

#include "core/execution_timer.h"
#include "core/loop_common.h"

#include "data/prototype_manager.h"

#include "proto/inserter.h"

#include "game/event/game_events.h"
#include "game/logic/conveyor_controller.h"
#include "game/player/keybind_manager.h"

#include "gui/imgui_manager.h"
#include "render/render_loop.h"
#include "render/renderer.h"

using namespace jactorio;


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
        common.gameDataLocal.prototype.LoadData(data::PrototypeManager::kDataFolder);
    }
    catch (proto::ProtoError&) {
        // Prototype loading error
        return;
    }
    catch (std::filesystem::filesystem_error&) {
        // Data folder not found error
        LOG_MESSAGE_F(error, "data folder not found at %s", data::PrototypeManager::kDataFolder);
        return;
    }

    LOG_MESSAGE(info, "Prototype loading complete");
    common.prototypeLoadingComplete = true;


    common.keybindManager.LoadDefaultKeybinds();

    LogicLoop(common);


    LOG_MESSAGE(info, "Logic thread exited");
}
