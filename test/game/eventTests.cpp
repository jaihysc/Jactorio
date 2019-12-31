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

	
	void test_func(int a, int b) {
		counter++;
	}
	
	void test_func2(int a) {
		counter2--;
	}
	
	TEST(event, subscribe_raise_event) {
		using namespace jactorio::game;

		reset_counter();
		Event::clear_all_data();

		Event::subscribe(event_type::game_chunk_generated, test_func);
		Event::subscribe(event_type::game_gui_character_open, test_func2);
		
		Event::raise(event_type::game_chunk_generated, 1, 2);
		EXPECT_EQ(counter, 1);

		// Subscribing same handler twice, means it gets called twice
		Event::subscribe(event_type::game_gui_character_open, test_func2);
		Event::raise(event_type::game_gui_character_open, 1);

		EXPECT_EQ(counter2, -2);
	}

	TEST(event, unsubscribe_event) {
		using namespace jactorio::game;

		reset_counter();
		Event::clear_all_data();
		
		Event::subscribe(event_type::game_chunk_generated, test_func);
		
		EXPECT_EQ(Event::unsubscribe(event_type::game_chunk_generated, test_func), true);
		EXPECT_EQ(Event::unsubscribe(event_type::game_chunk_generated, test_func2), false);  // Does not exist

		
		Event::raise(event_type::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}

	TEST(event, clear_all_data) {
		using namespace jactorio::game;

		reset_counter();

		Event::subscribe(event_type::game_chunk_generated, test_func);
		Event::clear_all_data();

		// Nothing gets raises since it is cleared
		Event::raise(event_type::game_chunk_generated, 1);
		EXPECT_EQ(counter, 0);
	}
}