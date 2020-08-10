// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/math.h"

namespace jactorio::core
{
	TEST(Math, Narrow) {
		auto a = SafeCast<int>(42);
		auto c = SafeCast<uint64_t>(1);  // Widening cast

		// const auto b = SafeCast<float>(static_cast<double>(321321312));  // Error: Must be integral
		// auto d = SafeCast<uint8_t>(256);  // Error: Loss of data
	}

	TEST(Math, FloatEq) {
		// Checks floats are equal within a tolerance

		EXPECT_FALSE(FloatEq(2.f, 2.1f, 0.00005));
		EXPECT_FALSE(FloatEq(5.f, 2.1f, 0.00005));
		// EXPECT_FALSE(f_eq(2.00001f, 2.00000f));
		EXPECT_FALSE(FloatEq(2.0001f, 2.0000f, 0.00005));

		EXPECT_TRUE(FloatEq(2.0001f, 2.0001f, 0.00005));
		EXPECT_TRUE(FloatEq(2.00000001f, 2.00000000f, 0.00005));
		EXPECT_TRUE(FloatEq(2.00000001f, 1.1f + 0.9f, 0.00005));
	}

	TEST(Math, Trigonometric) {
		EXPECT_FLOAT_EQ(Sin(8.5), 0.147809411);
		EXPECT_FLOAT_EQ(Cos(8.5), 0.989015863);
		EXPECT_FLOAT_EQ(Tan(8.5), 0.149451001);
	}

	TEST(Math, TrigonometricFast) {
		EXPECT_FLOAT_EQ(SinF(90), 1);
		EXPECT_FLOAT_EQ(SinF(45), 0.707106781);

		EXPECT_FLOAT_EQ(CosF(87), 0.052335956);
		EXPECT_FLOAT_EQ(CosF(40), 0.766044443);

		EXPECT_FLOAT_EQ(TanF(71), 2.904210878);
		EXPECT_FLOAT_EQ(TanF(43), 0.932515086);
	}
}
