// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/main_menu.h"

#include <SDL.h>
#undef main // SDL redefines main to do initialization, this is not needed here

#include "jactorio.h"

#include "core/loop_common.h"
#include "core/resource_guard.h"
#include "data/save_game_manager.h"
#include "game/event/hardware_events.h"
#include "gui/components.h"
#include "gui/layout.h"
#include "gui/menus.h"
#include "proto/label.h"
#include "render/renderer.h"

using namespace jactorio;

///
/// \param fraction Fraction of the screen width to occupy
J_NODISCARD static float GetMainMenuWidth(const float fraction = 1.f / 3) {
    constexpr auto min_width = 150;

    const auto calculated_width = SafeCast<float>(render::Renderer::GetWindowWidth()) * fraction;
    if (calculated_width < min_width)
        return min_width;

    return calculated_width;
}

///
/// \param fraction Fraction of the screen height to occupy
J_NODISCARD static float GetMainMenuHeight(const float fraction = 1.f / 2) {
    constexpr auto min_height = 200;

    const auto calculated_height = SafeCast<float>(render::Renderer::GetWindowHeight()) * fraction;
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
/// Quarter of menu (with some separation between buttons)
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
/// Spans quarter of menu, ImGui default height
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
    ImGui::SetCursorPosX(new_cursor_x + GetButtonMiniSpan() * SafeCast<float>(button_gap));
}

