// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_TILE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_TILE_H

#include "data/prototype/framework/world_object.h"

namespace jactorio::data
{
    class Sprite;

    ///
    /// Tiles which are auto-placed by the world generator <br>
    /// Configure how the world generator places these tiles with a Noise_layer
    class Tile final : public FWorldObject
    {
    public:
        PROTOTYPE_CATEGORY(tile);

        Tile() = default;

        ///
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

        /// Separately managed by data_manager
        PYTHON_PROP_I(Sprite*, sprite, nullptr);


        // ======================================================================
        // Renderer

        J_NODISCARD Sprite* OnRGetSprite(SpriteSetT /*set*/) const override {
            return sprite;
        }

        J_NODISCARD SpriteFrameT OnRGetSpriteFrame(const UniqueDataBase& /*unique_data*/,
                                                   GameTickT /*game_tick*/) const override {
            return 0;
        }

        J_NODISCARD SpriteSetT OnRGetSpriteSet(Orientation /*orientation*/,
                                               game::WorldData& /*world_data*/,
                                               const WorldCoord& /*world_coords*/) const override {
            return 0;
        }

        bool OnRShowGui(const render::GuiRenderer& /*g_rendr*/, game::ChunkTileLayer* /*tile_layer*/) const override {
            return false;
        }

        // ======================================================================
        // Data

        void OnDeserialize(game::WorldData& world_data,
                           const WorldCoord& world_coord,
                           game::ChunkTileLayer& tile_layer) const override {}

        void PostLoadValidate(const PrototypeManager& data_manager) const override;
    };

    inline void Tile::PostLoadValidate(const PrototypeManager& /*proto_manager*/) const {
        J_DATA_ASSERT(sprite != nullptr, "Sprite was not provided");
    }
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_TILE_H
