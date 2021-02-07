// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/resource_guard.h"

#include <string>

namespace jactorio
{
    namespace
    {
        bool non_capturing_called = false;
    }

    TEST(ResourceGuardTest, CallDestructor) {
        {
            auto guard = ResourceGuard<void>([]() { non_capturing_called = true; });
        }

        EXPECT_TRUE(non_capturing_called);
    }

    TEST(ResourceGuardTest, DeduceTemplateArgs) {
        auto guard  = ResourceGuard(+[]() {});
        auto guard2 = ResourceGuard(+[]() -> int { return 0; });
        auto guard3 = ResourceGuard(+[]() -> std::string { return ""; });
    }

    TEST(ResourceGuardTest, CapturingCallDestructor) {
        bool called = false;
        {
            auto guard = CapturingGuard<void()>([&] { called = true; });
        }

        EXPECT_TRUE(called);
    }
} // namespace jactorio
