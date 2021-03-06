// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/deferral_timer.h"

#include "jactorioTests.h"

namespace jactorio::game
{
    class DeferralTimerTest : public testing::Test
    {
    protected:
        Logic logic_;
        World world_;

        DeferralTimer& timer_ = logic_.deferralTimer;

        class MockDeferred final : public TestMockEntity
        {
        public:
            mutable bool callbackCalled            = false;
            mutable proto::UniqueDataBase* dataPtr = nullptr;
            mutable DeferralTimer* dTimer          = nullptr;

            void OnDeferTimeElapsed(World& /*world*/, Logic& logic, proto::UniqueDataBase* unique_data) const override {
                callbackCalled = true;
                dataPtr        = unique_data;
                dTimer         = &logic.deferralTimer;
            };
        };

        MockDeferred deferred_;

        class MockUniqueData final : public proto::FEntityData
        {
        };
    };

    TEST_F(DeferralTimerTest, RregisterAtTick) {
        MockUniqueData unique_data;

        const auto index = timer_.RegisterAtTick(deferred_, &unique_data, 2);
        EXPECT_EQ(index.dueTick, 2);
        EXPECT_EQ(index.callbackIndex, 1);
        EXPECT_TRUE(index.Valid());

        logic_.DeferralUpdate(world_, 0);
        EXPECT_FALSE(deferred_.callbackCalled);

        logic_.DeferralUpdate(world_, 1);
        EXPECT_FALSE(deferred_.callbackCalled);

        logic_.DeferralUpdate(world_, 2);
        EXPECT_TRUE(deferred_.callbackCalled);
        EXPECT_EQ(deferred_.dataPtr, &unique_data);
        EXPECT_EQ(deferred_.dTimer, &timer_);
    }

    TEST_F(DeferralTimerTest, RegisterFromTick) {
        MockUniqueData unique_data;

        // Elapse 2 ticks from now
        const auto index = timer_.RegisterFromTick(deferred_, &unique_data, 2);
        EXPECT_EQ(index.dueTick, 2);
        EXPECT_EQ(index.callbackIndex, 1);
        EXPECT_TRUE(index.Valid());

        logic_.DeferralUpdate(world_, 0);
        EXPECT_FALSE(deferred_.callbackCalled);

        logic_.DeferralUpdate(world_, 1);
        EXPECT_FALSE(deferred_.callbackCalled);

        logic_.DeferralUpdate(world_, 2);
        EXPECT_TRUE(deferred_.callbackCalled);
        EXPECT_EQ(deferred_.dataPtr, &unique_data);
        EXPECT_EQ(deferred_.dTimer, &timer_);
    }

    TEST_F(DeferralTimerTest, RegisterDeferralRemoveOldCallbacks) {
        timer_.RegisterAtTick(deferred_, nullptr, 2);

        logic_.DeferralUpdate(world_, 2);
        ASSERT_TRUE(deferred_.callbackCalled);

        // Callback at 2 has been removed since it update was called for game tick 2
        EXPECT_TRUE(timer_.GetDebugInfo().callbacks.empty());
    }

    TEST_F(DeferralTimerTest, RemoveDeferral) {
        const auto entry = timer_.RegisterAtTick(deferred_, nullptr, 2);

        timer_.RemoveDeferral(entry);

        // Callback removed
        logic_.DeferralUpdate(world_, 2);
        EXPECT_FALSE(deferred_.callbackCalled);
    }

    TEST_F(DeferralTimerTest, RemoveDeferralMultiple) {
        const auto deferral_entry   = timer_.RegisterAtTick(deferred_, nullptr, 2);
        const auto deferral_entry_2 = timer_.RegisterAtTick(deferred_, nullptr, 2);

        timer_.RemoveDeferral(deferral_entry);
        timer_.RemoveDeferral(deferral_entry_2);

        // Both deferrals removed
        logic_.DeferralUpdate(world_, 2);
        EXPECT_FALSE(deferred_.callbackCalled);
    }

    TEST_F(DeferralTimerTest, RemoveDeferralNonExistent) {
        timer_.RemoveDeferral({32, 999});
    }

    TEST_F(DeferralTimerTest, RemoveDeferralEntry) {
        {
            const MockDeferred deferred;
            auto entry = timer_.RegisterAtTick(deferred, nullptr, 1);

            timer_.RemoveDeferralEntry(entry);

            EXPECT_EQ(entry.callbackIndex, 0);
            EXPECT_FALSE(entry.Valid());

            logic_.DeferralUpdate(world_, 1);
            EXPECT_FALSE(deferred.callbackCalled);
        }
        {
            DeferralTimer::DeferralEntry entry{12, 0}; // Invalid entry since .second is 0
            timer_.RemoveDeferralEntry(entry);
        }
    }

    TEST_F(DeferralTimerTest, SerializeDeferralEntry) {
        const DeferralTimer::DeferralEntry entry{32, 123};

        const auto result = TestSerializeDeserialize(entry);
        EXPECT_EQ(result.dueTick, 32);
        EXPECT_EQ(result.callbackIndex, 123);
    }

    TEST_F(DeferralTimerTest, SerializeCallbacks) {
        data::PrototypeManager proto;
        data::UniqueDataManager unique;

        auto& defer_proto = proto.Make<MockDeferred>();
        MockUniqueData unique_data;

        timer_.RegisterAtTick(defer_proto, &unique_data, 10);


        proto.GenerateRelocationTable();
        unique.AssignId(unique_data);
        unique.StoreRelocationEntry(unique_data);

        data::active_prototype_manager   = &proto;
        data::active_unique_data_manager = &unique;
        const auto result                = TestSerializeDeserialize(timer_);


        const auto info = result.GetDebugInfo();
        ASSERT_EQ(info.callbacks.size(), 1);
        ASSERT_EQ(info.callbacks.at(10).size(), 1);

        const auto& entry = info.callbacks.at(10)[0];
        EXPECT_EQ(entry.prototype, &defer_proto);
        EXPECT_EQ(entry.uniqueData, &unique_data);
    }
} // namespace jactorio::game
