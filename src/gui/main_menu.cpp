// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/main_menu.h"

#include "jactorio.h"

#include "core/loop_common.h"
#include "data/save_game_manager.h"
#include "game/event/hardware_events.h"
#include "gui/components.h"
#include "gui/layout.h"
#include "gui/menus.h"
#include "render/renderer.h"

using namespace jactorio;

///
/// \param fraction Fraction of the screen width to occupy
J_NODISCARD static float GetMainMenuWidth(const float fraction = 1.f / 3) {
    constexpr auto min_width = 150;

    const auto calculated_width = core::SafeCast<float>(render::Renderer::GetWindowWidth()) * fraction;
    if (calculated_width < min_width)
        return min_width;

    return calculated_width;
}

///
/// \param fraction Fraction of the screen height to occupy
J_NODISCARD static float GetMainMenuHeight(const float fraction = 1.f / 2) {
    constexpr auto min_height = 200;

    const auto calculated_height = core::SafeCast<float>(render::Renderer::GetWindowHeight()) * fraction;
    if (calculated_height < min_height)
        return min_height;

    return calculated_height;
}

///
/// Spans entire menu
J_NODISCARD static float GetButtonWidth() {
    return ImGui::GetWindowSize().x - gui::GetTotalWindowPaddingX();
}

J_NODISCARD static float GetButtonHeight() {
    return 50;
}

///
/// Quarter of menu (width some separation between buttons)
J_NODISCARD static float GetButtonMiniWidth() {
    return GetButtonWidth() / 4 - gui::GetTotalWindowItemSpacingX(1);
}

///
/// Total space by mini button taken including padding and spacing
J_NODISCARD static auto GetButtonMiniSpan() {
    return GetButtonMiniWidth() + gui::kGuiStyleItemSpacingX;
}


///
/// Button which spans entire menu
/// \param width If -1: jactorio default, 0: Imgui default
/// \param height If -1: jactorio default 0: Imgui default
J_NODISCARD static bool MenuButton(const char* label, float width = -1, float height = -1) {
    if (width == -1) {
        width = GetButtonWidth();
    }

    if (height == -1) {
        height = GetButtonHeight();
    }

    return ImGui::Button(label, {width, height});
}

///
/// Spans quarter of menu, half height
J_NODISCARD static bool MenuButtonMini(const char* label) {
    return MenuButton(label, GetButtonMiniWidth(), 0);
}

///
/// Menu button for heading back to previous menu when clicked
/// \return true if clicked
static bool MenuBackButton(gui::MainMenuData& menu_data, const gui::MainMenuData::Window new_menu) {
    if (MenuButtonMini("Back")) {
        menu_data.currentMenu = new_menu;
        return true;
    }

    return false;
}


///
/// Sets draw cursor to where next menu button should begin (similar to a tab stop)
/// \remark Do NOT call SameLineMenuButtonMini, it is already on the same line
/// \param button_gap Additional gap of that would have fit provided button count to skip
static void ToNextMenuButtonMiniBegin(const unsigned button_gap = 0) {
    ImGui::SameLine();

    const auto but_width    = GetButtonMiniWidth();
    const auto old_cursor_x = ImGui::GetCursorPosX();

    float new_cursor_x = ImGui::GetStyle().WindowPadding.x;

    while (new_cursor_x <= old_cursor_x) {
        new_cursor_x += but_width;
        new_cursor_x += gui::kGuiStyleItemSpacingX;
    }
    ImGui::SetCursorPosX(new_cursor_x + GetButtonMiniSpan() * core::SafeCast<float>(button_gap));
}

///
/// \param button_gap Additional gap of that would have fit provided button count
static void SameLineMenuButtonMini(const unsigned button_gap = 0) {
    ImGui::SameLine();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + GetButtonMiniSpan() * core::SafeCast<float>(button_gap));
}


void ErrorText(const char* error_msg) {
    gui::ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Text, gui::kGuiColTextError);
    ImGui::TextUnformatted(error_msg);
}

///
/// \param error_str Cleared when user dismisses it
void ErrorTextDismissible(std::string& error_str) {
    if (error_str.empty())
        return;

    ErrorText(error_str.c_str());

    ImGui::SameLine();

    if (ImGui::Button("x", {10, 10})) {
        error_str.clear();
    }
}

// ======================================================================


static void ChangeGameState(ThreadedLoopCommon& common, const ThreadedLoopCommon::GameState new_state) {
    SetVisible(gui::Menu::MainMenu, false);

    common.mainMenuData.currentMenu = gui::MainMenuData::Window::main;
    common.gameState                = new_state;
}

