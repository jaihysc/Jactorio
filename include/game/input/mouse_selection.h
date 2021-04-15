// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#define JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#pragma once

#include <cstdint>
#include <utility>

#include "jactorio.h"

#include "game/world/chunk.h"
#include "proto/detail/type.h"

namespace jactorio
{
    namespace data
    {
        class PrototypeManager;
    }
    namespace proto
    {
        class Entity;
    } // namespace proto

    namespace game
    {
        class Player;
    }
} // namespace jactorio

namespace jactorio::game
{
    /// Handles mouse input and selection
    class MouseSelection
    {
        static constexpr OverlayLayer kCursorOverlayLayer_ = OverlayLayer::cursor;

    public:
        static void SetCursor(int32_t x_pos, int32_t y_pos) noexcept;

        J_NODISCARD static int32_t GetCursorX() noexcept;
        J_NODISCARD static int32_t GetCursorY() noexcept;
        J_NODISCARD static Position2<int32_t> GetCursor() noexcept;

        // ======================================================================
        // Client only mouse selection (affects only rendering) For Player mouse selection, see player

        /// Draws a selection box if NO entity is selected, otherwise, draws a ghost of the entity selected at the
        /// cursor
        void DrawCursorOverlay(GameWorlds& worlds, Player& player, const data::PrototypeManager& proto);

        /// Draws cursor_sprite when over entity & no item selected or item not placeable
        /// With item selected: draws ghost of entity
        void DrawOverlay(World& world,
                         const WorldCoord& coord,
                         Orientation orientation,
                         const proto::Entity* selected_entity,
                         const proto::Sprite& cursor_sprite);

    private:
        ChunkCoord lastChunkPos_               = {0, 0};
        const proto::Sprite* lastCursorSprite_ = nullptr;

        void RemoveLastOverlay(World& world) const;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
