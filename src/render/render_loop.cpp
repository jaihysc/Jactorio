// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/render_loop.h"

#include <chrono>
#include <thread>

#include <SDL.h>
#include <examples/imgui_impl_sdl.h>

#include "jactorio.h"

#include "render/gui/imgui_manager.h"
#include "render/opengl/shader.h"

using namespace jactorio;

unsigned int window_x = 0;
unsigned int window_y = 0;

render::Renderer* main_renderer = nullptr;

void render::ChangeWindowSize(game::EventData& event,
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

    event.SubscribeOnce(game::EventType::renderer_tick, []() { main_renderer->GlResizeWindow(window_x, window_y); });

    LOG_MESSAGE_F(debug, "Resolution changed to %dx%d", window_size_x, window_size_y);
}

render::Renderer* render::GetBaseRenderer() {
    return main_renderer;
}


void RenderingLoop(LogicRenderLoopCommon& common, render::DisplayWindow& display_window) {
    LOG_MESSAGE(info, "2 - Runtime stage");

    auto next_frame = std::chrono::steady_clock::now(); // For zeroing the time

    SDL_Event e;

    while (!common.renderThreadShouldExit) {
        EXECUTION_PROFILE_SCOPE(render_loop_timer, "Render loop");

        auto& player_data  = common.gameDataGlobal.player;
        auto& player_world = common.gameDataGlobal.worlds[player_data.world.GetId()];

        // ======================================================================
        // RENDER LOOP ======================================================================
        {
            EXECUTION_PROFILE_SCOPE(logic_update_timer, "Render update");

            common.gameDataLocal.event.Raise<game::RendererTickEvent>(
                game::EventType::renderer_tick,
                game::RendererTickEvent::DisplayWindowContainerT{std::ref(display_window)});

            render::Renderer::GlClear();
            std::lock_guard<std::mutex> guard{common.worldDataMutex};

            // MVP Matrices updated in here
            main_renderer->GlRenderPlayerPosition(common.gameDataGlobal.logic.GameTick(),
                                                  player_world,
                                                  player_data.world.GetPositionX(),
                                                  player_data.world.GetPositionY());


            std::lock_guard<std::mutex> gui_guard{common.playerDataMutex};

            ImguiDraw(display_window,
                      common.gameDataGlobal.worlds,
                      common.gameDataGlobal.logic,
                      player_data,
                      common.gameDataLocal.prototype,
                      common.gameDataLocal.event);
        }
        // ======================================================================
        // ======================================================================

        // Sleep until the next fixed update interval
        auto time_end = std::chrono::steady_clock::now();
        while (time_end > next_frame) {
            next_frame += std::chrono::nanoseconds(16666666);
        }
        std::this_thread::sleep_until(next_frame);

        SDL_GL_SwapWindow(display_window.GetWindow());

        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            display_window.HandleSdlEvent(common, e);
        }
    }
}

void render::RenderInit(LogicRenderLoopCommon& common) {
    core::CapturingGuard<void()> loop_termination_guard([&]() {
        common.renderThreadShouldExit = true;
        common.logicThreadShouldExit  = true;
    });

    // Init window
    DisplayWindow display_window{};
    try {
        if (display_window.Init(840, 490) != 0)
            return;
    }
    catch (data::DataException&) {
        return;
    }


    core::ResourceGuard imgui_manager_guard(&ImguiTerminate);
    Setup(display_window);

    // Shader
    // From my testing, allocating it on the heap is faster than using the stack
    core::ResourceGuard<void> renderer_guard([]() { delete main_renderer; });
    main_renderer = new Renderer();

    const Shader shader(std::vector<ShaderCreationInput>{{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
                                                         {"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER}});
    shader.Bind();
    main_renderer->GetMvpManager().SetMvpUniformLocation(shader.GetUniformLocation("u_model_view_projection_matrix"));

    // Texture will be bound to slot 0 above, tell this to shader
    Shader::SetUniform1I(shader.GetUniformLocation("u_texture"), 0);


    // ======================================================================
    // Accessing game data

    // Since game data will be now accessed, wait until prototype loading is complete
    LOG_MESSAGE(debug, "Waiting for prototype loading to complete");
    while (!common.prototypeLoadingComplete)
        ;
    LOG_MESSAGE(debug, "Continuing render initialization");


    // Loading textures
    auto renderer_sprites = RendererSprites();
    renderer_sprites.GInitializeSpritemap(common.gameDataLocal.prototype, data::Sprite::SpriteGroup::terrain, true);
    renderer_sprites.GInitializeSpritemap(common.gameDataLocal.prototype, data::Sprite::SpriteGroup::gui, false);


    Renderer::GlSetup();
    main_renderer->GlSetDrawThreads(8);

    // Terrain
    main_renderer->SetSpriteUvCoords(renderer_sprites.GetSpritemap(data::Sprite::SpriteGroup::terrain).spritePositions);
    renderer_sprites.GetTexture(data::Sprite::SpriteGroup::terrain)->Bind(0);

    // Gui
    SetupCharacterData(renderer_sprites);


    // ======================================================================

    common.gameDataLocal.input.key.Register(
        [&]() {
            common.gameDataLocal.event.SubscribeOnce(game::EventType::renderer_tick, [](game::EventBase& e) {
                auto& render_e = static_cast<game::RendererTickEvent&>(e);
                auto& window   = render_e.windows[0].get();

                window.SetFullscreen(!window.IsFullscreen());
                main_renderer->GlResizeWindow(window_x, window_y);
            });
        },
        SDLK_SPACE,
        game::InputAction::key_down);

    RenderingLoop(common, display_window);

    LOG_MESSAGE(info, "Renderer thread exited");
}