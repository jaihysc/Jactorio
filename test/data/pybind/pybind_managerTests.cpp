// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/09/2019

#include <gtest/gtest.h>

#include "data/data_exception.h"
#include "data/pybind/pybind_manager.h"

namespace data
{
	class PybindManagerTest : public testing::Test
	{
	protected:
		void SetUp() override {
			jactorio::data::py_interpreter_init();
		}

		void TearDown() override {
			jactorio::data::py_interpreter_terminate();
		}
	};


	TEST_F(PybindManagerTest, InvalidPythonStr) {
		bool caught = false;
		try {
			jactorio::data::py_exec("asdf");
		}
		catch (jactorio::data::Data_exception& e) {
			caught = true;
		}

		if (!caught) {
			FAIL();  // Failed to throw exception on error
		}
	}

	TEST_F(PybindManagerTest, ValidPythonStr) {
		EXPECT_EQ(jactorio::data::py_exec("print(\"Hello\")"), 0);
	}
}
