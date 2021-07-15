// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#define JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#pragma once

#include <cstdint>
#include <utility>

#include "jactorio.h"

#include "game/world/chunk.h"
#include "proto/detail/type.h"

namespace jactorio::data
{
    class PrototypeManager;
}
namespace jactorio::proto
{
    class Entity;
} // namespace jactorio::proto

namespace jactorio::game
{
    class Player;
}

namespace jactorio::render
{
    class TileRenderer;
}

namespace jactorio::game
{
    /// Handles mouse input and selection
    class MouseSelection
    {
    public:
        static void SetCursor(const Position2<int32_t>& cursor_pos) noexcept;
        J_NODISCARD static Position2<int32_t> GetCursor() noexcept;

        // Rendering

        /// Draws a selection box if NO entity is selected
        /// otherwise, draws a ghost of the entity selected at the cursor
        static void DrawCursorOverlay(render::TileRenderer& renderer,
                                      GameWorlds& worlds,
                                      Player& player,
                                      const data::PrototypeManager& proto);

    private:
        static Position2<int32_t> cursorPos_;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
