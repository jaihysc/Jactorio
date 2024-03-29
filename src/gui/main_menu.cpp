// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/main_menu.h"

#include <SDL.h>
#undef main // SDL redefines main to do initialization, this is not needed here

#include "jactorio.h"

#include "config.h"
#include "core/convert.h"
#include "core/loop_common.h"
#include "core/resource_guard.h"
#include "data/save_game_manager.h"
#include "game/event/game_events.h"
#include "game/event/hardware_events.h"
#include "gui/components.h"
#include "gui/layout.h"
#include "gui/menus.h"
#include "proto/label.h"
#include "proto/localization.h"
#include "render/tile_renderer.h"

using namespace jactorio;

/// \param fraction Fraction of the screen width to occupy
J_NODISCARD static float GetMainMenuWidth(const float fraction = 1.f / 3) {
    constexpr auto min_width = 150;

    const auto calculated_width = SafeCast<float>(render::TileRenderer::GetWindowWidth()) * fraction;
    if (calculated_width < min_width)
        return min_width;

    return calculated_width;
}

/// \param fraction Fraction of the screen height to occupy
J_NODISCARD static float GetMainMenuHeight(const float fraction = 1.f / 2) {
    constexpr auto min_height = 200;

    const auto calculated_height = SafeCast<float>(render::TileRenderer::GetWindowHeight()) * fraction;
    if (calculated_height < min_height)
        return min_height;

    return calculated_height;
}

/// Spans entire menu
J_NODISCARD static float GetButtonWidth() {
    return ImGui::GetWindowSize().x - gui::GetTotalWindowPaddingX();
}

J_NODISCARD static float GetButtonHeight() {
    return 50;
}

/// Quarter of menu (with some separation between buttons)
J_NODISCARD static float GetButtonMiniWidth() {
    return GetButtonWidth() / 4 - gui::GetTotalWindowItemSpacingX(1);
}

/// Total space by mini button taken including padding and spacing
J_NODISCARD static auto GetButtonMiniSpan() {
    return GetButtonMiniWidth() + gui::kGuiStyleItemSpacingX;
}


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

/// Spans quarter of menu, ImGui default height
J_NODISCARD static bool MenuButtonMini(const char* label) {
    return MenuButton(label, GetButtonMiniWidth(), 0);
}

/// Menu button for heading back to previous menu when clicked
/// Clears any outstanding error messages on backwards click
/// \return true if clicked
static bool MenuBackButton(gui::MainMenuData& menu_data, const gui::MainMenuData::Window new_menu) {
    if (MenuButtonMini("<")) {
        menu_data.currentMenu = new_menu;
        menu_data.lastError.clear();
        return true;
    }

    return false;
}


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

/// \param error_str Cleared when user dismisses it
static void ErrorTextDismissible(std::string& error_str) {
    if (error_str.empty())
        return;

    ErrorText(error_str.c_str());

    ImGui::SameLine();

    if (ImGui::Button("x", {10, 10})) {
        error_str.clear();
    }
}

static const char* GetLocalText(const ThreadedLoopCommon& common, const std::string& label_name) {
    return common.gameController.proto.GetLocalText(label_name).c_str();
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
    title.Begin(GetLocalText(common, proto::LabelNames::kMenuNewGame));


    auto seed = common.gameController.worlds[0].GetWorldGeneratorSeed(); // Should be same for all worlds
    ImGui::InputInt(GetLocalText(common, proto::LabelNames::kMenuNewGameSeed), &seed);


    MenuBackButton(common.mainMenuData, gui::MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (MenuButtonMini(GetLocalText(common, proto::LabelNames::kMenuNewGamePlay))) {
        common.gameController.ResetGame();

        ChangeGameState(common, ThreadedLoopCommon::GameState::in_world);
    }


    // Sets seed for newly reset global data as well
    for (auto& world : common.gameController.worlds) {
        world.SetWorldGeneratorSeed(seed);
    }
}

/// Lists all save games, loads save game user clicks on
static void LoadSaveGameMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_save_game_browser_menu");

    const GuiTitle title;
    title.Begin(GetLocalText(common, proto::LabelNames::kMenuLoadGame));

    ErrorTextDismissible(common.mainMenuData.lastError);

    for (const auto& save_game : data::GetSaveDirIt()) {
        const auto filename = save_game.path().stem().string();

        if (MenuButton(filename.c_str())) {
            try {
                common.gameController.LoadGame(filename.c_str());
            }
            catch (cereal::Exception& e) {
                common.mainMenuData.lastError = e.what();
                LOG_MESSAGE_F(error, "Failed to load game %s : %s", filename.c_str(), e.what());
            }
        }
    }

    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (MenuButtonMini(GetLocalText(common, proto::LabelNames::kMenuLoadGamePlay))) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::in_world);
    }
}

