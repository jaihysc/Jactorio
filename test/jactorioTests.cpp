// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include <filesystem>

#include "core/crash_handler.h"

int main(int ac, char* av[]) {
    current_path(std::filesystem::path(av[0]).parent_path());

    jactorio::RegisterCrashHandler();

    testing::InitGoogleTest(&ac, av);
    RUN_ALL_TESTS();
    return 0; // Always return 0 even if tests fail, as a non-zero return code is seen as abnormal test suite exit
}
