// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "core/float_math.h"

namespace core
{
	TEST(FloatMath, FloatEq) {
		// Checks floats are equal within a tolerance

		EXPECT_FALSE(jactorio::core::FloatEq(2.f, 2.1f, 0.00005));
		EXPECT_FALSE(jactorio::core::FloatEq(5.f, 2.1f, 0.00005));
		// EXPECT_FALSE(jactorio::core::f_eq(2.00001f, 2.00000f));
		EXPECT_FALSE(jactorio::core::FloatEq(2.0001f, 2.0000f, 0.00005));

		EXPECT_TRUE(jactorio::core::FloatEq(2.0001f, 2.0001f, 0.00005));
		EXPECT_TRUE(jactorio::core::FloatEq(2.00000001f, 2.00000000f, 0.00005));
		EXPECT_TRUE(jactorio::core::FloatEq(2.00000001f, 1.1f + 0.9f, 0.00005));
	}

	TEST(FloatMath, Trigonometric) {
		EXPECT_FLOAT_EQ(jactorio::core::Sin(8.5), 0.147809411);
		EXPECT_FLOAT_EQ(jactorio::core::Cos(8.5), 0.989015863);
		EXPECT_FLOAT_EQ(jactorio::core::Tan(8.5), 0.149451001);
	}

	TEST(FloatMath, TrigonometricFast) {
		EXPECT_FLOAT_EQ(jactorio::core::SinF(90), 1);
		EXPECT_FLOAT_EQ(jactorio::core::SinF(45), 0.707106781);

		EXPECT_FLOAT_EQ(jactorio::core::CosF(87), 0.052335956);
		EXPECT_FLOAT_EQ(jactorio::core::CosF(40), 0.766044443);

		EXPECT_FLOAT_EQ(jactorio::core::TanF(71), 2.904210878);
		EXPECT_FLOAT_EQ(jactorio::core::TanF(43), 0.932515086);
	}
}
