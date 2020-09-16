// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <gtest/gtest.h>

#include "core/convert.h"

namespace jactorio::core
{
    TEST(Math, SafeCast) {
        auto a = SafeCast<int>(42);
        auto c = SafeCast<uint64_t>(1); // Widening cast

        // const auto b = SafeCast<float>(static_cast<double>(321321312));  // Error: Must be integral
        // auto d = SafeCast<uint8_t>(256);  // Error: Loss of data
    }

    TEST(Math, SafeCastAssign) {
        auto a = 42;
        auto c = 1ull;

        SafeCastAssign(a, c);

        EXPECT_EQ(a, c);
    }
} // namespace jactorio::core
