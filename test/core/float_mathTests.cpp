// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "core/float_math.h"

namespace core
{
	TEST(FloatMath, FEq) {
		// Checks floats are equal within a tolerance

		EXPECT_FALSE(jactorio::core::f_eq(2.f, 2.1f, 0.00005));
		EXPECT_FALSE(jactorio::core::f_eq(5.f, 2.1f, 0.00005));
		// EXPECT_FALSE(jactorio::core::f_eq(2.00001f, 2.00000f));
		EXPECT_FALSE(jactorio::core::f_eq(2.0001f, 2.0000f, 0.00005));

		EXPECT_TRUE(jactorio::core::f_eq(2.0001f, 2.0001f, 0.00005));
		EXPECT_TRUE(jactorio::core::f_eq(2.00000001f, 2.00000000f, 0.00005));
		EXPECT_TRUE(jactorio::core::f_eq(2.00000001f, 1.1f + 0.9f, 0.00005));
	}
}
