// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/render_controller.h"

#include <GL/glew.h>

#include "core/execution_timer.h"
#include "core/loop_common.h"
#include "gui/imgui_manager.h"
#include "gui/main_menu.h"
#include "gui/menus.h"
#include "gui/menus_debug.h"
#include "proto/localization.h"
#include "render/imgui_renderer.h"
#include "render/opengl/error.h"

using namespace jactorio;

void render::RenderController::Init() {
    if (displayWindow.Init(840, 490) != 0) {
        throw std::runtime_error("Failed to initialize display window");
    }

    InitRendererCommon();
    imManager.Init(displayWindow);
    renderer.Init();
    renderer.GlSetDrawThreads(8);
}

void render::RenderController::LoadedInit(ThreadedLoopCommon& common) {
    InitGuiFont(common);
    InitTextures(common);
    renderer.InitShader();
}

void render::RenderController::RenderMainMenu(ThreadedLoopCommon& common) const {
    imManager.imRenderer.Bind();
    imManager.BeginFrame(displayWindow);
    gui::StartMenu(common);
    imManager.RenderFrame();
}

void render::RenderController::RenderWorld(ThreadedLoopCommon& common) {
    auto& player       = common.gameController.player;
    auto& player_world = common.gameController.worlds[player.world.GetId()];

    std::lock_guard guard{common.worldDataMutex};

    renderer.SetPlayerPosition(player.world.GetPosition());
    renderer.GlBind();
    renderer.GlRender(player_world); // Updates MVP Matrices


    common.gameController.player.world.SetMouseSelectedTile( //
        renderer.ScreenPosToWorldCoord(common.gameController.player.world.GetPosition(),
                                       game::MouseSelection::GetCursor()));

    renderer.GlPrepareBegin();
    game::MouseSelection::DrawCursorOverlay(
        renderer, common.gameController.worlds, common.gameController.player, common.gameController.proto);


    {
        std::lock_guard gui_guard{common.playerDataMutex};
        EXECUTION_PROFILE_SCOPE(imgui_draw_timer, "Imgui draw");

        imManager.imRenderer.Bind();
        imManager.BeginFrame(displayWindow);

        if (IsVisible(gui::Menu::MainMenu)) {
            gui::MainMenu(common);
        }

        imManager.PrepareWorld(player_world, renderer);
        imManager.PrepareGui(common.gameController.worlds,
                             common.gameController.logic,
                             player,
                             common.gameController.proto,
                             common.gameController.event);

        gui::DebugMenuLogic(
            common.gameController.worlds, common.gameController.logic, player, common.gameController.proto, renderer);
    }

    renderer.GlBind();
    renderer.GlPrepareEnd();

    imManager.imRenderer.Bind(); // Gui must be drawn last to be above all else
    imManager.RenderFrame();
}

void render::RenderController::InitRendererCommon() const noexcept {
    // Enables transparency in textures
    DEBUG_OPENGL_CALL(glEnable(GL_BLEND));
    DEBUG_OPENGL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Depth buffer
    // ImGui does not use depth buffer
    /*
    DEBUG_OPENGL_CALL(glEnable(GL_DEPTH_TEST));
    DEBUG_OPENGL_CALL(glDepthFunc(GL_LEQUAL));
    */
}

void render::RenderController::InitGuiFont(ThreadedLoopCommon& common) {
    bool loaded_local  = false;
    auto localizations = common.gameController.proto.GetAll<proto::Localization>();
    for (const auto& local : localizations) {
        assert(local != nullptr);
        if (local->identifier == common.gameController.localIdentifier) {
            if (!imManager.LoadFont(*local)) {
                throw std::runtime_error("Failed to load font");
            }
            loaded_local = true;
            break;
        }
    }
    if (!loaded_local) {
        LOG_MESSAGE(warning, "No font was loaded, using default font");
    }

    imManager.imRenderer.InitFontsTexture(); // Must be called after loading gui font
}

void render::RenderController::InitTextures(ThreadedLoopCommon& common) {
    rendererSprites.GlInitializeSpritemap(common.gameController.proto, proto::Sprite::SpriteGroup::terrain, false);
    rendererSprites.GlInitializeSpritemap(common.gameController.proto, proto::Sprite::SpriteGroup::gui, false);

    renderer.InitTexture(rendererSprites.GetSpritemap(proto::Sprite::SpriteGroup::terrain),
                         rendererSprites.GetTexture(proto::Sprite::SpriteGroup::terrain));
    imManager.InitData(rendererSprites.GetSpritemap(proto::Sprite::SpriteGroup::gui),
                       rendererSprites.GetTexture(proto::Sprite::SpriteGroup::gui));
}
