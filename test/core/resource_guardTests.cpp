// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/resource_guard.h"

namespace jactorio::core
{
	namespace
	{
		bool non_capturing_called = false;
	}

	TEST(ResourceGuardTest, CallDestructor) {
		{
			auto guard = ResourceGuard<void>([]() {
				non_capturing_called = true;
			});
		}

		EXPECT_TRUE(non_capturing_called);
	}

	TEST(ResourceGuardTest, CapturingCallDestructor) {
		bool called = false;
		{
			auto guard = CapturingGuard<void()>([&] {
				called = true;
			});
		}

		EXPECT_TRUE(called);
	}
}
