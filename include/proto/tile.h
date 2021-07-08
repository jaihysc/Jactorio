// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_TILE_H
#define JACTORIO_INCLUDE_PROTO_TILE_H
#pragma once

#include "proto/framework/world_object.h"

namespace jactorio::proto
{
    class Sprite;

    /// Tiles which are auto-placed by the world generator
    /// Configure how the world generator places these tiles with a Noise_layer
    class Tile final : public FWorldObject
    {
    public:
        PROTOTYPE_CATEGORY(tile);

        Tile() = default;

        /// \param sprite_ptr Points to a sprite prototype
        explicit Tile(Sprite* sprite_ptr) : sprite(sprite_ptr) {}

        ~Tile() override = default;

        Tile(const Tile& other)     = default;
        Tile(Tile&& other) noexcept = default;
        Tile& operator=(const Tile& other) = default;
        Tile& operator=(Tile&& other) noexcept = default;

        // ======================================================================

        /// If true, resources will not spawn on this and player cannot navigate onto it
        PYTHON_PROP_REF_I(bool, isWater, false);

        /// Separately managed by PrototypeManager
        PYTHON_PROP_I(Sprite*, sprite, nullptr);


        // Renderer

        bool OnRShowGui(const gui::Context& /*context*/, game::ChunkTile* /*tile*/) const override {
            return false;
        }

        // Data

        void OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const override {}

        void PostLoadValidate(const data::PrototypeManager& proto) const override;
    };

    inline void Tile::PostLoadValidate(const data::PrototypeManager& /*proto*/) const {
        J_PROTO_ASSERT(sprite != nullptr, "Sprite was not provided");
    }
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_TILE_H
