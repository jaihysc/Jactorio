#include <gtest/gtest.h>

#include "GLFW/glfw3.h"

#include "game/input/input_manager.h"

namespace game
{
	namespace input = jactorio::game::input_manager;


	namespace
	{
		int test_val = 0;

		void test_input_callback() {
			test_val = 1;
		}
	}

	TEST(input_manager, input_callback_register) {
		// Key is converted to a scancode
		const unsigned int callback_id = input::subscribe(
			test_input_callback,
			GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SHIFT);

		// callback_id should not be 0
		EXPECT_NE(callback_id, 0);

		input::set_input(GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SHIFT);
		input::raise();
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

	TEST(input_manager, dispatch_input_callbacks) {
		input::subscribe(test_callback2, GLFW_KEY_V, GLFW_PRESS);
		input::subscribe(test_callback2, GLFW_KEY_Z, GLFW_PRESS);
		input::subscribe(test_callback3,
		                               GLFW_KEY_X, GLFW_RELEASE, GLFW_MOD_CAPS_LOCK);

		input::set_input(GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SUPER);
		input::raise();
		EXPECT_EQ(counter, 0);


		input::set_input(GLFW_KEY_V, GLFW_PRESS, GLFW_MOD_SHIFT);
		input::raise();
		input::raise();
		EXPECT_EQ(counter, 0);

		// Callback2 called once
		input::set_input(GLFW_KEY_V, GLFW_PRESS);
		input::raise();
		EXPECT_EQ(counter, 1);
		input::set_input(GLFW_KEY_V, GLFW_RELEASE);


		// The GLFW_RELEASE action is only ever calls callbacks once
		input::set_input(GLFW_KEY_X, GLFW_RELEASE, GLFW_MOD_CAPS_LOCK);
		input::raise();
		input::raise();
		EXPECT_EQ(counter, 2);


		// Inputs stack until released
		input::set_input(GLFW_KEY_V, GLFW_PRESS);
		input::set_input(GLFW_KEY_Z, GLFW_PRESS);
		input::raise();
		EXPECT_EQ(counter, 4);

	}


	namespace
	{
		int counter2 = 0;

		void test_callback5() {
			counter2++;
		}
	}

	TEST(input_manager, remove_input_callback) {
		const unsigned int callback_id = input::subscribe(
			test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);
		input::subscribe(test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);

		input::unsubscribe(callback_id, GLFW_KEY_SPACE, GLFW_PRESS);

		input::set_input(GLFW_KEY_SPACE, GLFW_PRESS);
		input::raise();

		// Only one of the callbacks was erased, therefore it should increment counter2 only once
		// instead of twice
		EXPECT_EQ(counter2, 1);
	}
}
