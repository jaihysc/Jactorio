// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "core/data_type.h"

namespace core
{
	class QuadPositionTest : public testing::Test
	{
	protected:
		jactorio::core::QuadPosition q1_{{3, 3}, {3, 3}};
		jactorio::core::QuadPosition q2_{{3, 2}, {1, 0}};
	};


	TEST_F(QuadPositionTest, QuadPositionAddition) {
		const auto q_3 = q1_ + q2_;

		EXPECT_FLOAT_EQ(q_3.topLeft.x, 6);
		EXPECT_FLOAT_EQ(q_3.topLeft.y, 5);
		EXPECT_FLOAT_EQ(q_3.bottomRight.x, 4);
		EXPECT_FLOAT_EQ(q_3.bottomRight.y, 3);
	}

	TEST_F(QuadPositionTest, QuadPositionAdditionAssignment) {
		q1_ += q2_;

		EXPECT_FLOAT_EQ(q1_.topLeft.x, 6);
		EXPECT_FLOAT_EQ(q1_.topLeft.y, 5);
		EXPECT_FLOAT_EQ(q1_.bottomRight.x, 4);
		EXPECT_FLOAT_EQ(q1_.bottomRight.y, 3);
	}

	TEST_F(QuadPositionTest, QuadPositionSubtraction) {
		const auto q_3 = q1_ - q2_;

		EXPECT_FLOAT_EQ(q_3.topLeft.x, 0);
		EXPECT_FLOAT_EQ(q_3.topLeft.y, 1);
		EXPECT_FLOAT_EQ(q_3.bottomRight.x, 2);
		EXPECT_FLOAT_EQ(q_3.bottomRight.y, 3);
	}

	TEST_F(QuadPositionTest, QuadPositionSubtractionAssignment) {
		q1_ -= q2_;

		EXPECT_FLOAT_EQ(q1_.topLeft.x, 0);
		EXPECT_FLOAT_EQ(q1_.topLeft.y, 1);
		EXPECT_FLOAT_EQ(q1_.bottomRight.x, 2);
		EXPECT_FLOAT_EQ(q1_.bottomRight.y, 3);
	}


	TEST_F(QuadPositionTest, QuadPositionMultiplication) {
		const auto q_3 = q1_ * q2_;

		EXPECT_FLOAT_EQ(q_3.topLeft.x, 9);
		EXPECT_FLOAT_EQ(q_3.topLeft.y, 6);
		EXPECT_FLOAT_EQ(q_3.bottomRight.x, 3);
		EXPECT_FLOAT_EQ(q_3.bottomRight.y, 0);
	}

	TEST_F(QuadPositionTest, QuadPositionMultiplicationAssignment) {
		q1_ *= q2_;

		EXPECT_FLOAT_EQ(q1_.topLeft.x, 9);
		EXPECT_FLOAT_EQ(q1_.topLeft.y, 6);
		EXPECT_FLOAT_EQ(q1_.bottomRight.x, 3);
		EXPECT_FLOAT_EQ(q1_.bottomRight.y, 0);
	}


	TEST_F(QuadPositionTest, QuadPositionDivision) {
		const auto q_3 = q1_ / q2_;

		EXPECT_FLOAT_EQ(q_3.topLeft.x, 1);
		EXPECT_FLOAT_EQ(q_3.topLeft.y, 1.5);
		EXPECT_FLOAT_EQ(q_3.bottomRight.x, 3);
		// EXPECT_TRUE(isnan(q_3.bottom_right.y));
	}

	TEST_F(QuadPositionTest, QuadPositionDivisionAssignment) {
		q1_ /= q2_;

		EXPECT_FLOAT_EQ(q1_.topLeft.x, 1);
		EXPECT_FLOAT_EQ(q1_.topLeft.y, 1.5);
		EXPECT_FLOAT_EQ(q1_.bottomRight.x, 3);
		// EXPECT_TRUE(isnan(q_1.bottom_right.y));
	}
}