/// Asks for save name and saves current world
static void SaveGameMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_save_game_menu");

    const GuiTitle title;
    title.Begin(GetLocalText(common, proto::LabelNames::kMenuSaveGame));


    auto* save_name = common.mainMenuData.saveName;

    ErrorTextDismissible(common.mainMenuData.lastError);

    if (!data::IsValidSaveName(save_name)) {
        ErrorText(GetLocalText(common, proto::LabelNames::kMenuSaveGameInvalidName));
    }

    ImGui::InputText(
        GetLocalText(common, proto::LabelNames::kMenuSaveGameSaveName), save_name, MainMenuData::kMaxSaveNameLength);


    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
    SameLineMenuButtonMini(2);

    if (data::IsValidSaveName(save_name)) {
        if (MenuButtonMini(GetLocalText(common, proto::LabelNames::kMenuSaveGameSave))) {
            try {
                common.gameController.SaveGame(save_name);
                common.mainMenuData.currentMenu = MainMenuData::Window::main;
            }
            catch (cereal::Exception& e) {
                common.mainMenuData.lastError = e.what();
                LOG_MESSAGE_F(error, "Failed to save game as %s : %s", save_name, e.what());
            }
        }
    }
}

// Options menu

/// Changes player_action's keybind to next key up
static void ChangeKeyNextKeyUp(ThreadedLoopCommon& common, game::PlayerAction::Type player_action) {
    common.gameController.event.SubscribeOnce(game::EventType::input_activity, [&common, player_action](const auto& e) {
        const auto& input_variant = SafeCast<const game::InputActivityEvent&>(e).input;

        if (std::holds_alternative<game::KeyboardActivityEvent>(input_variant)) {
            const auto& kb_event = std::get<game::KeyboardActivityEvent>(input_variant);

            if (kb_event.keyAction != game::InputAction::key_up) {
                ChangeKeyNextKeyUp(common, player_action);
                return;
            }

            common.gameController.keybindManager.ChangeActionKey(player_action, kb_event.key);
            common.gameController.keybindManager.ChangeActionMods(player_action, kb_event.mods);
        }
        else {
            const auto& ms_event = std::get<game::MouseActivityEvent>(input_variant);

            if (ms_event.keyAction != game::InputAction::key_up) {
                ChangeKeyNextKeyUp(common, player_action);
                return;
            }

            common.gameController.keybindManager.ChangeActionKey(player_action, ms_event.key);
            common.gameController.keybindManager.ChangeActionMods(player_action, ms_event.mods);
        }
    });
}


/// Allows the user to change keybinds
static void OptionKeybindMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(3.f / 4), GetMainMenuHeight(2.f / 3)});
    menu.Begin("_option_change_keybind_menu");

    const GuiTitle title;
    title.Begin(GetLocalText(common, proto::LabelNames::kMenuOptionChangeKeybind));


    ImGui::TextUnformatted(GetLocalText(common, proto::LabelNames::kMenuOptionChangeKeybindName));

    ToNextMenuButtonMiniBegin(1);
    ImGui::TextUnformatted(GetLocalText(common, proto::LabelNames::kMenuOptionChangeKeybindKey));

    ToNextMenuButtonMiniBegin();
    ImGui::TextUnformatted(GetLocalText(common, proto::LabelNames::kMenuOptionChangeKeybindAction));


    /// Button which when clicked will set the next key for the player action
    auto keybind_button = [&common](const game::InputManager::IntKeyMouseCodePair int_code, const SDL_Keymod mods) {
        std::string keybind_name;

        // Mod names

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
                keybind_name += GetLocalText(common, proto::LabelNames::kKeyMouseLeft);
                break;
            case game::MouseInput::middle:
                keybind_name += GetLocalText(common, proto::LabelNames::kKeyMouseMiddle);
                break;
            case game::MouseInput::right:
                keybind_name += GetLocalText(common, proto::LabelNames::kKeyMouseRight);
                break;
            case game::MouseInput::x1:
                keybind_name += GetLocalText(common, proto::LabelNames::kKeyMouseX1);
                break;
            case game::MouseInput::x2:
                keybind_name += GetLocalText(common, proto::LabelNames::kKeyMouseX2);
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
            return GetLocalText(common, key_action_labels[key_action_index]);
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
    const auto& info = common.gameController.keybindManager.GetKeybindInfo();

    // Omit displaying the test player action, which should be the last player action
    static_assert(game::PlayerAction::Type::test ==
                  static_cast<game::PlayerAction::Type>(game::PlayerAction::kActionCount_ - 1));

    for (std::size_t i = 0; i < info.size() - 1; ++i) {
        const auto action_label_name = std::string(proto::LabelNames::kPlayerActionPrefix) + std::to_string(i);
        ImGui::TextUnformatted(GetLocalText(common, action_label_name));


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
            common.gameController.keybindManager.ChangeActionKeyAction(player_action, clicked_player_action);
        }
    }

    if (MenuBackButton(common.mainMenuData, MainMenuData::Window::options)) {
        common.gameController.SaveSetting();
    }

    SameLineMenuButtonMini(2);

    if (MenuButtonMini(GetLocalText(common, proto::LabelNames::kMenuOptionChangeKeybindReset))) {
        common.gameController.keybindManager.LoadDefaultKeybinds();
    }
}

