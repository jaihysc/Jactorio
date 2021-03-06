// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_EVENT_TYPE_H
#define JACTORIO_INCLUDE_GAME_EVENT_EVENT_TYPE_H
#pragma once

namespace jactorio::game
{
    enum class EventType
    {
        none = 0,

        // User playing the game

        // Game window
        // window_move,
        // window_resize,

        // Keyboard/mouse input
        keyboard_activity, // Keyboard key has activity
        mouse_activity,    // Mouse key has activity
        input_activity,    // Keyboard or mouse activity
        // mouse_move,
        // mouse_scroll,


        // In-game

        logic_tick,    // Called every game tick
        renderer_tick, // Called prior event tick PRIOR to rendering

        // World gen
        game_chunk_generated, // TODO unimplemented

        // Player
        // game_player_mine,
        game_gui_open, // TODO unimplemented
        // game_gui_character_open,

        // Entity
        // game_entity_build,
        // game_entity_damaged,
        // game_entity_death,
    };

#define BIT_F(n) (1 << (n))

    // Bitfield of categories
    enum class EventCategory
    {
        none = 0,

        application = BIT_F(1), // Window

        input    = BIT_F(2),
        keyboard = BIT_F(3),
        mouse    = BIT_F(4),

        in_game = BIT_F(5)
    };

#undef BIT_F
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_EVENT_EVENT_TYPE_H
