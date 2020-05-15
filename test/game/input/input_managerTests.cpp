// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#include <gtest/gtest.h>

#include "jactorio.h"

#include "game/input/input_manager.h"
#include "renderer/gui/imgui_manager.h"

namespace game
{
	namespace
	{
		int test_val = 0;

		void TestInputCallback() {
			test_val = 1;
		}
	}

	class InputManagerTest : public testing::Test
	{
	protected:
		jactorio::game::KeyInput keyInput_;
		int counter_ = 0;

		void TearDown() override {
			keyInput_.ClearData();
		}
	};

	TEST_F(InputManagerTest, InputCallbackRegister) {
		// Key is converted to a scancode
		const unsigned int callback_id =
			keyInput_.Subscribe(TestInputCallback,
			                    jactorio::game::InputKey::t,
			                    jactorio::game::InputAction::key_down,
			                    jactorio::game::InputMod::shift);

		// callback_id should not be 0
		EXPECT_NE(callback_id, 0);

		keyInput_.SetInput(jactorio::game::InputKey::t,
		                   jactorio::game::InputAction::key_down,
		                   jactorio::game::InputMod::shift);
		keyInput_.Raise();
		EXPECT_EQ(test_val, 1);
	}

	TEST_F(InputManagerTest, DispatchInputCallbacks) {
		keyInput_.Subscribe([&]() { counter_++; },
		                    jactorio::game::InputKey::v,
		                    jactorio::game::InputAction::key_down);
		keyInput_.Subscribe([&]() { counter_++; },
		                    jactorio::game::InputKey::z,
		                    jactorio::game::InputAction::key_down);

		keyInput_.Subscribe([&]() { counter_++; },
		                    jactorio::game::InputKey::x,
		                    jactorio::game::InputAction::key_up,
		                    jactorio::game::InputMod::caps_lk);

		// ======================================================================

		keyInput_.SetInput(jactorio::game::InputKey::t,
		                   jactorio::game::InputAction::key_down,
		                   jactorio::game::InputMod::super);
		keyInput_.Raise();  // Nothing called
		EXPECT_EQ(counter_, 0);


		keyInput_.SetInput(jactorio::game::InputKey::v,
		                   jactorio::game::InputAction::key_down,
		                   jactorio::game::InputMod::caps_lk);
		keyInput_.Raise();
		keyInput_.Raise();
		EXPECT_EQ(counter_, 0);

		// Callback2 called once
		keyInput_.SetInput(jactorio::game::InputKey::v, jactorio::game::InputAction::key_down);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 1);
		keyInput_.SetInput(jactorio::game::InputKey::v, jactorio::game::InputAction::key_up);


		// The RELEASE action is only ever calls callbacks once
		keyInput_.SetInput(jactorio::game::InputKey::x,
		                   jactorio::game::InputAction::key_up,
		                   jactorio::game::InputMod::caps_lk);
		keyInput_.Raise();
		keyInput_.Raise();
		EXPECT_EQ(counter_, 2);


		// Inputs stack until released
		keyInput_.SetInput(jactorio::game::InputKey::v, jactorio::game::InputAction::key_down);
		keyInput_.SetInput(jactorio::game::InputKey::z, jactorio::game::InputAction::key_down);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 4);

	}

	TEST_F(InputManagerTest, InputCallbackPressFirst) {
		// The action key_down will only raise once compared to key_pressed which repeatedly raises after the initial raise

		keyInput_.Subscribe(
			[&]() {  // Callback 1
				counter_ -= 50;
			},
			jactorio::game::InputKey::v,
			jactorio::game::InputAction::key_down,
			jactorio::game::InputMod::super);

		keyInput_.Subscribe(
			[&]() {  // Callback 2
				counter_ += 100;
			},
			jactorio::game::InputKey::v,
			jactorio::game::InputAction::key_pressed,
			jactorio::game::InputMod::super);


		keyInput_.SetInput(jactorio::game::InputKey::v,
		                   jactorio::game::InputAction::key_down,
		                   jactorio::game::InputMod::super);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 50);  // Callback 1 + 2 called

		keyInput_.Raise();
		EXPECT_EQ(counter_, 150);  // Callback 2 called

		keyInput_.Raise();
		EXPECT_EQ(counter_, 250);  // Callback 2 called
	}

	TEST_F(InputManagerTest, InputCallbackHeld) {
		// The action key_held applies for both key_pressed and key_released
		keyInput_.Subscribe(
			[&]() {
				counter_ += 1;
			},
			jactorio::game::InputKey::v,
			jactorio::game::InputAction::key_held);


		keyInput_.SetInput(jactorio::game::InputKey::v,
		                   jactorio::game::InputAction::key_down);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 1);

		// event emitted will be Key_pressed since this is the second time
		keyInput_.Raise();
		EXPECT_EQ(counter_, 2);

		keyInput_.SetInput(jactorio::game::InputKey::v,
		                   jactorio::game::InputAction::key_repeat);
		keyInput_.Raise();
		EXPECT_EQ(counter_, 3);
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
		const unsigned int callback_id =
			keyInput_.Subscribe([&]() { counter_++; },
			                    jactorio::game::InputKey::space,
			                    jactorio::game::InputAction::key_pressed);

		keyInput_.Subscribe([&]() { counter_++; },
		                    jactorio::game::InputKey::space,
		                    jactorio::game::InputAction::key_pressed);

		keyInput_.Unsubscribe(callback_id,
		                      jactorio::game::InputKey::space,
		                      jactorio::game::InputAction::key_pressed);

		keyInput_.SetInput(jactorio::game::InputKey::space, jactorio::game::InputAction::key_down);
		keyInput_.Raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter_, 1);
	}

	TEST_F(InputManagerTest, ClearData) {
		keyInput_.Subscribe([&]() { counter_++; },
		                    jactorio::game::InputKey::space,
		                    jactorio::game::InputAction::key_pressed);
		keyInput_.Subscribe([&]() { counter_++; },
		                    jactorio::game::InputKey::space,
		                    jactorio::game::InputAction::key_pressed);

		keyInput_.ClearData();

		// Should not increment counter2 since all callbacks were cleared
		keyInput_.SetInput(jactorio::game::InputKey::space, jactorio::game::InputAction::key_down);
		keyInput_.Raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter_, 0);
	}
}
