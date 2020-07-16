// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/data_exception.h"
#include "data/pybind/pybind_manager.h"

namespace jactorio::data
{
	class PybindManagerTest : public testing::Test
	{
	protected:
		void SetUp() override {
			PyInterpreterInit();
		}

		void TearDown() override {
			PyInterpreterTerminate();
		}
	};


	TEST_F(PybindManagerTest, InvalidPythonStr) {
		bool caught = false;
		try {
			PyExec("asdf");
		}
		catch (DataException& e) {
			caught = true;
		}

		if (!caught) {
			FAIL();  // Failed to throw exception on error
		}
	}

	TEST_F(PybindManagerTest, ValidPythonStr) {
		EXPECT_EQ(jactorio::data::PyExec("print(\"Hello\")"), 0);
	}
}