///
/// \param button_gap Additional gap of that would have fit provided button count
static void SameLineMenuButtonMini(const unsigned button_gap = 0) {
    ImGui::SameLine();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + GetButtonMiniSpan() * SafeCast<float>(button_gap));
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
/// Changes player_action's keybind to next key up
static void ChangeKeyNextKeyUp(ThreadedLoopCommon& common, game::PlayerAction::Type player_action) {
    common.gameDataLocal.event.SubscribeOnce(game::EventType::input_activity, [&common, player_action](const auto& e) {
        const auto& input_variant = static_cast<const game::InputActivityEvent&>(e).input;

        if (std::holds_alternative<game::KeyboardActivityEvent>(input_variant)) {
            const auto& kb_event = std::get<game::KeyboardActivityEvent>(input_variant);

            if (kb_event.keyAction != game::InputAction::key_up) {
                ChangeKeyNextKeyUp(common, player_action);
                return;
            }

            common.keybindManager.ChangeActionKey(player_action, kb_event.key);
            common.keybindManager.ChangeActionMods(player_action, kb_event.mods);
        }
        else {
            const auto& ms_event = std::get<game::MouseActivityEvent>(input_variant);

            if (ms_event.keyAction != game::InputAction::key_up) {
                ChangeKeyNextKeyUp(common, player_action);
                return;
            }

            common.keybindManager.ChangeActionKey(player_action, ms_event.key);
            common.keybindManager.ChangeActionMods(player_action, ms_event.mods);
        }
    });
}


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


    ///
    /// Button which when clicked will set the next key for the player action
    auto keybind_button = [](const game::InputManager::IntKeyMouseCodePair int_code, const SDL_Keymod mods) {
        std::string keybind_name;

        // Mod names

        ///
        /// Tests if the provided key mod(s) is set
        auto keymod_set = [&mods](const SDL_Keymod target_mods) {
            return (mods & (static_cast<SDL_Keymod>(0) | target_mods)) == target_mods;
        };

        if (keymod_set(KMOD_LCTRL)) {
            keybind_name += "L CTRL + ";
        }
        if (keymod_set(KMOD_RCTRL)) {
            keybind_name += "R CTRL + ";
        }

        if (keymod_set(KMOD_LSHIFT)) {
            keybind_name += "L SHIFT + ";
        }
        if (keymod_set(KMOD_RSHIFT)) {
            keybind_name += "R SHIFT + ";
        }

        if (keymod_set(KMOD_LALT)) {
            keybind_name += "L ALT + ";
        }
        if (keymod_set(KMOD_RALT)) {
            keybind_name += "R ALT + ";
        }


        // Key name
        if (int_code >= 0) {
            keybind_name += SDL_GetKeyName(int_code);
        }
        else {
            switch (static_cast<game::MouseInput>(int_code * -1)) {
            case game::MouseInput::left:
                keybind_name += "Mouse left";
                break;
            case game::MouseInput::middle:
                keybind_name += "Mouse middle";
                break;
            case game::MouseInput::right:
                keybind_name += "Mouse right";
                break;
            case game::MouseInput::x1:
                keybind_name += "Mouse x1";
                break;
            case game::MouseInput::x2:
                keybind_name += "Mouse x2";
                break;

            default:
                assert(false); // Unknown mouse key
                break;
            }
        }


        if (keybind_name.empty()) {
            keybind_name = "???";
        }

        return MenuButtonMini(keybind_name.c_str());
    };

    ///
    /// Dropdown which displays current key action, and can be opened to select a new key action
    /// \return {dropdown clicked, clicked InputAction}
    auto key_action_dropdown = [&common](const char* dropdown_name,
                                         game::InputAction current_key_action) -> std::pair<bool, game::InputAction> {
        // MSVC fails CTAD with when this array is captured in a lambda
        constexpr std::array<const char*, 6> key_action_labels{
            proto::LabelNames::kKeyActionNone,
            proto::LabelNames::kKeyActionKDown,
            proto::LabelNames::kKeyActionKPressed,
            proto::LabelNames::kKeyActionKRepeat,
            proto::LabelNames::kKeyActionKHeld,
            proto::LabelNames::kKeyActionKUp,
        };

        auto localize_key_action = [&common, &key_action_labels](const std::size_t key_action_index) {
            return common.gameDataLocal.proto.Get<proto::Label>(std::string(key_action_labels[key_action_index]))
                ->GetLocalizedName()
                .c_str();
        };

        const auto current_key_action_index  = static_cast<int>(current_key_action);
        const auto* current_key_action_c_str = localize_key_action(current_key_action_index);

        if (ImGui::BeginCombo(dropdown_name, current_key_action_c_str)) {
            ResourceGuard guard(+[]() { ImGui::EndCombo(); });

            for (std::size_t i = 0; i < key_action_labels.size(); ++i) {
                const bool is_selected = i == current_key_action_index;

                if (ImGui::Selectable(localize_key_action(i), is_selected)) {
                    return {true, static_cast<game::InputAction>(i)};
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }

        return {false, static_cast<game::InputAction>(0)};
    };


    // Key action which was selected
    const auto& info = common.keybindManager.GetKeybindInfo();

    // Omit displaying the test player action, which should be the last player action
    static_assert(game::PlayerAction::Type::test ==
                  static_cast<game::PlayerAction::Type>(game::PlayerAction::kActionCount_ - 1));

    for (std::size_t i = 0; i < info.size() - 1; ++i) {
        const auto action_label_name = std::string(proto::LabelNames::kPlayerActionPrefix) + std::to_string(i);
        const auto* label            = common.gameDataLocal.proto.Get<proto::Label>(action_label_name);
        assert(label != nullptr);

        ImGui::TextUnformatted(label->GetLocalizedName().c_str());


        const auto& [key, key_action, key_mod] = info[i];
        const auto player_action               = static_cast<game::PlayerAction::Type>(i);

        // Key bind
        {
            ToNextMenuButtonMiniBegin(1);

            ImGuard guard;

            std::string key_button_id;
            key_button_id.push_back(i);

            guard.PushID(key_button_id.c_str());

            if (keybind_button(key, key_mod)) { // Clicked
                ChangeKeyNextKeyUp(common, player_action);
            }
        }


        // Key action

        ImGuard guard;
        SameLineMenuButtonMini();

        guard.PushItemWidth(GetButtonMiniWidth());

        std::string combo_id = "##_option_change_keybind_menu-combo";
        combo_id.push_back(i); // Uniquely identifies each dropdown

        auto [dropdown_clicked, clicked_player_action] = key_action_dropdown(combo_id.c_str(), key_action);
        if (dropdown_clicked) {
            common.keybindManager.ChangeActionKeyAction(player_action, clicked_player_action);
        }
    }

    if (MenuBackButton(common.mainMenuData, MainMenuData::Window::options)) {
        data::SerializeKeybinds(common.keybindManager);
    }

    SameLineMenuButtonMini(2);

    if (MenuButtonMini("Reset")) {
        common.keybindManager.LoadDefaultKeybinds();
    }
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
