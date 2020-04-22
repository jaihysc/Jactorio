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
		void TearDown() override {
			jactorio::game::input_manager::clear_data();
		}
	};

	TEST_F(InputManagerTest, InputCallbackRegister) {
		// Key is converted to a scancode
		const unsigned int callback_id = jactorio::game::input_manager::subscribe(
			test_input_callback,
			GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SHIFT);

		// callback_id should not be 0
		EXPECT_NE(callback_id, 0);

		jactorio::game::input_manager::set_input(GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SHIFT);
		jactorio::game::input_manager::raise();
		EXPECT_EQ(test_val, 1);
	}

	namespace
	{
		int counter = 0;

		void test_callback2() {
			counter++;
		}

		void test_callback3() {
			counter++;
		}
	}

	TEST_F(InputManagerTest, DispatchInputCallbacks) {
		counter = 0;

		jactorio::game::input_manager::subscribe(test_callback2, GLFW_KEY_V, GLFW_PRESS);
		jactorio::game::input_manager::subscribe(test_callback2, GLFW_KEY_Z, GLFW_PRESS);
		jactorio::game::input_manager::subscribe(test_callback3, GLFW_KEY_X, GLFW_RELEASE, GLFW_MOD_CAPS_LOCK);

		jactorio::game::input_manager::set_input(GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SUPER);
		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 0);


		jactorio::game::input_manager::set_input(GLFW_KEY_V, GLFW_PRESS, GLFW_MOD_SHIFT);
		jactorio::game::input_manager::raise();
		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 0);

		// Callback2 called once
		jactorio::game::input_manager::set_input(GLFW_KEY_V, GLFW_PRESS);
		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 1);
		jactorio::game::input_manager::set_input(GLFW_KEY_V, GLFW_RELEASE);


		// The GLFW_RELEASE action is only ever calls callbacks once
		jactorio::game::input_manager::set_input(GLFW_KEY_X, GLFW_RELEASE, GLFW_MOD_CAPS_LOCK);
		jactorio::game::input_manager::raise();
		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 2);


		// Inputs stack until released
		jactorio::game::input_manager::set_input(GLFW_KEY_V, GLFW_PRESS);
		jactorio::game::input_manager::set_input(GLFW_KEY_Z, GLFW_PRESS);
		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 4);

	}

	TEST_F(InputManagerTest, input_callback_press_first) {
		counter = 0;
		// The action GLFW_PRESS_FIRST will only raise once compared to GLFW_PRESS which repeatedly raises

		jactorio::game::input_manager::subscribe([]() {  // Callback 1
			counter -= 50;
		}, GLFW_KEY_V, GLFW_PRESS_FIRST, GLFW_MOD_SUPER);

		jactorio::game::input_manager::subscribe([]() {  // Callback 2
			counter += 100;
		}, GLFW_KEY_V, GLFW_PRESS, GLFW_MOD_SUPER);


		jactorio::game::input_manager::set_input(GLFW_KEY_V, GLFW_PRESS, GLFW_MOD_SUPER);
		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 50);  // Callback 1 + 2 called

		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 150);  // Callback 2 called

		jactorio::game::input_manager::raise();
		EXPECT_EQ(counter, 250);  // Callback 2 called
	}

	// TEST_F(InputManagerTest, dispatch_input_callbacks_imgui_block) {
	// 	auto guard = jactorio::core::Resource_guard(&input::clear_data);
	//
	// 	counter = 0;
	// 	input::subscribe(test_callback2, GLFW_KEY_V, GLFW_PRESS);
	//
	// 	input::set_input(GLFW_KEY_V, GLFW_PRESS);
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

	namespace
	{
		int counter2 = 0;

		void test_callback5() {
			counter2++;
		}
	}

	TEST_F(InputManagerTest, remove_input_callback) {
		counter = 0;

		const unsigned int callback_id = 
		  jactorio::game::input_manager::subscribe(test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);

		jactorio::game::input_manager::subscribe(test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);

		jactorio::game::input_manager::unsubscribe(callback_id, GLFW_KEY_SPACE, GLFW_PRESS);

		jactorio::game::input_manager::set_input(GLFW_KEY_SPACE, GLFW_PRESS);
		jactorio::game::input_manager::raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter2, 1);
	}

	TEST_F(InputManagerTest, clear_data) {
		counter2 = 0;

		jactorio::game::input_manager::subscribe(test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);
		jactorio::game::input_manager::subscribe(test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);

		jactorio::game::input_manager::clear_data();

		// Should not increment counter2 since all callbacks were cleared
		jactorio::game::input_manager::set_input(GLFW_KEY_SPACE, GLFW_PRESS);
		jactorio::game::input_manager::raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter2, 0);
	}
}
