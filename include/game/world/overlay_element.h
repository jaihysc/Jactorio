// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_OVERLAY_ELEMENT_H
#define JACTORIO_INCLUDE_GAME_WORLD_OVERLAY_ELEMENT_H
#pragma once

#include "jactorio.h"

#include "core/coordinate_tuple.h"
#include "core/data_type.h"

namespace jactorio::proto
{
    class Sprite;
}

namespace jactorio::game
{
    /// Named values for each z buffer number
    enum class OverlayLayer
    {
        // A separate layer is only needed when it needs to be accessed independently, otherwise join together in single
        // layer
        cursor = 0,
        debug,
        count_
    };

    constexpr int kOverlayLayerCount = static_cast<int>(OverlayLayer::count_);


    /// Holds items which do not adhere to the tiles for rendering
    class OverlayElement
    {
        /// z position of first layer
        static constexpr float kDefaultZPos = 0.4f;
        /// Every layer above increases its z position by this amount
        static constexpr float kZPosMultiplier = 0.01f;

    public:
        /*
        OverlayElement(const proto::Sprite& sprite,
                       const Position2<PositionT>& position,
                       const Position2<PositionT>& size)
            : OverlayElement(sprite, Position3<PositionT>{position, 0.f}, size) {
        }
        */

        OverlayElement(const proto::Sprite& sprite,
                       const Position2<OverlayOffsetAxis>& position,
                       const Position2<OverlayOffsetAxis>& size,
                       const OverlayLayer layer)
            : OverlayElement(sprite, Position3<OverlayOffsetAxis>{position, ToZPosition(layer)}, size) {}

        OverlayElement(const proto::Sprite& sprite,
                       const Position3<OverlayOffsetAxis>& position,
                       const Position2<OverlayOffsetAxis>& size)
            : sprite(&sprite), position(position), size(size) {}

        // ======================================================================

        void SetZPosition(const OverlayOffsetAxis z_pos) {
            position.z = z_pos;
        }

        void SetZPosition(const OverlayLayer layer) {
            assert(layer != OverlayLayer::count_);
            SetZPosition(ToZPosition(layer));
        }

        J_NODISCARD static OverlayOffsetAxis ToZPosition(const OverlayLayer layer) noexcept {
            return kDefaultZPos + kZPosMultiplier * static_cast<float>(layer);
        }

        // ======================================================================

        const proto::Sprite* sprite;

        SpriteSetT spriteSet = 0;

        /// Distance (tiles) from top left of chunk to top left of sprite + z value
        Position3<OverlayOffsetAxis> position;

        /// Distance (tiles) the sprite spans
        Position2<OverlayOffsetAxis> size;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_OVERLAY_ELEMENT_H
