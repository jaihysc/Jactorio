// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/keybind_manager.h"

#include "game/game_data.h"
#include "game/input/input_manager.h"

namespace jactorio::game
{
    ///
    /// Should set a input to trigger the action
    TEST(KeybindManager, ChangeActionInput) {
        InputManager input_manager;
        GameDataGlobal data_global;
        KeybindManager keybind_manager(input_manager, data_global);

        keybind_manager.ChangeActionInput(PlayerAction::Type::test, SDLK_0, InputAction::key_down);

        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionX(), 0.f);
        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionY(), 0.f);

        InputManager::SetInput(SDLK_0, InputAction::key_down);
        input_manager.Raise();

        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionX(), -100.f);
        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionY(), 120.f);
    }

    ///
    /// The previous input for an action should be unsubscribed from and no longer trigger the action
    TEST(KeybindManager, UnsubscribePreviousInput) {
        InputManager input_manager;
        GameDataGlobal data_global;
        KeybindManager keybind_manager(input_manager, data_global);

        keybind_manager.ChangeActionInput(PlayerAction::Type::test, SDLK_0, InputAction::key_down);
        keybind_manager.ChangeActionInput(PlayerAction::Type::test, SDLK_1, InputAction::key_up, KMOD_CAPS);

        InputManager::SetInput(SDLK_0, InputAction::key_down);
        input_manager.Raise();

        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionX(), 0.f);
        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionY(), 0.f);

        InputManager::SetInput(SDLK_1, InputAction::key_up, KMOD_CAPS);
        input_manager.Raise();

        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionX(), -100.f);
        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionY(), 120.f);
    }
} // namespace jactorio::game