// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorio.h"

#include "game/input/input_manager.h"
#include "renderer/gui/imgui_manager.h"

namespace jactorio::game
{
	class InputManagerTest : public testing::Test
	{
	protected:
		KeyInput keyInput_;
		int counter_ = 0;

		void TearDown() override {
			keyInput_.ClearData();
		}
	};

	TEST_F(InputManagerTest, InputCallbackRegister) {
		int test_val = 0;

		const auto callback_id = keyInput_.Register(
			[&]() {
				test_val = 1;
			},
			SDLK_t, InputAction::key_down, KMOD_LSHIFT);

		// callback_id should not be 0
		EXPECT_NE(callback_id, 0);

		keyInput_.SetInput(SDLK_t,
		                   InputAction::key_down,
		                   KMOD_LSHIFT);
		keyInput_.Raise();
		EXPECT_EQ(test_val, 1);
	}

	TEST_F(InputManagerTest, DispatchInputCallbacks) {
		keyInput_.Register([&]() { counter_++; },
		                   SDLK_v,
		                   InputAction::key_down);
		keyInput_.Register([&]() { counter_++; },
		                   SDLK_z,
		                   InputAction::key_down);

		keyInput_.Register([&]() { counter_++; },
		                   SDLK_x,
		                   InputAction::key_up,
		                   KMOD_CAPS);

		// ======================================================================

		keyInput_.SetInput(SDLK_t,
		                   InputAction::key_down,
		                   KMOD_MODE);
		keyInput_.Raise();  // Nothing called
		EXPECT_EQ(counter_, 0);


		keyInput_.SetInput(SDLK_v,
		                   InputAction::key_down,
		                   KMOD_CAPS);
		keyInput_.Raise();
		keyInput_.Raise();
		EXPECT_EQ(counter_, 0);

		// Callback2 called once
		keyInput_.SetInput(SDLK_v, InputAction::key_down);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 1);
		keyInput_.SetInput(SDLK_v, InputAction::key_up);


		// The RELEASE action is only ever calls callbacks once
		keyInput_.SetInput(SDLK_x,
		                   InputAction::key_up,
		                   KMOD_CAPS);
		keyInput_.Raise();
		keyInput_.Raise();
		keyInput_.Raise();
		keyInput_.Raise();
		EXPECT_EQ(counter_, 2);


		// Inputs stack until released
		keyInput_.SetInput(SDLK_v, InputAction::key_down);
		keyInput_.SetInput(SDLK_z, InputAction::key_down);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 4);

	}

	TEST_F(InputManagerTest, InputCallbackPressFirst) {
		// The action key_down will only raise once compared to key_pressed which repeatedly raises after the initial raise

		keyInput_.Register(
			[&]() {  // Callback 1
				counter_ -= 50;
			},
			SDLK_v,
			InputAction::key_down,
			KMOD_MODE);

		keyInput_.Register(
			[&]() {  // Callback 2
				counter_ += 100;
			},
			SDLK_v,
			InputAction::key_pressed,
			KMOD_MODE);


		keyInput_.SetInput(SDLK_v,
		                   InputAction::key_down,
		                   KMOD_MODE);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 50);  // Callback 1 + 2 called

		keyInput_.Raise();
		EXPECT_EQ(counter_, 150);  // Callback 2 called

		keyInput_.Raise();
		EXPECT_EQ(counter_, 250);  // Callback 2 called
	}

	TEST_F(InputManagerTest, InputCallbackHeld) {
		// The action key_held applies for both key_pressed and key_released
		keyInput_.Register(
			[&]() {
				counter_ += 1;
			},
			SDLK_v,
			InputAction::key_held);


		keyInput_.SetInput(SDLK_v,
		                   InputAction::key_down);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 1);

		// event emitted will be Key_pressed since this is the second time
		keyInput_.Raise();
		EXPECT_EQ(counter_, 2);

		keyInput_.SetInput(SDLK_v,
		                   InputAction::key_repeat);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 3);
	}

	TEST_F(InputManagerTest, MouseButtonPress) {
		// Mouse buttons are stored as negative numbers, to avoid conflict with keyboard numbres
		int count = 0;

		keyInput_.Register([&]() {
			count = 1;
		}, MouseInput::left, InputAction::key_down);

		keyInput_.SetInput(MouseInput::left, InputAction::key_down);
		keyInput_.Raise();

		EXPECT_EQ(count, 1);
	}

	// TEST_F(InputManagerTest, dispatch_input_callbacks_imgui_block) {
	// 	auto guard = jactorio::core::Resource_guard(&input::clear_data);
	//
	// 	input::subscribe(test_callback2, GLFW_KEY_V, jactorio::game::inputAction::key_pressed);
	//
	// 	input::set_input(GLFW_KEY_V, jactorio::game::inputAction::key_pressed);
	//
	// 	// Block
	// 	jactorio::renderer::imgui_manager::input_captured = true;
	// 	input::raise();
	// 	EXPECT_EQ(counter, 0);
	//
	// 	// Unblock
	// 	jactorio::renderer::imgui_manager::input_captured = false;
	// 	input::raise();
	// 	EXPECT_EQ(counter, 1);
	// }

	TEST_F(InputManagerTest, RemoveInputCallback) {
		const auto callback_id =
			keyInput_.Register([&]() { counter_++; },
			                   SDLK_SPACE,
			                   InputAction::key_pressed);

		keyInput_.Register([&]() { counter_++; },
		                   SDLK_SPACE,
		                   InputAction::key_pressed);

		keyInput_.Unsubscribe(callback_id,
		                      SDLK_SPACE,
		                      InputAction::key_pressed);

		keyInput_.SetInput(SDLK_SPACE, InputAction::key_down);
		keyInput_.Raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter_, 1);
	}

	TEST_F(InputManagerTest, ClearData) {
		keyInput_.Register([&]() { counter_++; },
		                   SDLK_SPACE,
		                   InputAction::key_pressed);
		keyInput_.Register([&]() { counter_++; },
		                   SDLK_SPACE,
		                   InputAction::key_pressed);

		keyInput_.ClearData();

		// Should not increment counter2 since all callbacks were cleared
		keyInput_.SetInput(SDLK_SPACE, InputAction::key_down);
		keyInput_.Raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter_, 0);
	}
}
