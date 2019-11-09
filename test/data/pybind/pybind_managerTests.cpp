#include <gtest/gtest.h>

#include "data/pybind/pybind_manager.h"

TEST(pybind_manager, invalid_python_str) {
	jactorio::data::pybind_manager::py_interpreter_init();
	const auto result = jactorio::data::pybind_manager::exec("asdf");

	// result is the error
	EXPECT_EQ(result.empty(), false);
	jactorio::data::pybind_manager::py_interpreter_terminate();
}

TEST(pybind_manager, valid_python_str) {
	jactorio::data::pybind_manager::py_interpreter_init();
	const auto result = jactorio::data::pybind_manager::exec("print(\"Hello\")");

	// result should be empty since input is valid
	EXPECT_EQ(result.empty(), true);
	jactorio::data::pybind_manager::py_interpreter_terminate();
}