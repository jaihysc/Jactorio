// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#include <gtest/gtest.h>

#include "game/world/deferral_timer.h"

#include <memory>

namespace game
{
	class Mock_deferred final : public jactorio::data::Deferred
	{
	public:
		mutable bool callback_called = false;
		mutable jactorio::data::Unique_data_base* data_ptr = nullptr;
		mutable jactorio::game::Deferral_timer* d_timer = nullptr;

		void on_defer_time_elapsed(jactorio::game::Deferral_timer& timer,
		                           jactorio::data::Unique_data_base* unique_data) const override {
			callback_called = true;
			data_ptr = unique_data;
			d_timer = &timer;
		};
	};

	class Mock_unique_data final : public jactorio::data::Unique_data_base
	{
	};


	TEST(deferral_timer, register_at_tick) {
		jactorio::game::Deferral_timer timer{};

		const Mock_deferred deferred{};
		const auto unique_data = std::make_unique<Mock_unique_data>();

		const auto index = timer.register_at_tick(deferred, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer.deferral_update(0);
		EXPECT_FALSE(deferred.callback_called);

		timer.deferral_update(1);
		EXPECT_FALSE(deferred.callback_called);

		timer.deferral_update(2);
		EXPECT_TRUE(deferred.callback_called);
		EXPECT_EQ(deferred.data_ptr, unique_data.get());
		EXPECT_EQ(deferred.d_timer, &timer);
	}

	TEST(deferral_timer, register_from_tick) {
		jactorio::game::Deferral_timer timer{};

		const Mock_deferred deferred{};
		const auto unique_data = std::make_unique<Mock_unique_data>();

		// Elapse 2 ticks from now
		const auto index = timer.register_from_tick(deferred, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer.deferral_update(0);
		EXPECT_FALSE(deferred.callback_called);

		timer.deferral_update(1);
		EXPECT_FALSE(deferred.callback_called);

		timer.deferral_update(2);
		EXPECT_TRUE(deferred.callback_called);
		EXPECT_EQ(deferred.data_ptr, unique_data.get());
		EXPECT_EQ(deferred.d_timer, &timer);
	}

	TEST(deferral_timer, register_deferral_remove_old_callbacks) {
		jactorio::game::Deferral_timer timer{};

		Mock_deferred deferred{};
		timer.register_at_tick(deferred, nullptr, 2);

		timer.deferral_update(2);
		ASSERT_TRUE(deferred.callback_called);

		// Callback at 2 has been removed since it update was called for game tick 2
		deferred.callback_called = false;  // Reset
		timer.deferral_update(2);
		EXPECT_FALSE(deferred.callback_called);
	}

	TEST(deferral_timer, remove_deferral) {
		jactorio::game::Deferral_timer timer{};

		const Mock_deferred deferred{};
		const auto index = timer.register_at_tick(deferred, nullptr, 2);

		timer.remove_deferral(index);

		// Callback removed
		timer.deferral_update(2);
		ASSERT_FALSE(deferred.callback_called);
	}

	TEST(deferral_timer, remove_deferral_non_existent) {
		jactorio::game::Deferral_timer timer{};

		timer.remove_deferral({32, 999});
	}
}
