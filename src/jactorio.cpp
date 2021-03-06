﻿// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <filesystem>
#include <thread>

#include "jactorio.h"

#include "config.h"
#include "core/crash_handler.h"
#include "core/loop_common.h"
#include "core/resource_guard.h"
#include "game/logic_loop.h"
#include "render/render_loop.h"

void InitializeGame() {
    using namespace jactorio;

    // Rendering + logic initialization
    LOG_MESSAGE(info, "1 - Data stage");

    ThreadedLoopCommon common_data;

    std::thread logic_thread    = std::thread(game::InitLogicLoop, std::ref(common_data));
    std::thread renderer_thread = std::thread(render::RenderInit, std::ref(common_data));

    logic_thread.join();
    renderer_thread.join();
}

/// ENTRY POINT
int main(int ac, char* av[]) {
    using namespace jactorio;

    current_path(std::filesystem::path(av[0]).parent_path());

    // Log file
    ResourceGuard log_guard(&CloseLogFile);
    OpenLogFile();

    RegisterCrashHandler();

    // Initial startup message
    LOG_MESSAGE_F(info,
                  "%s | %s build %d, version: %s\n\n",
                  CConfig::kBuildTargetPlatform,
                  CConfig::kBuildType,
                  CConfig::kBuildNumber,
                  CConfig::kVersion);

    InitializeGame();

    LOG_MESSAGE(info, "goodbye!");
    return 0;
}
