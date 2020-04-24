// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#include <gtest/gtest.h>

#include "game/world/deferral_timer.h"

#include <memory>

namespace game
{
	class DeferralTimerTest : public testing::Test
	{
	protected:
		jactorio::game::Deferral_timer timer_{};

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
	};

	TEST_F(DeferralTimerTest, register_at_tick) {
		const Mock_deferred deferred{};
		const auto unique_data = std::make_unique<Mock_unique_data>();

		const auto index = timer_.register_at_tick(deferred, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.deferral_update(0);
		EXPECT_FALSE(deferred.callback_called);

		timer_.deferral_update(1);
		EXPECT_FALSE(deferred.callback_called);

		timer_.deferral_update(2);
		EXPECT_TRUE(deferred.callback_called);
		EXPECT_EQ(deferred.data_ptr, unique_data.get());
		EXPECT_EQ(deferred.d_timer, &timer_);
	}

	TEST_F(DeferralTimerTest, register_from_tick) {
		const Mock_deferred deferred{};
		const auto unique_data = std::make_unique<Mock_unique_data>();

		// Elapse 2 ticks from now
		const auto index = timer_.register_from_tick(deferred, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.deferral_update(0);
		EXPECT_FALSE(deferred.callback_called);

		timer_.deferral_update(1);
		EXPECT_FALSE(deferred.callback_called);

		timer_.deferral_update(2);
		EXPECT_TRUE(deferred.callback_called);
		EXPECT_EQ(deferred.data_ptr, unique_data.get());
		EXPECT_EQ(deferred.d_timer, &timer_);
	}

	TEST_F(DeferralTimerTest, register_deferral_remove_old_callbacks) {
		Mock_deferred deferred{};
		timer_.register_at_tick(deferred, nullptr, 2);

		timer_.deferral_update(2);
		ASSERT_TRUE(deferred.callback_called);

		// Callback at 2 has been removed since it update was called for game tick 2
		deferred.callback_called = false;  // Reset
		timer_.deferral_update(2);
		EXPECT_FALSE(deferred.callback_called);
	}

	TEST_F(DeferralTimerTest, remove_deferral) {
		const Mock_deferred deferred{};
		const auto index = timer_.register_at_tick(deferred, nullptr, 2);

		timer_.remove_deferral(index);

		// Callback removed
		timer_.deferral_update(2);
		EXPECT_FALSE(deferred.callback_called);
	}

	TEST_F(DeferralTimerTest, remove_deferral_multiple) {
		const Mock_deferred deferred{};

		const auto deferral_entry = timer_.register_at_tick(deferred, nullptr, 2);
		const auto deferral_entry_2 = timer_.register_at_tick(deferred, nullptr, 2);

		timer_.remove_deferral(deferral_entry);
		timer_.remove_deferral(deferral_entry_2);

		// Both deferrals removed
		timer_.deferral_update(2);
		EXPECT_FALSE(deferred.callback_called);
	}

	TEST_F(DeferralTimerTest, remove_deferral_non_existent) {
		timer_.remove_deferral({32, 999});
	}
}
