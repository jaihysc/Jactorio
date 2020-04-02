// 
// deferral_timerTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/02/2020
// Last modified: 04/02/2020
// 

#include <gtest/gtest.h>

#include "game/world/deferral_timer.h"

#include <memory>

namespace game
{
	class Mock_deferred final : public jactorio::data::Deferred
	{
	public:
		bool callback_called = false;
		jactorio::data::Unique_data_base* data_ptr = nullptr;

		void on_defer_time_elapsed(jactorio::data::Unique_data_base* unique_data) override {
			callback_called = true;
			data_ptr = unique_data;
		};
	};

	class Mock_unique_data final : public jactorio::data::Unique_data_base
	{
	};


	TEST(deferral_timer, register_deferral) {
		jactorio::game::Deferral_timer timer{};

		Mock_deferred deferred{};
		const auto unique_data = std::make_unique<Mock_unique_data>();

		const auto index = timer.register_deferral(deferred, unique_data.get(), 2);
		EXPECT_EQ(index, 0);

		timer.deferral_update(0);
		EXPECT_FALSE(deferred.callback_called);

		timer.deferral_update(1);
		EXPECT_FALSE(deferred.callback_called);

		timer.deferral_update(2);
		EXPECT_TRUE(deferred.callback_called);
		EXPECT_EQ(deferred.data_ptr, unique_data.get());
	}

	TEST(deferral_timer, register_deferral_remove_old_callbacks) {
		jactorio::game::Deferral_timer timer{};

		Mock_deferred deferred{};
		timer.register_deferral(deferred, nullptr, 2);

		timer.deferral_update(2);
		ASSERT_TRUE(deferred.callback_called);

		// Callback at 2 has been removed since it update was called for game tick 2
		deferred.callback_called = false;  // Reset
		timer.deferral_update(2);
		EXPECT_FALSE(deferred.callback_called);
	}

	TEST(deferral_timer, remove_deferral) {
		jactorio::game::Deferral_timer timer{};

		Mock_deferred deferred{};
		const auto index = timer.register_deferral(deferred, nullptr, 2);

		timer.remove_deferral(2, index);

		// Callback removed
		timer.deferral_update(2);
		ASSERT_FALSE(deferred.callback_called);
	}

	TEST(deferral_timer, remove_deferral_non_existent) {
		jactorio::game::Deferral_timer timer{};

		timer.remove_deferral(2, 999);
	}
}
