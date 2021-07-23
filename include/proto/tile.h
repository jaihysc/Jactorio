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

        /// If true, resources will not spawn on this and player cannot navigate onto it
        PYTHON_PROP_REF_I(bool, isWater, false);

        // Renderer

        bool OnRShowGui(const gui::Context& /*context*/, game::ChunkTile* /*tile*/) const override {
            return false;
        }

        // Data

        void OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const override {}

        void PostLoadValidate(const data::PrototypeManager& proto) const override;
    };

    inline void Tile::PostLoadValidate(const data::PrototypeManager& proto) const {
        FWorldObject::PostLoadValidate(proto);
    }
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_TILE_H
