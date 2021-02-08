// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/math.h"

namespace jactorio
{
    TEST(Math, FloatEq) {
        // Checks floats are equal within a tolerance

        EXPECT_FALSE(FloatEq(2.f, 2.1f, 0.00005f));
        EXPECT_FALSE(FloatEq(5.f, 2.1f, 0.00005f));
        EXPECT_FALSE(FloatEq(2.0001f, 2.0000f, 0.00005f));

        EXPECT_TRUE(FloatEq(2.0001f, 2.0001f, 0.00005f));
        EXPECT_TRUE(FloatEq(2.00000001f, 2.00000000f, 0.00005f));
        EXPECT_TRUE(FloatEq(2.00000001f, 1.1f + 0.9f, 0.00005f));
    }

    TEST(Math, Trigonometric) {
        EXPECT_DOUBLE_EQ(Sin(8.5), 0.1478094111296106183451);
        EXPECT_DOUBLE_EQ(Cos(8.5), 0.9890158633619168298104);
        EXPECT_DOUBLE_EQ(Tan(8.5), 0.149451001349127790203);
    }

    TEST(Math, TrigonometricFast) {
        EXPECT_DOUBLE_EQ(SinF(90), 1);
        EXPECT_DOUBLE_EQ(SinF(45), 0.7071067811865475244008);

        EXPECT_DOUBLE_EQ(CosF(87), 0.0523359562429438);
        EXPECT_DOUBLE_EQ(CosF(40), 0.7660444431189780352024);

        EXPECT_DOUBLE_EQ(TanF(71), 2.90421087767582);
        EXPECT_DOUBLE_EQ(TanF(43), 0.9325150861376617056122);
    }
} // namespace jactorio