/// Allows user to change languages
static void OptionLanguageMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_language_menu");

    const GuiTitle title;
    title.Begin(GetLocalText(common, proto::LabelNames::kMenuOptionChangeLanguage));


    ErrorTextDismissible(common.mainMenuData.lastError);
    ImGui::TextUnformatted(GetLocalText(common, proto::LabelNames::kMenuOptionChangeLanguageRestartNotice));

    for (auto* local : common.gameController.proto.GetAll<proto::Localization>()) {
        assert(local != nullptr);

        const auto button_text = local->identifier + " | " + local->GetLocalizedName();
        ImGuard guard;
        guard.PushID(button_text.c_str());

        if (MenuButton(button_text.c_str())) {
            common.gameController.localIdentifier = local->identifier;

            try {
                common.gameController.SaveSetting();
            }
            catch (std::runtime_error& e) {
                common.mainMenuData.lastError = e.what();
                LOG_MESSAGE_F(error, "Failed to save settings: %s", e.what());
            }
        }
    }

    MenuBackButton(common.mainMenuData, MainMenuData::Window::options);
}

/// Presents Various submenus for options of the game
static void OptionsMenu(ThreadedLoopCommon& common) {
    using namespace gui;

    const GuiMenu menu;
    SetupNextWindowCenter({GetMainMenuWidth(), GetMainMenuHeight()});
    menu.Begin("_options_menu");

    const GuiTitle title;
    title.Begin(GetLocalText(common, proto::LabelNames::kMenuOptions));

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuOptionChangeKeybind))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::option_change_keybind;
    }

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuOptionToggleFullscreen))) {
        common.gameController.event.SubscribeOnce(game::EventType::renderer_tick, [](const game::EventBase& e) {
            const auto& render_e = SafeCast<const game::RendererTickEvent&>(e);
            auto& window         = render_e.windows[0].get();

            window.SetFullscreen(!window.IsFullscreen());
        });
    }

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuOptionChangeLanguage))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::option_change_language;
    }

    MenuBackButton(common.mainMenuData, MainMenuData::Window::main);
}

/// \return true if a submenu was drawn
static bool DrawSubmenu(ThreadedLoopCommon& common) {
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
    case gui::MainMenuData::Window::option_change_language:
        OptionLanguageMenu(common);
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
    title.Begin(std::string("Jactorio | ") + CConfig::kVersion + " Build: " + std::to_string(CConfig::kBuildNumber));

#ifdef JACTORIO_DEBUG_BUILD
    ImGui::TextUnformatted("NOTE: Debug build");

    if (MenuButton("Debug start game")) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::in_world);
    }
#endif

    // ======================================================================

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuNewGame))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::new_game;
    }

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuLoadGame))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::load_game;
    }

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuOptions))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::options;
    }

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuQuit))) {
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
    title.Begin(GetLocalText(common, proto::LabelNames::kMenuMain));


    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuLoadGame))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::load_game;
    }

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuSaveGame))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::save_game;
    }

    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuOptions))) {
        common.mainMenuData.currentMenu = MainMenuData::Window::options;
    }


    if (MenuButton(GetLocalText(common, proto::LabelNames::kMenuQuit))) {
        ChangeGameState(common, ThreadedLoopCommon::GameState::main_menu);
    }
}
