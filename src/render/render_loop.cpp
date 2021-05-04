// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/render_loop.h"

#include <chrono>
#include <thread>

#include <SDL.h>
#include <examples/imgui_impl_sdl.h>
#include <exception>

#include "core/execution_timer.h"
#include "core/loop_common.h"
#include "core/resource_guard.h"
#include "game/event/game_events.h"
#include "gui/imgui_manager.h"
#include "gui/main_menu.h"
#include "gui/menus.h"
#include "gui/menus_debug.h"
#include "proto/localization.h"
#include "proto/sprite.h"
#include "render/opengl/shader.h"
#include "render/renderer.h"
#include "render/spritemap_generator.h"

using namespace jactorio;

unsigned int window_x = 0;
unsigned int window_y = 0;

void render::ChangeWindowSize(Renderer& renderer,
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
void TimedDrawFrame(render::DisplayWindow& display_window, std::chrono::steady_clock::time_point& next_frame) {
    // Sleep until the next fixed update interval
    const auto time_end = std::chrono::steady_clock::now();
    while (time_end > next_frame) {
        next_frame += std::chrono::nanoseconds(16666666);
    }
    std::this_thread::sleep_until(next_frame);

    SDL_GL_SwapWindow(display_window.GetWindow());

    render::Renderer::GlClear();
}

/// Retrieves and handles sdl events
void PollEvents(ThreadedLoopCommon& common, render::DisplayWindow& display_window, SDL_Event& e) {
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        display_window.HandleSdlEvent(common, e);
    }
}


void RenderMainMenuLoop(ThreadedLoopCommon& common, render::DisplayWindow& display_window) {
    constexpr auto main_menu_game_state = ThreadedLoopCommon::GameState::main_menu;

    if (common.gameState != main_menu_game_state)
        return;


    LOG_MESSAGE(info, "2 - Main menu");

    auto next_frame = std::chrono::steady_clock::now();

    SDL_Event e;

    while (common.gameState == main_menu_game_state && common.gameState != ThreadedLoopCommon::GameState::quit) {
        common.gameController.event.Raise<game::RendererTickEvent>(
            game::EventType::renderer_tick, game::RendererTickEvent::DisplayWindowContainerT{std::ref(display_window)});

        gui::ImguiBeginFrame(display_window);

        gui::StartMenu(common);

        gui::ImguiRenderFrame();

        // ======================================================================
        // ======================================================================

        TimedDrawFrame(display_window, next_frame);
        PollEvents(common, display_window, e);
    }
}

void RenderWorldLoop(ThreadedLoopCommon& common, render::DisplayWindow& display_window) {
    constexpr auto world_render_game_state = ThreadedLoopCommon::GameState::in_world;

    if (common.gameState != world_render_game_state)
        return;


    LOG_MESSAGE(info, "3 - Runtime stage");

    auto next_frame = std::chrono::steady_clock::now(); // For zeroing the time

    SDL_Event e;

    while (common.gameState == world_render_game_state && common.gameState != ThreadedLoopCommon::GameState::quit) {
        EXECUTION_PROFILE_SCOPE(render_loop_timer, "Render loop");

        auto& player       = common.gameController.player;
        auto& player_world = common.gameController.worlds[player.world.GetId()];

        // ======================================================================
        // RENDER LOOP ======================================================================
        {
            EXECUTION_PROFILE_SCOPE(logic_update_timer, "Render update");

            common.gameController.event.Raise<game::RendererTickEvent>(
                game::EventType::renderer_tick,
                game::RendererTickEvent::DisplayWindowContainerT{std::ref(display_window)});

            std::lock_guard<std::mutex> guard{common.worldDataMutex};

            common.renderer->SetPlayerPosition(player.world.GetPosition());

            // MVP Matrices updated in here
            common.renderer->GlRenderPlayerPosition(common.gameController.logic.GameTick(), player_world);

            common.renderer->GlPrepareBegin();
            game::MouseSelection::DrawCursorOverlay(*common.renderer,
                                                    common.gameController.worlds,
                                                    common.gameController.player,
                                                    common.gameController.proto);

            common.gameController.player.world.SetMouseSelectedTile( //
                common.renderer->ScreenPosToWorldCoord(common.gameController.player.world.GetPosition(),
                                                       game::MouseSelection::GetCursor()));


            std::lock_guard<std::mutex> gui_guard{common.playerDataMutex};
            gui::ImguiBeginFrame(display_window);

            if (IsVisible(gui::Menu::MainMenu)) {
                gui::MainMenu(common);
            }

            gui::ImguiDraw(display_window,
                           common.gameController.worlds,
                           common.gameController.logic,
                           player,
                           common.gameController.proto,
                           common.gameController.event);

            gui::DebugMenuLogic(common.gameController.worlds,
                                common.gameController.logic,
                                player,
                                common.gameController.proto,
                                *common.renderer);

            //
            common.renderer->GlPrepareEnd();
            gui::ImguiRenderFrame();
        }
        // ======================================================================
        // ======================================================================

        TimedDrawFrame(display_window, next_frame);
        PollEvents(common, display_window, e);
    }
}

