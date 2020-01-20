#include <gtest/gtest.h>

#include "data/pybind/pybind_manager.h"

namespace data
{
	TEST(pybind_manager, invalid_python_str) {
		jactorio::data::pybind_manager::py_interpreter_init();
		EXPECT_NE(jactorio::data::pybind_manager::exec("asdf"), 0);

		// result is the error
		EXPECT_EQ(jactorio::data::pybind_manager::get_last_error_message().empty(), false);
		jactorio::data::pybind_manager::py_interpreter_terminate();
	}

	TEST(pybind_manager, valid_python_str) {
		jactorio::data::pybind_manager::py_interpreter_init();
		EXPECT_EQ(jactorio::data::pybind_manager::exec("print(\"Hello\")"), 0);

		jactorio::data::pybind_manager::py_interpreter_terminate();
	}
}
