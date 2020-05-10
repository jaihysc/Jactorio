// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/01/2019

#include <gtest/gtest.h>

#include "core/resource_guard.h"

namespace core
{
	namespace
	{
		bool non_capturing_called = false;
	}

	TEST(ResourceGuardTest, CallDestructor) {
		{
			auto guard = jactorio::core::ResourceGuard<void>([]() {
				non_capturing_called = true;
			});
		}

		EXPECT_TRUE(non_capturing_called);
	}

	TEST(ResourceGuardTest, CapturingCallDestructor) {
		bool called = false;
		{
			auto guard = jactorio::core::CapturingGuard<void()>([&] {
				called = true;
			});
		}

		EXPECT_TRUE(called);
	}
}
