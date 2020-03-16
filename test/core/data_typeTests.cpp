// 
// data_typeTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/16/2020
// Last modified: 03/16/2020
// 

#include <gtest/gtest.h>

#include "core/data_type.h"

namespace core
{
	TEST(data_type, quad_position_addition) {
		const jactorio::core::Quad_position q_1{{0, 1}, {2, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		const auto q_3 = q_1 + q_2;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 3);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 3);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 3);
		EXPECT_FLOAT_EQ(q_3.bottom_right.y, 3);
	}

	TEST(data_type, quad_position_addition_assignment) {
		jactorio::core::Quad_position q_1{{0, 1}, {2, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		q_1 += q_2;

		EXPECT_FLOAT_EQ(q_1.top_left.x, 3);
		EXPECT_FLOAT_EQ(q_1.top_left.y, 3);
		EXPECT_FLOAT_EQ(q_1.bottom_right.x, 3);
		EXPECT_FLOAT_EQ(q_1.bottom_right.y, 3);
	}

	TEST(data_type, quad_position_subtraction) {
		const jactorio::core::Quad_position q_1{{3, 3}, {3, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		const auto q_3 = q_1 - q_2;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 0);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 1);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 2);
		EXPECT_FLOAT_EQ(q_3.bottom_right.y, 3);
	}

	TEST(data_type, quad_position_subtraction_assignment) {
		jactorio::core::Quad_position q_1{{3, 3}, {3, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		q_1 -= q_2;

		EXPECT_FLOAT_EQ(q_1.top_left.x, 0);
		EXPECT_FLOAT_EQ(q_1.top_left.y, 1);
		EXPECT_FLOAT_EQ(q_1.bottom_right.x, 2);
		EXPECT_FLOAT_EQ(q_1.bottom_right.y, 3);
	}


	TEST(data_type, quad_position_multiplication) {
		const jactorio::core::Quad_position q_1{{3, 3}, {3, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		const auto q_3 = q_1 * q_2;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 9);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 6);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 3);
		EXPECT_FLOAT_EQ(q_3.bottom_right.y, 0);
	}

	TEST(data_type, quad_position_multiplication_assignment) {
		jactorio::core::Quad_position q_1{{3, 3}, {3, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		q_1 *= q_2;

		EXPECT_FLOAT_EQ(q_1.top_left.x, 9);
		EXPECT_FLOAT_EQ(q_1.top_left.y, 6);
		EXPECT_FLOAT_EQ(q_1.bottom_right.x, 3);
		EXPECT_FLOAT_EQ(q_1.bottom_right.y, 0);
	}


	TEST(data_type, quad_position_division) {
		const jactorio::core::Quad_position q_1{{3, 3}, {3, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		const auto q_3 = q_1 / q_2;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 1);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 1.5);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 3);
		// EXPECT_TRUE(isnan(q_3.bottom_right.y));
	}

	TEST(data_type, quad_position_division_assignment) {
		jactorio::core::Quad_position q_1{{3, 3}, {3, 3}};
		const jactorio::core::Quad_position q_2{{3, 2}, {1, 0}};

		q_1 /= q_2;

		EXPECT_FLOAT_EQ(q_1.top_left.x, 1);
		EXPECT_FLOAT_EQ(q_1.top_left.y, 1.5);
		EXPECT_FLOAT_EQ(q_1.bottom_right.x, 3);
		// EXPECT_TRUE(isnan(q_1.bottom_right.y));
	}
}
