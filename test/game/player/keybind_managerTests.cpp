// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/keybind_manager.h"

#include "game/game_data.h"
#include "game/input/input_manager.h"

namespace jactorio::game
{
    class KeybindManagerTest : public testing::Test
    {
    protected:
        InputManager inputManager_;
        GameDataGlobal dataGlobal_;
        KeybindManager keybindManager_{inputManager_, dataGlobal_};

        ///
        /// Expects the PlayerAction test to be called after executing provided function f
        void ExpectTestActionCalled(const std::function<void()>& f) const {
            EXPECT_FLOAT_EQ(dataGlobal_.player.world.GetPositionX(), 0.f);
            EXPECT_FLOAT_EQ(dataGlobal_.player.world.GetPositionY(), 0.f);

            f();

            EXPECT_FLOAT_EQ(dataGlobal_.player.world.GetPositionX(), -100.f);
            EXPECT_FLOAT_EQ(dataGlobal_.player.world.GetPositionY(), 120.f);
        }
    };

    ///
    /// Should set a keyboard input to trigger the action
    TEST_F(KeybindManagerTest, ChangeActionInput) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, SDLK_0, InputAction::key_down);

        ExpectTestActionCalled([this]() {
            InputManager::SetInput(SDLK_0, InputAction::key_down);
            inputManager_.Raise();
        });
    }

    ///
    /// Should set a mouse input to trigger the action
    TEST_F(KeybindManagerTest, MouseChangeActionInput) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, MouseInput::left, InputAction::key_down, KMOD_LALT);

        ExpectTestActionCalled([this]() {
            InputManager::SetInput(MouseInput::left, InputAction::key_down, KMOD_LALT);
            inputManager_.Raise();
        });
    }

    ///
    /// The previous input for an action should be unsubscribed from and no longer trigger the action
    TEST_F(KeybindManagerTest, UnsubscribePreviousInput) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, SDLK_0, InputAction::key_down);
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, SDLK_1, InputAction::key_up, KMOD_CAPS);
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, MouseInput::right, InputAction::key_up);

        // Does nothing because unsubscribed
        InputManager::SetInput(SDLK_0, InputAction::key_down);
        inputManager_.Raise();

        InputManager::SetInput(SDLK_1, InputAction::key_up, KMOD_CAPS);
        inputManager_.Raise();

        ExpectTestActionCalled([this]() {
            InputManager::SetInput(MouseInput::right, InputAction::key_up);
            inputManager_.Raise();
        });
    }

    ///
    /// Should change only the key of a player action
    TEST_F(KeybindManagerTest, ChangeActionKeyboardKey) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, SDLK_0, InputAction::key_down);

        keybindManager_.ChangeActionKey(PlayerAction::Type::test, SDLK_1);

        ExpectTestActionCalled([this]() {
            InputManager::SetInput(SDLK_1, InputAction::key_down);
            inputManager_.Raise();
        });
    }

    ///
    /// Should change only the key of a player action
    TEST_F(KeybindManagerTest, ChangeActionMouseKey) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, MouseInput::left, InputAction::key_up);

        keybindManager_.ChangeActionKey(PlayerAction::Type::test, MouseInput::right);

        ExpectTestActionCalled([this]() {
            InputManager::SetInput(MouseInput::right, InputAction::key_up);
            inputManager_.Raise();
        });
    }

    ///
    /// Should change only the key action of a player action
    TEST_F(KeybindManagerTest, ChangeActionKeyActionKeyboardKey) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, SDLK_a, InputAction::key_pressed);

        keybindManager_.ChangeActionKeyAction(PlayerAction::Type::test, InputAction::key_up);

        ExpectTestActionCalled([this]() {
            InputManager::SetInput(SDLK_a, InputAction::key_up);
            inputManager_.Raise();
        });
    }

    ///
    /// Should change only the key action of a player action
    TEST_F(KeybindManagerTest, ChangeActionKeyActionMouseKey) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, MouseInput::left, InputAction::key_pressed);

        keybindManager_.ChangeActionKeyAction(PlayerAction::Type::test, InputAction::key_up);

        // keyboard button should not trigger keybind on mouse button
        InputManager::SetInput(static_cast<SDL_KeyCode>(MouseInput::left), // Has same numerical value
                               InputAction::key_up);
        inputManager_.Raise();


        ExpectTestActionCalled([this]() {
            InputManager::SetInput(MouseInput::left, InputAction::key_up);
            inputManager_.Raise();
        });
    }

    ///
    /// Should change only the mod of a player action
    TEST_F(KeybindManagerTest, ChangeActionMods) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, MouseInput::left, InputAction::key_down);

        keybindManager_.ChangeActionMods(PlayerAction::Type::test, KMOD_LALT);

        ExpectTestActionCalled([this]() {
            InputManager::SetInput(MouseInput::left, InputAction::key_down, KMOD_LALT);
            inputManager_.Raise();
        });
    }

    ///
    /// Should return information about all keybinds
    TEST_F(KeybindManagerTest, GetKeybindInfo) {
        keybindManager_.ChangeActionInput(PlayerAction::Type::activate_layer, MouseInput::right, InputAction::key_up);
        keybindManager_.ChangeActionInput(PlayerAction::Type::test, SDLK_0, InputAction::key_down);

        const auto& info = keybindManager_.GetKeybindInfo();

        EXPECT_EQ(info.size(), PlayerAction::kActionCount_);
    }
} // namespace jactorio::game