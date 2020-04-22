// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "core/data_type.h"

namespace core
{
	class QuadPositionTest : public testing::Test
	{
	protected:
		jactorio::core::Quad_position q_1_{{3, 3}, {3, 3}};
		jactorio::core::Quad_position q_2_{{3, 2}, {1, 0}};
	};


	TEST_F(QuadPositionTest, QuadPositionAddition) {
		const auto q_3 = q_1_ + q_2_;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 6);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 5);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 4);
		EXPECT_FLOAT_EQ(q_3.bottom_right.y, 3);
	}

	TEST_F(QuadPositionTest, QuadPositionAdditionAssignment) {
		q_1_ += q_2_;

		EXPECT_FLOAT_EQ(q_1_.top_left.x, 6);
		EXPECT_FLOAT_EQ(q_1_.top_left.y, 5);
		EXPECT_FLOAT_EQ(q_1_.bottom_right.x, 4);
		EXPECT_FLOAT_EQ(q_1_.bottom_right.y, 3);
	}

	TEST_F(QuadPositionTest, QuadPositionSubtraction) {
		const auto q_3 = q_1_ - q_2_;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 0);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 1);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 2);
		EXPECT_FLOAT_EQ(q_3.bottom_right.y, 3);
	}

	TEST_F(QuadPositionTest, QuadPositionSubtractionAssignment) {
		q_1_ -= q_2_;

		EXPECT_FLOAT_EQ(q_1_.top_left.x, 0);
		EXPECT_FLOAT_EQ(q_1_.top_left.y, 1);
		EXPECT_FLOAT_EQ(q_1_.bottom_right.x, 2);
		EXPECT_FLOAT_EQ(q_1_.bottom_right.y, 3);
	}


	TEST_F(QuadPositionTest, QuadPositionMultiplication) {
		const auto q_3 = q_1_ * q_2_;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 9);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 6);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 3);
		EXPECT_FLOAT_EQ(q_3.bottom_right.y, 0);
	}

	TEST_F(QuadPositionTest, QuadPositionMultiplicationAssignment) {
		q_1_ *= q_2_;

		EXPECT_FLOAT_EQ(q_1_.top_left.x, 9);
		EXPECT_FLOAT_EQ(q_1_.top_left.y, 6);
		EXPECT_FLOAT_EQ(q_1_.bottom_right.x, 3);
		EXPECT_FLOAT_EQ(q_1_.bottom_right.y, 0);
	}


	TEST_F(QuadPositionTest, QuadPositionDivision) {
		const auto q_3 = q_1_ / q_2_;

		EXPECT_FLOAT_EQ(q_3.top_left.x, 1);
		EXPECT_FLOAT_EQ(q_3.top_left.y, 1.5);
		EXPECT_FLOAT_EQ(q_3.bottom_right.x, 3);
		// EXPECT_TRUE(isnan(q_3.bottom_right.y));
	}

	TEST_F(QuadPositionTest, QuadPositionDivisionAssignment) {
		q_1_ /= q_2_;

		EXPECT_FLOAT_EQ(q_1_.top_left.x, 1);
		EXPECT_FLOAT_EQ(q_1_.top_left.y, 1.5);
		EXPECT_FLOAT_EQ(q_1_.bottom_right.x, 3);
		// EXPECT_TRUE(isnan(q_1.bottom_right.y));
	}
}
