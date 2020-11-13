// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/pybind_manager.h"
#include "proto/detail/exception.h"

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
        catch (proto::ProtoError&) {
            caught = true;
        }

        if (!caught) {
            FAIL(); // Failed to throw exception on error
        }
    }

    TEST_F(PybindManagerTest, ValidPythonStr) {
        EXPECT_EQ(PyExec("print(\"Hello\")"), 0);
    }
} // namespace jactorio::data
