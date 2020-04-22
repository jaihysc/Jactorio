// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/31/2019

#include <gtest/gtest.h>

#include "game/event/event.h"

namespace game
{
	namespace
	{
		int counter = 0;
		int counter2 = 0;

		void reset_counter() {
			counter = 0;
			counter2 = 0;
		}


		void test_callback1(jactorio::game::Logic_tick_event& e) {
			counter = e.game_tick;
		}

		void test_callback2(int, int) {
			counter2++;
		}
	}

	class EventTest : public testing::Test
	{
	protected:
		void TearDown() override {
			jactorio::game::Event::clear_all_data();
			reset_counter();
		}
	};

	TEST_F(EventTest, subscribe_raise_event) {
		jactorio::game::Event::subscribe(jactorio::game::event_type::logic_tick, test_callback1);
		// Event::subscribe(event_type::game_gui_character_open, test_callback2);

		jactorio::game::Event::raise<jactorio::game::Logic_tick_event>(jactorio::game::event_type::logic_tick, 12);
		EXPECT_EQ(counter, 12);
	}

	TEST_F(EventTest, subscribe_once) {
		// After handling, it will not run again
		jactorio::game::Event::subscribe_once(jactorio::game::event_type::logic_tick, test_callback1);

		jactorio::game::Event::raise<jactorio::game::Logic_tick_event>(jactorio::game::event_type::logic_tick, 12);
		EXPECT_EQ(counter, 12);

		// This will no longer run since it has been handled once above
		jactorio::game::Event::raise<jactorio::game::Logic_tick_event>(jactorio::game::event_type::logic_tick, 22);
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
		jactorio::game::Event::subscribe(jactorio::game::event_type::game_chunk_generated, test_callback1);

		EXPECT_EQ(jactorio::game::Event::unsubscribe(jactorio::game::event_type::game_chunk_generated, test_callback1), true);
		EXPECT_EQ(jactorio::game::Event::unsubscribe(jactorio::game::event_type::game_chunk_generated, test_callback2),
		          false);  // Does not exist


		// One time
		jactorio::game::Event::subscribe_once(jactorio::game::event_type::game_chunk_generated, test_callback1);
		EXPECT_EQ(jactorio::game::Event::unsubscribe(jactorio::game::event_type::game_chunk_generated, test_callback1), true);
		EXPECT_EQ(jactorio::game::Event::unsubscribe(jactorio::game::event_type::game_chunk_generated, test_callback2),
		          false);  // Does not exist

		// Unchanged since unsubscribed
		jactorio::game::Event::raise<jactorio::game::Logic_tick_event>(jactorio::game::event_type::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}

	TEST_F(EventTest, clear_all_data) {
		jactorio::game::Event::subscribe(jactorio::game::event_type::game_chunk_generated, test_callback1);
		jactorio::game::Event::clear_all_data();

		// Nothing gets raises since it is cleared
		jactorio::game::Event::raise<jactorio::game::Logic_tick_event>(jactorio::game::event_type::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}
}
