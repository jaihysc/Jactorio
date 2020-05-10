// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#include <gtest/gtest.h>

#include "data/prototype/prototype_base.h"
#include "game/world/deferral_timer.h"

#include <memory>

namespace game
{
	class DeferralTimerTest : public testing::Test
	{
	protected:
		jactorio::game::DeferralTimer timer_{};

		class MockDeferred final : public jactorio::data::Deferred
		{
		public:
			mutable bool callbackCalled                     = false;
			mutable jactorio::data::UniqueDataBase* dataPtr = nullptr;
			mutable jactorio::game::DeferralTimer* dTimer   = nullptr;

			void OnDeferTimeElapsed(jactorio::game::DeferralTimer& timer,
			                        jactorio::data::UniqueDataBase* unique_data) const override {
				callbackCalled = true;
				dataPtr        = unique_data;
				dTimer         = &timer;
			};
		};

		class MockUniqueData final : public jactorio::data::UniqueDataBase
		{
		};
	};

	TEST_F(DeferralTimerTest, register_at_tick) {
		const MockDeferred deferred{};
		const auto unique_data = std::make_unique<MockUniqueData>();

		const auto index = timer_.RegisterAtTick(deferred, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.DeferralUpdate(0);
		EXPECT_FALSE(deferred.callbackCalled);

		timer_.DeferralUpdate(1);
		EXPECT_FALSE(deferred.callbackCalled);

		timer_.DeferralUpdate(2);
		EXPECT_TRUE(deferred.callbackCalled);
		EXPECT_EQ(deferred.dataPtr, unique_data.get());
		EXPECT_EQ(deferred.dTimer, &timer_);
	}

	TEST_F(DeferralTimerTest, register_from_tick) {
		const MockDeferred deferred{};
		const auto unique_data = std::make_unique<MockUniqueData>();

		// Elapse 2 ticks from now
		const auto index = timer_.RegisterFromTick(deferred, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.DeferralUpdate(0);
		EXPECT_FALSE(deferred.callbackCalled);

		timer_.DeferralUpdate(1);
		EXPECT_FALSE(deferred.callbackCalled);

		timer_.DeferralUpdate(2);
		EXPECT_TRUE(deferred.callbackCalled);
		EXPECT_EQ(deferred.dataPtr, unique_data.get());
		EXPECT_EQ(deferred.dTimer, &timer_);
	}

	TEST_F(DeferralTimerTest, register_deferral_remove_old_callbacks) {
		MockDeferred deferred{};
		timer_.RegisterAtTick(deferred, nullptr, 2);

		timer_.DeferralUpdate(2);
		ASSERT_TRUE(deferred.callbackCalled);

		// Callback at 2 has been removed since it update was called for game tick 2
		deferred.callbackCalled = false;  // Reset
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred.callbackCalled);
	}

	TEST_F(DeferralTimerTest, remove_deferral) {
		const MockDeferred deferred{};
		const auto index = timer_.RegisterAtTick(deferred, nullptr, 2);

		timer_.RemoveDeferral(index);

		// Callback removed
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred.callbackCalled);
	}

	TEST_F(DeferralTimerTest, remove_deferral_multiple) {
		const MockDeferred deferred{};

		const auto deferral_entry   = timer_.RegisterAtTick(deferred, nullptr, 2);
		const auto deferral_entry_2 = timer_.RegisterAtTick(deferred, nullptr, 2);

		timer_.RemoveDeferral(deferral_entry);
		timer_.RemoveDeferral(deferral_entry_2);

		// Both deferrals removed
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred.callbackCalled);
	}

	TEST_F(DeferralTimerTest, remove_deferral_non_existent) {
		timer_.RemoveDeferral({32, 999});
	}
}
