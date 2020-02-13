#include <gtest/gtest.h>

#include "core/float_math.h"

namespace core
{
	TEST(float_math, f_eq) {
		// Checks floats are equal within a tolerance
		
		EXPECT_FALSE(jactorio::core::f_eq(2.f, 2.1f));
		EXPECT_FALSE(jactorio::core::f_eq(5.f, 2.1f));
		// EXPECT_FALSE(jactorio::core::f_eq(2.00001f, 2.00000f));
		EXPECT_FALSE(jactorio::core::f_eq(2.0001f, 2.0000f));

		EXPECT_TRUE(jactorio::core::f_eq(2.0001f, 2.0001f));
		EXPECT_TRUE(jactorio::core::f_eq(2.00000001f, 2.00000000f));
		EXPECT_TRUE(jactorio::core::f_eq(2.00000001f, 1.1f + 0.9f));
	}
}