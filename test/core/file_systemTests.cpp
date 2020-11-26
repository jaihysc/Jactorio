// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/filesystem.h"

namespace jactorio::core
{
    class FileSystemTest : public testing::Test
    {
        std::string originalExecutingDir_;
    };

    TEST_F(FileSystemTest, ReadFileStrInvalidPath) {
        if (!ReadFile("").empty() || !ReadFile("someMysteriousFilePathThatDOESnotEXIST").empty()) {
            FAIL();
        }

        // Test does not check if the read contents are valid, it is assumed to be valid
    }
} // namespace jactorio::core
