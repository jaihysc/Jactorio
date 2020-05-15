// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <gtest/gtest.h>

#include "core/filesystem.h"

int main(int ac, char* av[]) {
	jactorio::core::SetExecutingDirectory(av[0]);


	testing::InitGoogleTest(&ac, av);
	RUN_ALL_TESTS();
	return 0;  // Always return 0 even if tests fail, as a non-zero return code is seen as abnormal test suite exit
}
