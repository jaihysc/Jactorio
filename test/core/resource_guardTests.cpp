#include <gtest/gtest.h>

#include "core/resource_guard.h"

namespace core
{
	namespace
	{
		int counter = 0;
		
		void func() {
			counter = 1;
		}
	}
	
	TEST(resource_guard, call_destructor) {
		counter = 0;
		{
			auto guard = jactorio::core::Resource_guard<void>(func);
		}

		EXPECT_EQ(counter, 1);
	}

	TEST(resource_guard, capturing_call_destructor) {
		counter = 0;
		{
			auto guard = jactorio::core::Capturing_guard<void()>([&] {
				func();
			});
		}

		EXPECT_EQ(counter, 1);
	}
}