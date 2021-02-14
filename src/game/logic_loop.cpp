// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic_loop.h"

#include <chrono>
#include <filesystem>
#include <thread>

#include "jactorio.h"

#include "core/execution_timer.h"
#include "core/loop_common.h"

using namespace jactorio;

void LogicLoop(ThreadedLoopCommon& common) {
    auto next_frame = std::chrono::steady_clock::now();
    while (common.gameState != ThreadedLoopCommon::GameState::quit) {
        EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");


        // Logic update loop
        if (common.gameState == ThreadedLoopCommon::GameState::in_world) {
            EXECUTION_PROFILE_SCOPE(logic_update_timer, "Logic update");

            std::lock_guard<std::mutex> guard{common.worldDataMutex};
            std::lock_guard<std::mutex> gui_guard{common.playerDataMutex};
            common.gameController.LogicUpdate();
        }


        auto time_end = std::chrono::steady_clock::now();
        while (time_end > next_frame) {
            next_frame += std::chrono::nanoseconds(16666666);
        }
        std::this_thread::sleep_until(next_frame);
    }
}


void game::InitLogicLoop(ThreadedLoopCommon& common) {
    ResourceGuard guard(+[]() { LOG_MESSAGE(info, "Logic thread exited"); });

    data::active_prototype_manager   = &common.gameController.proto;
    data::active_unique_data_manager = &common.gameController.unique;

    if (!common.gameController.Init())
        return;

    LOG_MESSAGE(info, "Prototype loading complete");
    common.prototypeLoadingComplete = true;


    LogicLoop(common);
}
