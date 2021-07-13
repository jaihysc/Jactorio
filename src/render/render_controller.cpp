// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/render_controller.h"

#include "core/loop_common.h"
#include "gui/imgui_manager.h"
#include "gui/main_menu.h"
#include "gui/menus.h"
#include "gui/menus_debug.h"
#include "proto/localization.h"
#include "render/imgui_renderer.h"
#include "render/opengl/error.h"
#include "render/renderer.h"

using namespace jactorio;

render::RenderController::~RenderController() {
    if (setupGui_) {
        gui::ImguiTerminate();
    }
}

void render::RenderController::Init() {
    if (displayWindow.Init(840, 490) != 0) {
        throw std::runtime_error("Failed to initialize display window");
    }

    InitGui();
    renderer.Init();
    renderer.GlSetDrawThreads(8);
}

void render::RenderController::LoadedInit(ThreadedLoopCommon& common) {
    InitGuiFont(common);
    InitTextures(common);
    InitShader(renderer);
}

void render::RenderController::RenderMainMenu(ThreadedLoopCommon& common) const {
    gui::ImguiBeginFrame(displayWindow);
    gui::StartMenu(common);
    gui::ImguiRenderFrame();
}

void render::RenderController::RenderWorld(ThreadedLoopCommon& common) {
    auto& player       = common.gameController.player;
    auto& player_world = common.gameController.worlds[player.world.GetId()];

    UpdateAnimationTexCoords();

    std::lock_guard guard{common.worldDataMutex};

    renderer.SetPlayerPosition(player.world.GetPosition());

    // MVP Matrices updated in here
    renderer.GlRenderPlayerPosition(common.gameController.logic.GameTick(), player_world);


    renderer.GlPrepareBegin();

    common.gameController.player.world.SetMouseSelectedTile( //
        renderer.ScreenPosToWorldCoord(common.gameController.player.world.GetPosition(),
                                       game::MouseSelection::GetCursor()));

    game::MouseSelection::DrawCursorOverlay(
        renderer, common.gameController.worlds, common.gameController.player, common.gameController.proto);


    std::lock_guard gui_guard{common.playerDataMutex};
    gui::ImguiBeginFrame(displayWindow);

    if (IsVisible(gui::Menu::MainMenu)) {
        gui::MainMenu(common);
    }

    gui::ImguiDraw(displayWindow,
                   common.gameController.worlds,
                   common.gameController.logic,
                   player,
                   common.gameController.proto,
                   common.gameController.event);

    gui::DebugMenuLogic(
        common.gameController.worlds, common.gameController.logic, player, common.gameController.proto, renderer);

    //
    renderer.GlPrepareEnd();
    gui::ImguiRenderFrame();
}

void render::RenderController::InitGui() {
    gui::Setup(displayWindow);
    setupGui_ = true;
}

void render::RenderController::InitGuiFont(ThreadedLoopCommon& common) const {
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

    ImGui_ImplOpenGL3_CreateFontsTexture(); // Must be called after loading gui font
}

void render::RenderController::InitTextures(ThreadedLoopCommon& common) {
    rendererSprites.GlInitializeSpritemap(common.gameController.proto, proto::Sprite::SpriteGroup::terrain, false);
    rendererSprites.GlInitializeSpritemap(common.gameController.proto, proto::Sprite::SpriteGroup::gui, false);

    // Terrain
    rendererSprites.GetTexture(proto::Sprite::SpriteGroup::terrain)->Bind(0);

    // Gui
    gui::SetupCharacterData(rendererSprites);
}

void render::RenderController::InitShader(Renderer& renderer) {
    auto [terrain_tex_coords, terrain_tex_coord_size] =
        rendererSprites.GetSpritemap(proto::Sprite::SpriteGroup::terrain).GenCurrentFrame();
    LOG_MESSAGE_F(info, "%d tex coords for tesselation renderer", terrain_tex_coord_size);

    GLint max_uniform_component;
    DEBUG_OPENGL_CALL(glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, &max_uniform_component));
    if (terrain_tex_coord_size > max_uniform_component / 4) {
        throw std::runtime_error(std::string("Max tex coords exceeded: ") + std::to_string(max_uniform_component / 4));
    }

    shader.Init({{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
                 {"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER},
                 {"data/core/shaders/te.tese", GL_TESS_EVALUATION_SHADER}},
                {{"__terrain_tex_coords_size", std::to_string(terrain_tex_coord_size)}});
    shader.Bind();
    renderer.GetMvpManager().SetMvpUniformLocation(shader.GetUniformLocation("u_model_view_projection_matrix"));

    // Texture will be bound to slot 0 above, tell this to shader
    DEBUG_OPENGL_CALL(glUniform1i(shader.GetUniformLocation("u_texture"), 0));
}

void render::RenderController::UpdateAnimationTexCoords() const noexcept {
    auto [tex_coords, size] = rendererSprites.GetSpritemap(proto::Sprite::SpriteGroup::terrain).GenNextFrame();

    static_assert(std::is_same_v<GLfloat, TexCoord::PositionT::ValueT>);
    DEBUG_OPENGL_CALL(glUniform4fv(shader.GetUniformLocation("u_tex_coords"), //
                                   size,
                                   reinterpret_cast<const GLfloat*>(tex_coords)));
}
