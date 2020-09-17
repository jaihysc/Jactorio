// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/gui/main_menu.h"

#include "jactorio.h"

#include "core/loop_common.h"
#include "render/gui/components.h"
#include "render/gui/gui_layout.h"
#include "render/rendering/renderer.h"

using namespace jactorio;

static float GetMainMenuWidth() {
    constexpr auto min_width = 150;

    const auto calculated_width = core::SafeCast<float>(render::Renderer::GetWindowWidth()) / 4.f;
    if (calculated_width < min_width)
        return min_width;

    return calculated_width;
}

static float GetMainMenuHeight() {
    constexpr auto min_height = 200;

    const auto calculated_height = core::SafeCast<float>(render::Renderer::GetWindowHeight()) / 2.f;
    if (calculated_height < min_height)
        return min_height;

    return calculated_height;
}


J_NODISCARD static bool MenuButton(const char* label) {
    constexpr auto button_height = 50;
    return ImGui::Button(label, {GetMainMenuWidth() - render::GetTotalWindowPaddingX(), button_height});
}

void render::MainMenu(ThreadedLoopCommon& common) {
    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_main_menu");

    const GuiTitle title;
    title.Begin("Jactorio | " JACTORIO_VERSION);

    if constexpr (JACTORIO_DEBUG_BUILD) {
        ImGui::Text("NOTE: Debug build");
    }

    // ======================================================================

    if (MenuButton("New game")) {
        common.gameState = ThreadedLoopCommon::GameState::in_world;
    }

    if (MenuButton("Load game")) {
        // TODO
    }

    if (MenuButton("Quit")) {
        common.gameState = ThreadedLoopCommon::GameState::quit;
    }
}
