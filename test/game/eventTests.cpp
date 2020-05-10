// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/31/2019

#include <gtest/gtest.h>

#include "game/event/event.h"

namespace game
{
	namespace
	{
		int counter  = 0;
		int counter2 = 0;

		void ResetCounter() {
			counter  = 0;
			counter2 = 0;
		}


		void TestCallback1(jactorio::game::LogicTickEvent& e) {
			counter = e.gameTick;
		}

		void TestCallback2(int, int) {
			counter2++;
		}
	}

	class EventTest : public testing::Test
	{
	protected:
		jactorio::game::EventData eventData_;

		void TearDown() override {
			ResetCounter();
		}
	};

	TEST_F(EventTest, subscribe_raise_event) {
		eventData_.Subscribe(jactorio::game::EventType::logic_tick, TestCallback1);
		// Event::subscribe(event_type::game_gui_character_open, test_callback2);

		eventData_.Raise<jactorio::game::LogicTickEvent>(jactorio::game::EventType::logic_tick, 12);
		EXPECT_EQ(counter, 12);
	}

	TEST_F(EventTest, subscribe_once) {
		// After handling, it will not run again
		eventData_.SubscribeOnce(jactorio::game::EventType::logic_tick, TestCallback1);

		eventData_.Raise<jactorio::game::LogicTickEvent>(jactorio::game::EventType::logic_tick, 12);
		EXPECT_EQ(counter, 12);

		// This will no longer run since it has been handled once above
		eventData_.Raise<jactorio::game::LogicTickEvent>(jactorio::game::EventType::logic_tick, 22);
		EXPECT_EQ(counter, 12);  // Keeps origin val above
	}

	// TEST_F(EventTest, subscribe_raise_event_imgui_bock) {
	// 	// Imgui sets the bool property input_captured
	// 	// This takes priority over all events, and if true no events are allowed to be emitted
	// 	using namespace jactorio::game;
	//
	// 	reset_counter();
	// 	Event::clear_all_data();
	//
	// 	Event::subscribe(event_type::logic_tick, test_callback1);
	// 	// Event::subscribe(event_type::game_gui_character_open, test_callback2);
	//
	// 	// Event blocked
	// 	jactorio::renderer::imgui_manager::input_captured = true;
	// 	Event::raise<Logic_tick_event>(event_type::logic_tick, 12);
	// 	EXPECT_EQ(counter, 0);
	//
	// 	// Unblocked
	// 	jactorio::renderer::imgui_manager::input_captured = false;
	// 	Event::raise<Logic_tick_event>(event_type::logic_tick, 12);
	// 	EXPECT_EQ(counter, 12);
	// }

	TEST_F(EventTest, unsubscribe_event) {
		eventData_.Subscribe(jactorio::game::EventType::game_chunk_generated, TestCallback1);

		EXPECT_EQ(eventData_.Unsubscribe(jactorio::game::EventType::game_chunk_generated, TestCallback1), true);
		EXPECT_EQ(eventData_.Unsubscribe(jactorio::game::EventType::game_chunk_generated, TestCallback2),
		          false);  // Does not exist


		// One time
		eventData_.SubscribeOnce(jactorio::game::EventType::game_chunk_generated, TestCallback1);
		EXPECT_EQ(eventData_.Unsubscribe(jactorio::game::EventType::game_chunk_generated, TestCallback1), true);
		EXPECT_EQ(eventData_.Unsubscribe(jactorio::game::EventType::game_chunk_generated, TestCallback2),
		          false);  // Does not exist

		// Unchanged since unsubscribed
		eventData_.Raise<jactorio::game::LogicTickEvent>(jactorio::game::EventType::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}

	TEST_F(EventTest, clear_all_data) {
		eventData_.Subscribe(jactorio::game::EventType::game_chunk_generated, TestCallback1);
		eventData_.ClearAllData();

		// Nothing gets raises since it is cleared
		eventData_.Raise<jactorio::game::LogicTickEvent>(jactorio::game::EventType::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}
}
