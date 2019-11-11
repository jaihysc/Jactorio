#include <gtest/gtest.h>

#include "GLFW/glfw3.h"

#include "game/input/input_manager.h"

namespace input = jactorio::game::input_manager;


int test_val = 0;

void test_input_callback() {
	test_val = 1;
}

TEST(input_manager, input_callback_register) {
	// Key is converted to a scancode
	const unsigned int callback_id = input::register_input_callback(
		test_input_callback,
		GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SHIFT);

	// callback_id should not be 0
	EXPECT_NE(callback_id, 0);
	
	input::set_input(GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SHIFT);
	input::dispatch_input_callbacks();
	EXPECT_EQ(test_val, 1);
}

int counter = 0;
void test_callback2() {
	counter++;
}
void test_callback3() {
	counter++;
}

TEST(input_manager, dispatch_input_callbacks) {
	input::register_input_callback(test_callback2, GLFW_KEY_V, GLFW_PRESS);
	input::register_input_callback(test_callback3, 
	                               GLFW_KEY_X, GLFW_RELEASE, GLFW_MOD_CAPS_LOCK);

	input::set_input(GLFW_KEY_T, GLFW_PRESS, GLFW_MOD_SUPER);
	input::dispatch_input_callbacks();
	EXPECT_EQ(counter, 0);

	
	input::set_input(GLFW_KEY_V, GLFW_PRESS, GLFW_MOD_SHIFT);
	input::dispatch_input_callbacks();
	input::dispatch_input_callbacks();
	EXPECT_EQ(counter, 0);

	// Callback2 called once
	input::set_input(GLFW_KEY_V, GLFW_PRESS);
	input::dispatch_input_callbacks();
	EXPECT_EQ(counter, 1);

	// Callback3 called twice
	input::set_input(GLFW_KEY_X, GLFW_RELEASE, GLFW_MOD_CAPS_LOCK);
	input::dispatch_input_callbacks();
	input::dispatch_input_callbacks();
	EXPECT_EQ(counter, 3);
}


int counter2 = 0;
void test_callback5() {
	counter2++;
}

TEST(input_manager, remove_input_callback) {
	const unsigned int callback_id = input::register_input_callback(
		test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);
	input::register_input_callback(test_callback5, GLFW_KEY_SPACE, GLFW_PRESS);
	
	input::remove_input_callback(callback_id, GLFW_KEY_SPACE, GLFW_PRESS);
	
	input::set_input(GLFW_KEY_SPACE, GLFW_PRESS);
	input::dispatch_input_callbacks();

	// Only one of the callbacks was erased, therefore it should increment counter2 only once
	// instead of twice
	EXPECT_EQ(counter2, 1);
}