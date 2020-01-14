#ifndef GAME_EVENT_EVENT_TYPE_H
#define GAME_EVENT_EVENT_TYPE_H

namespace jactorio::game
{
	enum class event_type
	{
		// TODO world gen events, entity, player are not implemented
		none = 0,

		// User playing the game

		// Game window
		window_move, window_resize,
		
		// Keyboard/mouse input
		key_press, key_repeat, key_release,
		mouse_move, mouse_scroll,

		
		// In-game
		
		logic_tick,  // Called every game tick
		renderer_tick,  // Called prior event tick PRIOR to rendering
		
		// World gen
		game_chunk_generated,
		
		// Player
		game_player_mine,
		game_gui_open,
		game_gui_character_open,

		// Entity
		game_entity_build,
		game_entity_damaged,
		game_entity_death,
	};

#define BIT_F(n) (1 << (n))

	// Bitfield of categories
	enum event_category
	{
		none = 0,

		application = BIT_F(1),  // Window
		
		input = BIT_F(2),
		keyboard = BIT_F(3),
		mouse = BIT_F(4),

		in_game = BIT_F(5)
	};

#undef BIT_F
}

#endif // GAME_EVENT_EVENT_TYPE_H
