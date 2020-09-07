// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/cereal/support/decimal.h"

#include "jactorioTests.h"

namespace jactorio::data
{
    TEST(CerealSupport, DecDecimal) {
        constexpr auto original_val = 6345532.536;

        const Decimal3T val(original_val);

        const auto result = TestSerializeDeserialize(val);

        EXPECT_DOUBLE_EQ(result.getAsDouble(), original_val);
    }
} // namespace jactorio::data
