// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorio.h"

#include "game/input/input_manager.h"
#include "gui/imgui_manager.h"

namespace jactorio::game
{
    class InputManagerTest : public testing::Test
    {
    protected:
        InputManager input_;
        int counter_ = 0;

        void TearDown() override {
            input_.Clear();
        }
    };

    TEST_F(InputManagerTest, InputCallbackRegister) {
        int test_val = 0;

        const auto callback_id = input_.Register([&]() { test_val = 1; }, SDLK_t, InputAction::key_down, KMOD_LSHIFT);

        // callback_id should not be 0
        EXPECT_NE(callback_id, 0);

        input_.SetInput(SDLK_t, InputAction::key_down, KMOD_LSHIFT);
        input_.Raise();
        EXPECT_EQ(test_val, 1);
    }

    TEST_F(InputManagerTest, DispatchInputCallbacks) {
        input_.Register([&]() { counter_++; }, SDLK_v, InputAction::key_down);
        input_.Register([&]() { counter_++; }, SDLK_z, InputAction::key_down);

        input_.Register([&]() { counter_++; }, SDLK_x, InputAction::key_up, KMOD_CAPS);

        // ======================================================================

        input_.SetInput(SDLK_t, InputAction::key_down, KMOD_MODE);
        input_.Raise(); // Nothing called
        EXPECT_EQ(counter_, 0);


        input_.SetInput(SDLK_v, InputAction::key_down, KMOD_CAPS);
        input_.Raise();
        input_.Raise();
        EXPECT_EQ(counter_, 0);

        // Callback2 called once
        input_.SetInput(SDLK_v, InputAction::key_down);
        input_.Raise();
        EXPECT_EQ(counter_, 1);
        input_.SetInput(SDLK_v, InputAction::key_up);


        // The RELEASE action is only ever calls callbacks once
        input_.SetInput(SDLK_x, InputAction::key_up, KMOD_CAPS);
        input_.Raise();
        input_.Raise();
        input_.Raise();
        input_.Raise();
        EXPECT_EQ(counter_, 2);


        // Inputs stack until released
        input_.SetInput(SDLK_v, InputAction::key_down);
        input_.SetInput(SDLK_z, InputAction::key_down);
        input_.Raise();
        EXPECT_EQ(counter_, 4);
    }

    TEST_F(InputManagerTest, InputCallbackPressFirst) {
        // The action key_down will only raise once compared to key_pressed which repeatedly raises after the initial
        // raise

        input_.Register(
            [&]() { // Callback 1
                counter_ -= 50;
            },
            SDLK_v,
            InputAction::key_down,
            KMOD_MODE);

        input_.Register(
            [&]() { // Callback 2
                counter_ += 100;
            },
            SDLK_v,
            InputAction::key_pressed,
            KMOD_MODE);


        input_.SetInput(SDLK_v, InputAction::key_down, KMOD_MODE);
        input_.Raise();
        EXPECT_EQ(counter_, 50); // Callback 1 + 2 called

        input_.Raise();
        EXPECT_EQ(counter_, 150); // Callback 2 called

        input_.Raise();
        EXPECT_EQ(counter_, 250); // Callback 2 called
    }

    TEST_F(InputManagerTest, InputCallbackHeld) {
        // The action key_held applies for both key_pressed and key_released
        input_.Register([&]() { counter_ += 1; }, SDLK_v, InputAction::key_held);


        input_.SetInput(SDLK_v, InputAction::key_down);
        input_.Raise();
        EXPECT_EQ(counter_, 1);

        // event emitted will be Key_pressed since this is the second time
        input_.Raise();
        EXPECT_EQ(counter_, 2);

        input_.SetInput(SDLK_v, InputAction::key_repeat);
        input_.Raise();
        EXPECT_EQ(counter_, 3);
    }

    TEST_F(InputManagerTest, MouseButtonPress) {
        // Mouse buttons are stored as negative numbers, to avoid conflict with keyboard numbres
        int count = 0;

        input_.Register([&]() { count = 1; }, MouseInput::left, InputAction::key_down);

        input_.SetInput(MouseInput::left, InputAction::key_down);
        input_.Raise();

        EXPECT_EQ(count, 1);
    }

    // TEST_F(InputManagerTest, dispatch_input_callbacks_imgui_block) {
    // 	auto guard = jactorio::Resource_guard(&input::clear_data);
    //
    // 	input::subscribe(test_callback2, GLFW_KEY_V, jactorio::game::inputAction::key_pressed);
    //
    // 	input::set_input(GLFW_KEY_V, jactorio::game::inputAction::key_pressed);
    //
    // 	// Block
    // 	jactorio::render::imgui_manager::input_captured = true;
    // 	input::raise();
    // 	EXPECT_EQ(counter, 0);
    //
    // 	// Unblock
    // 	jactorio::render::imgui_manager::input_captured = false;
    // 	input::raise();
    // 	EXPECT_EQ(counter, 1);
    // }

    TEST_F(InputManagerTest, RemoveInputCallback) {
        const auto callback_id = input_.Register([&]() { counter_++; }, SDLK_SPACE, InputAction::key_pressed);

        input_.Register([&]() { counter_++; }, SDLK_SPACE, InputAction::key_pressed);

        input_.Unsubscribe(callback_id);

        InputManager::SetInput(SDLK_SPACE, InputAction::key_down);
        input_.Raise();

        // key_down unsubscribed, before repeat still subscribed, increment only once instead of twice
        EXPECT_EQ(counter_, 1);
    }

    TEST_F(InputManagerTest, ClearData) {
        input_.Register([&]() { counter_++; }, SDLK_SPACE, InputAction::key_pressed);
        input_.Register([&]() { counter_++; }, SDLK_SPACE, InputAction::key_pressed);

        input_.Clear();

        // Should not increment counter2 since all callbacks were cleared
        input_.SetInput(SDLK_SPACE, InputAction::key_down);
        input_.Raise();

        // Only one of the callbacks was erased, therefore it should increment counter2 only once
        // instead of twice
        EXPECT_EQ(counter_, 0);
    }
} // namespace jactorio::game
