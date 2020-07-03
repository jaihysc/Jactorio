// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 07/03/2020

#include <gtest/gtest.h>

#include "core/utility.h"

namespace jactorio::core
{
	TEST(Utility, StrToLower) {
		EXPECT_EQ(StrToLower("ASDADxcxvcASDJAKDJ:AL"), "asdadxcxvcasdjakdj:al");
	}

	TEST(Utility, StrToUpper) {
		EXPECT_EQ(StrToUpper("ASDADxcxvcASDJAKDJ:AL"), "ASDADXCXVCASDJAKDJ:AL");
	}
}
