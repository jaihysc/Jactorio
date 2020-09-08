// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <thread>

#include "jactorio.h"

#include "core/crash_handler.h"
#include "core/filesystem.h"
#include "game/logic_loop.h"
#include "render//render_loop.h"

void InitializeGame() {
    using namespace jactorio;

    // Rendering + logic initialization
    LOG_MESSAGE(info, "1 - Data stage");

    LogicRenderLoopCommon common_data;

    std::thread logic_thread    = std::thread(game::InitLogicLoop, std::ref(common_data));
    std::thread renderer_thread = std::thread(render::RenderInit, std::ref(common_data));

    logic_thread.join();
    renderer_thread.join();
}

///
/// ENTRY POINT
///
int main(int ac, char* av[]) {
    using namespace jactorio;

    core::SetExecutingDirectory(av[0]);

    // Log file
    core::ResourceGuard log_guard(&core::CloseLogFile);
    core::OpenLogFile();

    core::RegisterCrashHandler();

    // Initial startup message
    LOG_MESSAGE_F(info, "%s | %s build, version: %s\n\n", JACTORIO_BUILD_TARGET_PLATFORM, BUILD_TYPE, JACTORIO_VERSION);

    InitializeGame();

    LOG_MESSAGE(info, "goodbye!");
    return 0;
}
