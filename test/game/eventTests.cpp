// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/event/event.h"

namespace jactorio::game
{
    // Raising the event automatically increments counter
    class MockEvent final : public EventBase
    {
    public:
        explicit MockEvent(const int increment, int& counter) {
            counter += increment;
        }

        EVENT_TYPE(logic_tick)
        EVENT_CATEGORY(in_game)
    };

    class EventTest : public testing::Test
    {
    protected:
        EventData eventData_;
    };

    TEST_F(EventTest, SubscribeRaiseEvent) {
        int counter = 0;

        eventData_.Subscribe(EventType::logic_tick, [](auto& /*event*/) {});

        eventData_.Raise<MockEvent>(EventType::logic_tick, 12, counter);
        EXPECT_EQ(counter, 12);

        eventData_.Raise<MockEvent>(EventType::logic_tick, 12, counter);
        EXPECT_EQ(counter, 24);
    }

    TEST_F(EventTest, SubscribeOnce) {
        // After handling, it will not run again

        int counter = 0;

        eventData_.SubscribeOnce(EventType::logic_tick, [](auto& /*event*/) {});
        eventData_.SubscribeOnce(EventType::renderer_tick, [](auto& /*event*/) {});


        eventData_.Raise<MockEvent>(EventType::logic_tick, 12, counter);
        EXPECT_EQ(counter, 12);

        // This will no longer run since it has been handled once above
        eventData_.Raise<MockEvent>(EventType::logic_tick, 12, counter);
        EXPECT_EQ(counter, 12); // Keeps origin val above


        // Render tick event unaffected
        eventData_.Raise<MockEvent>(EventType::renderer_tick, 2, counter);
        EXPECT_EQ(counter, 14);
    }

    TEST_F(EventTest, SubscribeOnceInCallback) {
        int counter = 0;

        eventData_.SubscribeOnce(EventType::logic_tick, [this](auto& /*event*/) {
            eventData_.SubscribeOnce(EventType::logic_tick, [](auto& /*event*/) {});
        });


        eventData_.Raise<MockEvent>(EventType::logic_tick, 1, counter);
        EXPECT_EQ(counter, 1);

        eventData_.Raise<MockEvent>(EventType::logic_tick, 10, counter);
        EXPECT_EQ(counter, 11);
    }

    TEST_F(EventTest, UnsubscribeEvent) {

        int counter = 0;

        auto callback = [](const EventBase& /*event*/) {};

        auto callback2 = [](const EventBase& /*event*/) {};


        eventData_.Subscribe(EventType::game_chunk_generated, callback);

        EXPECT_TRUE(eventData_.Unsubscribe(EventType::game_chunk_generated, callback));
        EXPECT_FALSE(eventData_.Unsubscribe(EventType::game_chunk_generated, callback2)); // Does not exist


        // One time
        eventData_.SubscribeOnce(EventType::game_chunk_generated, callback);
        EXPECT_TRUE(eventData_.Unsubscribe(EventType::game_chunk_generated, callback));
        EXPECT_FALSE(eventData_.Unsubscribe(EventType::game_chunk_generated, callback2)); // Does not exist

        // counter unchanged since all callbacks unsubscribed
        eventData_.Raise<MockEvent>(EventType::game_chunk_generated, 1, counter);
        EXPECT_EQ(counter, 0);
    }

    TEST_F(EventTest, ClearAllData) {
        int counter = 0;
        eventData_.Subscribe(EventType::game_chunk_generated, [](auto& /*event*/) {});
        eventData_.ClearAllData();

        // Nothing gets raises since it is cleared
        eventData_.Raise<MockEvent>(EventType::game_chunk_generated, 1, counter);
        EXPECT_EQ(counter, 0);
    }
} // namespace jactorio::game
