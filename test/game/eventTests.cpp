#include <gtest/gtest.h>

#include "game/event/event.h"

namespace game
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

	void test_callback2(int a, int b) {
		counter2++;
	}

	TEST(event, subscribe_raise_event) {
		using namespace jactorio::game;

		reset_counter();
		Event::clear_all_data();

		Event::subscribe(event_type::logic_tick, test_callback1);
		// Event::subscribe(event_type::game_gui_character_open, test_callback2);

		Event::raise<Logic_tick_event>(event_type::logic_tick, 12);
		EXPECT_EQ(counter, 12);
	}

	// TEST(event, subscribe_raise_event_imgui_bock) {
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
	
	TEST(event, unsubscribe_event) {
		using namespace jactorio::game;
	
		reset_counter();
		Event::clear_all_data();
		
		Event::subscribe(event_type::game_chunk_generated, test_callback1);
		
		EXPECT_EQ(Event::unsubscribe(event_type::game_chunk_generated, test_callback1), true);
		EXPECT_EQ(Event::unsubscribe(event_type::game_chunk_generated, test_callback2), false);  // Does not exist
	
		// Unchanged since unsubscribed
		Event::raise<Logic_tick_event>(event_type::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}
	
	TEST(event, clear_all_data) {
		using namespace jactorio::game;
	
		reset_counter();
	
		Event::subscribe(event_type::game_chunk_generated, test_callback1);
		Event::clear_all_data();
	
		// Nothing gets raises since it is cleared
		Event::raise<Logic_tick_event>(event_type::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}
}