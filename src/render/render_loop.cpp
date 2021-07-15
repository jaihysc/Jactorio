// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/render_loop.h"

#include <chrono>
#include <thread>

#include <SDL.h>
#include <examples/imgui_impl_sdl.h>
#include <exception>

#include "core/execution_timer.h"
#include "core/loop_common.h"
#include "game/event/game_events.h"

using namespace jactorio;

unsigned int window_x = 0;
unsigned int window_y = 0;

void render::ChangeWindowSize(TileRenderer& renderer,
                              game::EventData& event,
                              const unsigned int window_size_x,
                              const unsigned int window_size_y) {
    // Ignore minimize
    if (window_size_x == 0 && window_size_y == 0)
        return;

    // Same size
    if (window_x == window_size_x && window_y == window_size_y)
        return;

    window_x = window_size_x;
    window_y = window_size_y;

    event.SubscribeOnce(game::EventType::renderer_tick,
                        [&renderer](auto& /*e*/) { renderer.GlResizeWindow(window_x, window_y); });

    LOG_MESSAGE_F(debug, "Resolution changed to %dx%d", window_size_x, window_size_y);
}

/// Waits until next frame time, draws frame
static void TimedDrawFrame(render::DisplayWindow& display_window, std::chrono::steady_clock::time_point& next_frame) {
    // Sleep until the next fixed update interval
    const auto time_end = std::chrono::steady_clock::now();
    while (time_end > next_frame) {
        next_frame += std::chrono::nanoseconds(16666666);
    }
    std::this_thread::sleep_until(next_frame);

    SDL_GL_SwapWindow(display_window.GetWindow());

    render::TileRenderer::GlClear();
}

/// Retrieves and handles sdl events
static void PollEvents(ThreadedLoopCommon& common, render::DisplayWindow& display_window, SDL_Event& e) {
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        display_window.HandleSdlEvent(common, e);
    }
}


static void RenderMainMenuLoop(ThreadedLoopCommon& common) {
    constexpr auto main_menu_game_state = ThreadedLoopCommon::GameState::main_menu;

    if (common.gameState != main_menu_game_state)
        return;


    LOG_MESSAGE(info, "2 - Main menu");

    auto next_frame = std::chrono::steady_clock::now();
    SDL_Event e;
    while (common.gameState == main_menu_game_state && common.gameState != ThreadedLoopCommon::GameState::quit) {
        common.gameController.event.Raise<game::RendererTickEvent>(
            game::EventType::renderer_tick,
            game::RendererTickEvent::DisplayWindowContainerT{std::ref(common.renderController->displayWindow)});

        common.renderController->RenderMainMenu(common);
        TimedDrawFrame(common.renderController->displayWindow, next_frame);
        PollEvents(common, common.renderController->displayWindow, e);
    }
}

static void RenderWorldLoop(ThreadedLoopCommon& common) {
    constexpr auto world_render_game_state = ThreadedLoopCommon::GameState::in_world;

    if (common.gameState != world_render_game_state)
        return;


    LOG_MESSAGE(info, "3 - Runtime stage");

    auto next_frame = std::chrono::steady_clock::now(); // For zeroing the time
    SDL_Event e;
    while (common.gameState == world_render_game_state && common.gameState != ThreadedLoopCommon::GameState::quit) {
        EXECUTION_PROFILE_SCOPE(render_loop_timer, "Render loop");

        {
            EXECUTION_PROFILE_SCOPE(logic_update_timer, "Render update");

            common.gameController.event.Raise<game::RendererTickEvent>(
                game::EventType::renderer_tick,
                game::RendererTickEvent::DisplayWindowContainerT{std::ref(common.renderController->displayWindow)});

            common.renderController->RenderWorld(common);
        }

        TimedDrawFrame(common.renderController->displayWindow, next_frame);
        PollEvents(common, common.renderController->displayWindow, e);
    }
}

static void Init(ThreadedLoopCommon& common) {
    const auto render_controller = std::make_unique<render::RenderController>();
    common.renderController      = render_controller.get();

    common.renderController->Init();


    // Since game data will be now accessed, wait until prototype loading is complete
    LOG_MESSAGE(debug, "Waiting for prototype loading to complete");
    while (!common.prototypeLoadingComplete) {
        if (common.gameState == ThreadedLoopCommon::GameState::quit) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOG_MESSAGE(debug, "Continuing render initialization");


    common.renderController->LoadedInit(common);


    // Runtime
    while (common.gameState != ThreadedLoopCommon::GameState::quit) {
        RenderMainMenuLoop(common);
        RenderWorldLoop(common);
    }
}

void render::RenderInit(ThreadedLoopCommon& common) {
    try {
        Init(common);
    }
    catch (std::exception& e) {
        LOG_MESSAGE_F(error, "Render thread exception '%s'", e.what());
    }

    common.gameState = ThreadedLoopCommon::GameState::quit;
    LOG_MESSAGE(info, "Renderer thread exited");
}