static void NewGameMenu(ThreadedLoopCommon& common) {
    const gui::GuiMenu menu;
    gui::SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_new_game_menu");

    const gui::GuiTitle title;
    title.Begin("New game");


    auto seed = common.GetDataGlobal().worlds[0].GetWorldGeneratorSeed(); // Should be same for all worlds
    ImGui::InputInt("Seed", &seed);


    MenuBackButton(common.mainMenuData, gui::MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (MenuButtonMini("Start")) {
        common.ResetGlobalData();

        ChangeGameState(common, ThreadedLoopCommon::GameState::in_world);
    }


    // Sets seed for newly reset global data as well
    for (auto& world : common.GetDataGlobal().worlds) {
        world.SetWorldGeneratorSeed(seed);
    }
}

///
/// Lists all save games, loads save game user clicks on
void LoadSaveGameMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_save_game_browser_menu");

    const GuiTitle title;
    title.Begin("Load game");

    auto& last_load_error = common.mainMenuData.lastLoadError;
    ErrorTextDismissible(last_load_error);

    for (const auto& save_game : data::GetSaveDirIt()) {
        const auto filename = save_game.path().stem().string();

        if (MenuButton(filename.c_str())) {
            try {
                data::DeserializeGameData(common.gameDataLocal, common.GetDataGlobal(), filename);
            }
            catch (cereal::Exception& e) {
                last_load_error = e.what();
                LOG_MESSAGE_F(error, "Failed to load game %s : %s", filename.c_str(), e.what());
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
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_save_game_menu");

    const GuiTitle title;
    title.Begin("Save game");


    auto* save_name       = common.mainMenuData.saveName;
    auto& last_save_error = common.mainMenuData.lastSaveError;

    ErrorTextDismissible(last_save_error);

    if (!data::IsValidSaveName(save_name)) {
        ErrorText("Invalid save name");
    }

    ImGui::InputText("Save name", save_name, MainMenuData::kMaxSaveNameLength);


    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (data::IsValidSaveName(save_name)) {
        if (MenuButtonMini("Save")) {
            try {
                data::SerializeGameData(common.GetDataGlobal(), save_name);
                common.mainMenuData.currentMenu = MainMenuData::Window::main;
            }
            catch (cereal::Exception& e) {
                last_save_error = e.what();
                LOG_MESSAGE_F(error, "Failed to save game as %s : %s", save_name, e.what());
            }
        }
    }
}

// Options menu

///
/// Allows the user to change keybinds
void OptionKeybindMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(3.f / 4), GetMainMenuHeight(2.f / 3)});
    menu.Begin("_option_change_keybind_menu");

    const GuiTitle title;
    title.Begin("Keybinds");


    ImGui::TextUnformatted("Name");

    ToNextMenuButtonMiniBegin(1);
    ImGui::TextUnformatted("Key");

    ToNextMenuButtonMiniBegin();
    ImGui::TextUnformatted("Action");


    // Set Key action, applies to all keybinds which are newly set
    const char* items[] = {
        "None",
        "Key down",
        "Key pressed (down + before repeat)",
        "Key repeat",
        "Key held (pressed + repeat)",
        "Key up",
    };

    static const char* current_item = items[0];


    // Key action which was selected
    const auto& info = common.keybindManager.GetKeybindInfo();
    for (std::size_t i = 0; i < info.size(); ++i) {
        const auto& keybind = info[i];

        ImGui::Text("%llu", keybind);

        ToNextMenuButtonMiniBegin(1);

        // Key bind
        {
            ImGuard guard;

            std::string id;
            id.push_back(i);

            guard.PushID(id.c_str());
            if (MenuButtonMini("Key bind")) {
                int key_action_index = 0;
                for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                    const bool is_selected = current_item == items[i];

                    if (is_selected) {
                        key_action_index = i;
                    }
                }

                common.gameDataLocal.event.SubscribeOnce(
                    game::EventType::keyboard_activity, [&common, i, key_action_index](auto& e) {
                        auto& key_event = static_cast<game::KeyboardActivityEvent&>(e);

                        LOG_MESSAGE(debug, "Change keybind");

                        common.keybindManager.ChangeActionInput(static_cast<game::PlayerAction::Type>(i),
                                                                key_event.key,
                                                                static_cast<game::InputAction>(key_action_index),
                                                                key_event.mods);
                    });
            }
        }

        SameLineMenuButtonMini();

        // Key action
        // Dropdown menu to select key action
        ImGuard guard;
        guard.PushItemWidth(GetButtonMiniWidth());

        std::string combo_id = "##_option_change_keybind_menu-combo";
        combo_id.push_back(i); // Uniquely identifies each combo

        if (ImGui::BeginCombo(combo_id.c_str(), current_item)) {
            for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                const bool is_selected = current_item == items[i];

                if (ImGui::Selectable(items[i], is_selected))
                    current_item = items[i];

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    MenuBackButton(common.mainMenuData, MainMenuData::Window::options);
}

///
/// Presents Various submenus for options of the game
void OptionsMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_options_menu");

    const GuiTitle title;
    title.Begin("Options");

    if (MenuButton("Keybinds")) {
        common.mainMenuData.currentMenu = MainMenuData::Window::option_change_keybind;
    }

    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
}

///
/// \return true if a submenu was drawn
bool DrawSubmenu(ThreadedLoopCommon& common) {
    switch (common.mainMenuData.currentMenu) {
    case gui::MainMenuData::Window::main:
        break;
    case gui::MainMenuData::Window::new_game:
        NewGameMenu(common);
        return true;
    case gui::MainMenuData::Window::load_game:
        LoadSaveGameMenu(common);
        return true;
    case gui::MainMenuData::Window::save_game:
        SaveGameMenu(common);
        return true;

    case gui::MainMenuData::Window::options:
        OptionsMenu(common);
        return true;
    case gui::MainMenuData::Window::option_change_keybind:
        OptionKeybindMenu(common);
        return true;


    default:
        assert(false);
        break;
    }
    return false;
}

void gui::StartMenu(ThreadedLoopCommon& common) {
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

    if (MenuButton("Options")) {
        common.mainMenuData.currentMenu = MainMenuData::Window::options;
    }

    if (MenuButton("Quit")) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::quit);
    }
}

void gui::MainMenu(ThreadedLoopCommon& common) {
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

    if (MenuButton("Options")) {
        common.mainMenuData.currentMenu = MainMenuData::Window::options;
    }


    if (MenuButton("Quit")) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::main_menu);
    }
}
