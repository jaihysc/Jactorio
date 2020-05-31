// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#include <gtest/gtest.h>

#include <memory>

#include "data/prototype/prototype_base.h"
#include "game/world/deferral_timer.h"

namespace jactorio::game
{
	class DeferralTimerTest : public testing::Test
	{
	protected:
		DeferralTimer timer_{};

		class MockDeferred final : public data::Deferred
		{
		public:
			mutable bool callbackCalled           = false;
			mutable data::UniqueDataBase* dataPtr = nullptr;
			mutable DeferralTimer* dTimer         = nullptr;

			void OnDeferTimeElapsed(DeferralTimer& timer,
			                        data::UniqueDataBase* unique_data) const override {
				callbackCalled = true;
				dataPtr        = unique_data;
				dTimer         = &timer;
			};
		};

		const MockDeferred deferred_{};

		class MockUniqueData final : public data::UniqueDataBase
		{
		};
	};

	TEST_F(DeferralTimerTest, RregisterAtTick) {
		const auto unique_data = std::make_unique<MockUniqueData>();

		const auto index = timer_.RegisterAtTick(deferred_, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.DeferralUpdate(0);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(1);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(2);
		EXPECT_TRUE(deferred_.callbackCalled);
		EXPECT_EQ(deferred_.dataPtr, unique_data.get());
		EXPECT_EQ(deferred_.dTimer, &timer_);
	}

	TEST_F(DeferralTimerTest, RegisterFromTick) {
		const auto unique_data = std::make_unique<MockUniqueData>();

		// Elapse 2 ticks from now
		const auto index = timer_.RegisterFromTick(deferred_, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.DeferralUpdate(0);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(1);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(2);
		EXPECT_TRUE(deferred_.callbackCalled);
		EXPECT_EQ(deferred_.dataPtr, unique_data.get());
		EXPECT_EQ(deferred_.dTimer, &timer_);
	}

	TEST_F(DeferralTimerTest, RegisterDeferralRemoveOldCallbacks) {
		timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.DeferralUpdate(2);
		ASSERT_TRUE(deferred_.callbackCalled);

		// Callback at 2 has been removed since it update was called for game tick 2
		deferred_.callbackCalled = false;  // Reset
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferral) {
		const auto entry = timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.RemoveDeferral(entry);

		// Callback removed
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferralMultiple) {
		const auto deferral_entry   = timer_.RegisterAtTick(deferred_, nullptr, 2);
		const auto deferral_entry_2 = timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.RemoveDeferral(deferral_entry);
		timer_.RemoveDeferral(deferral_entry_2);

		// Both deferrals removed
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferralNonExistent) {
		timer_.RemoveDeferral({32, 999});
	}

	TEST_F(DeferralTimerTest, RemoveDeferralEntry) {
		{
			MockDeferred deferred{};
			auto entry = timer_.RegisterAtTick(deferred, nullptr, 1);
			
			timer_.RemoveDeferralEntry(entry);

			EXPECT_EQ(entry.second, 0);

			timer_.DeferralUpdate(1);
			EXPECT_FALSE(deferred.callbackCalled);
		}
		{
			DeferralTimer::DeferralEntry entry{12, 0};  // Invalid entry since .second is 0
			timer_.RemoveDeferralEntry(entry);
		}
	}
}