void RenderingLoop(ThreadedLoopCommon& common, render::DisplayWindow& display_window) {
    while (common.gameState != ThreadedLoopCommon::GameState::quit) {
        RenderMainMenuLoop(common, display_window);
        RenderWorldLoop(common, display_window);
    }
}

static void Init(ThreadedLoopCommon& common) {
    using namespace render;

    // Init window
    DisplayWindow display_window;
    if (display_window.Init(840, 490) != 0) {
        throw std::runtime_error("Failed to initialize display window");
    }


    ResourceGuard imgui_manager_guard(&gui::ImguiTerminate);
    gui::Setup(display_window);

    // From my testing, allocating it on the heap is faster than using the stack
    auto renderer   = std::make_unique<Renderer>();
    common.renderer = renderer.get();

    // Shader

    const Shader shader(std::vector<ShaderCreationInput>{{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
                                                         {"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER},
                                                         {"data/core/shaders/te.tese", GL_TESS_EVALUATION_SHADER}});
    shader.Bind();
    renderer->GetMvpManager().SetMvpUniformLocation(shader.GetUniformLocation("u_model_view_projection_matrix"));

    // Texture will be bound to slot 0 above, tell this to shader
    Shader::SetUniform1I(shader.GetUniformLocation("u_texture"), 0);


    // ======================================================================
    // Accessing game data

    // Since game data will be now accessed, wait until prototype loading is complete
    LOG_MESSAGE(debug, "Waiting for prototype loading to complete");
    while (!common.prototypeLoadingComplete) {
        if (common.gameState == ThreadedLoopCommon::GameState::quit) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOG_MESSAGE(debug, "Continuing render initialization");


    // Load gui font
    bool loaded_local  = false;
    auto localizations = common.gameController.proto.GetAll<proto::Localization>();
    for (const auto& local : localizations) {
        assert(local != nullptr);
        if (local->identifier == common.gameController.localIdentifier) {
            gui::LoadFont(*local);
            loaded_local = true;
            break;
        }
    }
    if (!loaded_local) {
        LOG_MESSAGE(warning, "No font was loaded, using default font");
    }

    // Loading textures
    auto renderer_sprites = RendererSprites();
    renderer_sprites.GInitializeSpritemap(common.gameController.proto, proto::Sprite::SpriteGroup::terrain, true);
    renderer_sprites.GInitializeSpritemap(common.gameController.proto, proto::Sprite::SpriteGroup::gui, false);


    Renderer::GlSetup();
    renderer->GlSetDrawThreads(8);

    // Terrain
    renderer->SetSpriteUvCoords(renderer_sprites.GetSpritemap(proto::Sprite::SpriteGroup::terrain).spritePositions);
    renderer_sprites.GetTexture(proto::Sprite::SpriteGroup::terrain)->Bind(0);

    // Gui
    gui::SetupCharacterData(renderer_sprites);


    // ======================================================================

    RenderingLoop(common, display_window);
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
