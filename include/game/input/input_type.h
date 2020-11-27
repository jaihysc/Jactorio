// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_INPUT_INPUT_TYPE_H
#define JACTORIO_INCLUDE_GAME_INPUT_INPUT_TYPE_H
#pragma once

#include <SDL_keycode.h>

namespace jactorio::game
{

    enum class InputAction
    {
        none,

        // First pressed down
        key_down,

        // While pressed down, before repeat
        key_pressed,
        // Press down enough to repeat
        key_repeat,

        // pressed and repeat
        key_held,

        // Key lifted
        key_up
    };

    enum class MouseInput
    {
        // Starting at 1 avoids conflict at 0 when stored in a int alongside SDL_Keycode

        left = 1,
        middle,
        right,
        x1,
        x2
    };

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_INPUT_INPUT_TYPE_H
