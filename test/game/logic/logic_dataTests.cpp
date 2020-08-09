// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/prototype_base.h"
#include "game/logic/logic_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	class LogicDataTest : public testing::Test
	{
	protected:
		LogicData logicData_{};
	};

	TEST_F(LogicDataTest, OnTickAdvance) {
		// Should move the game_tick forward
		EXPECT_EQ(logicData_.GameTick(), 0);

		logicData_.GameTickAdvance();
		EXPECT_EQ(logicData_.GameTick(), 1);

		logicData_.GameTickAdvance();
		EXPECT_EQ(logicData_.GameTick(), 2);

		logicData_.GameTickAdvance();
		EXPECT_EQ(logicData_.GameTick(), 3);
	}

	// ======================================================================

	class DeferralTimerTest : public testing::Test
	{
	protected:
		LogicData logicData_{};
		WorldData worldData_{};

		LogicData::DeferralTimer& timer_ = logicData_.deferralTimer;

		class MockDeferred final : public data::IDeferred
		{
		public:
			mutable bool callbackCalled              = false;
			mutable data::UniqueDataBase* dataPtr    = nullptr;
			mutable LogicData::DeferralTimer* dTimer = nullptr;

			void OnDeferTimeElapsed(WorldData&, LogicData& logic_data,
			                        data::UniqueDataBase* unique_data) const override {
				callbackCalled = true;
				dataPtr        = unique_data;
				dTimer         = &logic_data.deferralTimer;
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
		EXPECT_TRUE(index.Valid());

		timer_.DeferralUpdate(worldData_, 0);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(worldData_, 1);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(worldData_, 2);
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
		EXPECT_TRUE(index.Valid());

		timer_.DeferralUpdate(worldData_, 0);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(worldData_, 1);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(worldData_, 2);
		EXPECT_TRUE(deferred_.callbackCalled);
		EXPECT_EQ(deferred_.dataPtr, unique_data.get());
		EXPECT_EQ(deferred_.dTimer, &timer_);
	}

	TEST_F(DeferralTimerTest, RegisterDeferralRemoveOldCallbacks) {
		timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.DeferralUpdate(worldData_, 2);
		ASSERT_TRUE(deferred_.callbackCalled);

		// Callback at 2 has been removed since it update was called for game tick 2
		EXPECT_TRUE(timer_.GetDebugInfo().callbacks.empty());
	}

	TEST_F(DeferralTimerTest, RemoveDeferral) {
		const auto entry = timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.RemoveDeferral(entry);

		// Callback removed
		timer_.DeferralUpdate(worldData_, 2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferralMultiple) {
		const auto deferral_entry   = timer_.RegisterAtTick(deferred_, nullptr, 2);
		const auto deferral_entry_2 = timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.RemoveDeferral(deferral_entry);
		timer_.RemoveDeferral(deferral_entry_2);

		// Both deferrals removed
		timer_.DeferralUpdate(worldData_, 2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferralNonExistent) {
		timer_.RemoveDeferral({32, 999});
	}

	TEST_F(DeferralTimerTest, RemoveDeferralEntry) {
		{
			const MockDeferred deferred{};
			auto entry = timer_.RegisterAtTick(deferred, nullptr, 1);

			timer_.RemoveDeferralEntry(entry);

			EXPECT_EQ(entry.second, 0);
			EXPECT_FALSE(entry.Valid());

			timer_.DeferralUpdate(worldData_, 1);
			EXPECT_FALSE(deferred.callbackCalled);
		}
		{
			LogicData::DeferralTimer::DeferralEntry entry{12, 0};  // Invalid entry since .second is 0
			timer_.RemoveDeferralEntry(entry);
		}
	}
}
