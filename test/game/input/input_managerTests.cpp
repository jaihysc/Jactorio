// 
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

		void test_input_callback() {
			test_val = 1;
		}
	}

	class InputManagerTest : public testing::Test
	{
	protected:
		jactorio::game::Key_input key_input_;
		int counter_ = 0;

		void TearDown() override {
			key_input_.clear_data();
		}
	};

	TEST_F(InputManagerTest, InputCallbackRegister) {
		// Key is converted to a scancode
		const unsigned int callback_id =
			key_input_.subscribe(test_input_callback,
			                     jactorio::game::inputKey::t,
			                     jactorio::game::inputAction::key_down,
			                     jactorio::game::inputMod::shift);

		// callback_id should not be 0
		EXPECT_NE(callback_id, 0);

		key_input_.set_input(jactorio::game::inputKey::t,
		                     jactorio::game::inputAction::key_down,
		                     jactorio::game::inputMod::shift);
		key_input_.raise();
		EXPECT_EQ(test_val, 1);
	}

	TEST_F(InputManagerTest, DispatchInputCallbacks) {
		key_input_.subscribe([&]() { counter_++; },
		                     jactorio::game::inputKey::v,
		                     jactorio::game::inputAction::key_down);
		key_input_.subscribe([&]() { counter_++; },
		                     jactorio::game::inputKey::z,
		                     jactorio::game::inputAction::key_down);

		key_input_.subscribe([&]() { counter_++; },
		                     jactorio::game::inputKey::x,
		                     jactorio::game::inputAction::key_up,
		                     jactorio::game::inputMod::caps_lk);

		// ======================================================================

		key_input_.set_input(jactorio::game::inputKey::t,
		                     jactorio::game::inputAction::key_down,
		                     jactorio::game::inputMod::super);
		key_input_.raise();  // Nothing called
		EXPECT_EQ(counter_, 0);


		key_input_.set_input(jactorio::game::inputKey::v,
		                     jactorio::game::inputAction::key_down,
		                     jactorio::game::inputMod::caps_lk);
		key_input_.raise();
		key_input_.raise();
		EXPECT_EQ(counter_, 0);

		// Callback2 called once
		key_input_.set_input(jactorio::game::inputKey::v, jactorio::game::inputAction::key_down);
		key_input_.raise();
		EXPECT_EQ(counter_, 1);
		key_input_.set_input(jactorio::game::inputKey::v, jactorio::game::inputAction::key_up);


		// The RELEASE action is only ever calls callbacks once
		key_input_.set_input(jactorio::game::inputKey::x,
		                     jactorio::game::inputAction::key_up,
		                     jactorio::game::inputMod::caps_lk);
		key_input_.raise();
		key_input_.raise();
		EXPECT_EQ(counter_, 2);


		// Inputs stack until released
		key_input_.set_input(jactorio::game::inputKey::v, jactorio::game::inputAction::key_down);
		key_input_.set_input(jactorio::game::inputKey::z, jactorio::game::inputAction::key_down);
		key_input_.raise();
		EXPECT_EQ(counter_, 4);

	}

	TEST_F(InputManagerTest, InputCallbackPressFirst) {
		// The action key_down will only raise once compared to key_pressed which repeatedly raises after the initial raise

		key_input_.subscribe(
			[&]() {  // Callback 1
				counter_ -= 50;
			},
			jactorio::game::inputKey::v,
			jactorio::game::inputAction::key_down,
			jactorio::game::inputMod::super);

		key_input_.subscribe(
			[&]() {  // Callback 2
				counter_ += 100;
			},
			jactorio::game::inputKey::v,
			jactorio::game::inputAction::key_pressed,
			jactorio::game::inputMod::super);


		key_input_.set_input(jactorio::game::inputKey::v,
		                     jactorio::game::inputAction::key_down,
		                     jactorio::game::inputMod::super);
		key_input_.raise();
		EXPECT_EQ(counter_, 50);  // Callback 1 + 2 called

		key_input_.raise();
		EXPECT_EQ(counter_, 150);  // Callback 2 called

		key_input_.raise();
		EXPECT_EQ(counter_, 250);  // Callback 2 called
	}

	TEST_F(InputManagerTest, InputCallbackHeld) {
		// The action key_held applies for both key_pressed and key_released
		key_input_.subscribe(
			[&]() {
				counter_ += 1;
			},
			jactorio::game::inputKey::v,
			jactorio::game::inputAction::key_held);


		key_input_.set_input(jactorio::game::inputKey::v,
		                     jactorio::game::inputAction::key_down);
		key_input_.raise();
		EXPECT_EQ(counter_, 1);

		// event emitted will be Key_pressed since this is the second time
		key_input_.raise();
		EXPECT_EQ(counter_, 2);

		key_input_.set_input(jactorio::game::inputKey::v,
		                     jactorio::game::inputAction::key_repeat);
		key_input_.raise();
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
			key_input_.subscribe([&]() { counter_++; },
			                     jactorio::game::inputKey::space,
			                     jactorio::game::inputAction::key_pressed);

		key_input_.subscribe([&]() { counter_++; },
		                     jactorio::game::inputKey::space,
		                     jactorio::game::inputAction::key_pressed);

		key_input_.unsubscribe(callback_id,
		                       jactorio::game::inputKey::space,
		                       jactorio::game::inputAction::key_pressed);

		key_input_.set_input(jactorio::game::inputKey::space, jactorio::game::inputAction::key_down);
		key_input_.raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter_, 1);
	}

	TEST_F(InputManagerTest, ClearData) {
		key_input_.subscribe([&]() { counter_++; },
		                     jactorio::game::inputKey::space,
		                     jactorio::game::inputAction::key_pressed);
		key_input_.subscribe([&]() { counter_++; },
		                     jactorio::game::inputKey::space,
		                     jactorio::game::inputAction::key_pressed);

		key_input_.clear_data();

		// Should not increment counter2 since all callbacks were cleared
		key_input_.set_input(jactorio::game::inputKey::space, jactorio::game::inputAction::key_down);
		key_input_.raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter_, 0);
	}
}
