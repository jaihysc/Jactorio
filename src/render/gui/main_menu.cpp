// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/gui/main_menu.h"

#include "jactorio.h"

#include "core/loop_common.h"
#include "data/save_game_manager.h"
#include "render/gui/components.h"
#include "render/gui/gui_layout.h"
#include "render/gui/gui_menus.h"
#include "render/rendering/renderer.h"

using namespace jactorio;

J_NODISCARD static float GetMainMenuWidth() {
    constexpr auto min_width = 150;

    const auto calculated_width = core::SafeCast<float>(render::Renderer::GetWindowWidth()) / 3.f;
    if (calculated_width < min_width)
        return min_width;

    return calculated_width;
}

J_NODISCARD static float GetMainMenuHeight() {
    constexpr auto min_height = 200;

    const auto calculated_height = core::SafeCast<float>(render::Renderer::GetWindowHeight()) / 2.f;
    if (calculated_height < min_height)
        return min_height;

    return calculated_height;
}

J_NODISCARD static float GetButtonWidth() {
    return GetMainMenuWidth() - render::GetTotalWindowPaddingX();
}

J_NODISCARD static float GetButtonHeight() {
    return 50;
}

J_NODISCARD static float GetButtonMiniWidth() {
    return GetButtonWidth() / 4;
}

J_NODISCARD static float GetButtonMiniHeight() {
    return GetButtonHeight() / 2;
}


///
/// \param width If 0, uses default
/// \param height If 0, uses default
J_NODISCARD static bool MenuButton(const char* label, float width = 0, float height = 0) {
    if (width == 0) {
        width = GetButtonWidth();
    }

    if (height == 0) {
        height = GetButtonHeight();
    }

    return ImGui::Button(label, {width, height});
}

J_NODISCARD static bool MenuButtonMini(const char* label) {
    return MenuButton(label, GetButtonMiniWidth(), GetButtonMiniHeight());
}

///
/// Menu button for heading back to previous menu when clicked
/// \return true if clicked
static bool MenuBackButton(MainMenuData& menu_data, const MainMenuData::Window new_menu) {
    if (MenuButtonMini("Back")) {
        menu_data.currentMenu = new_menu;
        return true;
    }

    return false;
}


///
/// \param button_gap Additional gap of that would have fit provided button count
static void SameLineMenuButtonMini(const unsigned button_gap = 0) {
    ImGui::SameLine();

    const auto previous_button_end_x = ImGui::GetCursorPosX() - render::GetTotalWindowItemSpacingX(1);
    ImGui::SetCursorPosX(previous_button_end_x + GetButtonMiniWidth() * core::SafeCast<float>(button_gap));
}


void ErrorText(const char* error_msg) {
    render::ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Text, render::kGuiColTextError);
    ImGui::TextUnformatted(error_msg);
}

// ======================================================================


static void ChangeGameState(ThreadedLoopCommon& common, const ThreadedLoopCommon::GameState new_state) {
    SetVisible(render::Menu::MainMenu, false);

    common.mainMenuData.currentMenu = MainMenuData::Window::main;
    common.gameState                = new_state;
}

static void NewGameMenu(ThreadedLoopCommon& common) {
    const render::GuiMenu menu;
    render::SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_new_game_menu");

    const render::GuiTitle title;
    title.Begin("New game");

    // Get set seed
    auto seed = common.gameDataGlobal.worlds[0].GetWorldGeneratorSeed(); // Should be same for all worlds
    ImGui::InputInt("Seed", &seed);

    for (auto& world : common.gameDataGlobal.worlds) {
        world.SetWorldGeneratorSeed(seed);
    }

    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (MenuButtonMini("Start")) {
        data::PrepareWorldDataClear(common.gameDataLocal, common.gameDataGlobal);

        for (auto& world : common.gameDataGlobal.worlds) {
            world.Clear();
        }

        ChangeGameState(common, ThreadedLoopCommon::GameState::in_world);
    }
}

///
/// Lists all save games, loads save game user clicks on
void LoadSaveGameMenu(ThreadedLoopCommon& common) {
    using namespace render;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_save_game_browser_menu");

    const GuiTitle title;
    title.Begin("Load game");

    auto& last_load_error = common.mainMenuData.lastLoadError;

    for (const auto& save_game : data::GetSaveDirIt()) {
        const auto filename = save_game.path().stem().string();

        if (MenuButton(filename.c_str())) {
            try {
                data::DeserializeGameData(common.gameDataLocal, common.gameDataGlobal, filename);
            }
            catch (cereal::Exception& e) {
                last_load_error = e.what();
                LOG_MESSAGE_F(error, "Failed to load game %s : %s", filename, e.what());
            }
        }
    }

    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (MenuButtonMini("Play")) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::in_world);
    }
}

///
/// Asks for save name and saves current world
void SaveGameMenu(ThreadedLoopCommon& common) {
    using namespace render;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_save_game_menu");

    const GuiTitle title;
    title.Begin("Save game");


    auto* save_name       = common.mainMenuData.saveName;
    auto& last_save_error = common.mainMenuData.lastSaveError;

    if (!data::IsValidSaveName(save_name)) {
        ErrorText("Invalid save name");
    }

    ImGui::InputText("Save name", save_name, MainMenuData::kMaxSaveNameLength);


    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (data::IsValidSaveName(save_name)) {
        if (MenuButtonMini("Save")) {
            try {
                data::SerializeGameData(common.gameDataGlobal, save_name);
                common.mainMenuData.currentMenu = MainMenuData::Window::main;
            }
            catch (cereal::Exception& e) {
                last_save_error = e.what();
                LOG_MESSAGE_F(error, "Failed to save game as %s : %s", save_name, e.what());
            }
        }
    }
}

///
/// \return true if a submenu was drawn
bool DrawSubmenu(ThreadedLoopCommon& common) {
    switch (common.mainMenuData.currentMenu) {
    case MainMenuData::Window::main:
        break;
    case MainMenuData::Window::new_game:
        NewGameMenu(common);
        return true;
    case MainMenuData::Window::load_game:
        LoadSaveGameMenu(common);
        return true;
    case MainMenuData::Window::save_game:
        SaveGameMenu(common);
        return true;


    default:
        assert(false);
        break;
    }
    return false;
}

void render::StartMenu(ThreadedLoopCommon& common) {
    if (DrawSubmenu(common))
        return;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_start_menu");

    const GuiTitle title;
    title.Begin("Jactorio | " JACTORIO_VERSION);

#ifdef JACTORIO_DEBUG_BUILD
    ImGui::TextUnformatted("NOTE: Debug build");

    if (MenuButton("Debug start game")) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::in_world);
    }
#endif

    // ======================================================================

    if (MenuButton("New game")) {
        common.mainMenuData.currentMenu = MainMenuData::Window::new_game;
    }

    if (MenuButton("Load game")) {
        common.mainMenuData.currentMenu = MainMenuData::Window::load_game;
    }

    if (MenuButton("Quit")) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::quit);
    }
}

void render::MainMenu(ThreadedLoopCommon& common) {
    if (DrawSubmenu(common))
        return;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_main_menu");

    const GuiTitle title;
    title.Begin("Main menu");


    if (MenuButton("Load game")) {
        common.mainMenuData.currentMenu = MainMenuData::Window::load_game;
    }

    if (MenuButton("Save game")) {
        common.mainMenuData.currentMenu = MainMenuData::Window::save_game;
    }

    if (MenuButton("Quit")) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::main_menu);
    }
}