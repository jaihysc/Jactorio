// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_GUI_MENU_DATA_H
#define JACTORIO_INCLUDE_RENDER_GUI_MENU_DATA_H
#pragma once

#include "core/data_type.h"

namespace jactorio::render
{
    struct MenuData
    {
        MenuData(const SpriteUvCoordsT& sprite_positions, const unsigned tex_id)
            : spritePositions(sprite_positions), texId(tex_id) {}

        const SpriteUvCoordsT& spritePositions;
        unsigned int texId = 0; // Assigned by openGL
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_GUI_MENU_DATA_H
