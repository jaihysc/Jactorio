// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/09/2019

#include <gtest/gtest.h>

#include "core/resource_guard.h"
#include "data/data_exception.h"
#include "data/pybind/pybind_manager.h"

namespace data
{
	TEST(pybind_manager, invalid_python_str) {
		jactorio::core::Resource_guard<void> guard(
			[]() { jactorio::data::pybind_manager::py_interpreter_terminate(); });
		jactorio::data::pybind_manager::py_interpreter_init();

		bool caught = false;
		try {
			jactorio::data::pybind_manager::exec("asdf");
		}
		catch (jactorio::data::Data_exception& e) {
			caught = true;
		}

		if (!caught) {
			FAIL();  // Failed to throw exception on error
		}
	}

	TEST(pybind_manager, valid_python_str) {
		jactorio::core::Resource_guard<void> guard(
			[]() { jactorio::data::pybind_manager::py_interpreter_terminate(); });

		jactorio::data::pybind_manager::py_interpreter_init();
		EXPECT_EQ(jactorio::data::pybind_manager::exec("print(\"Hello\")"), 0);
	}
}
