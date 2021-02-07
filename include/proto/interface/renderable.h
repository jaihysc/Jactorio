// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_INTERFACE_RENDERABLE_H
#define JACTORIO_INCLUDE_PROTO_INTERFACE_RENDERABLE_H
#pragma once

#include "jactorio.h"

#include "core/data_type.h"
#include "core/orientation.h"
#include "data/cereal/serialize.h"

namespace jactorio
{
    namespace proto
    {
        class Sprite;
        struct UniqueDataBase;
    } // namespace proto

    namespace game
    {
        class World;

        class ChunkTileLayer;
        class Chunk;
    } // namespace game

    namespace render
    {
        class RendererLayer;
        class GuiRenderer;
    } // namespace render
} // namespace jactorio

namespace jactorio::proto
{
    ///
    /// Inherit to allow drawing portions of a sprite
    struct IRenderableData
    {
    protected:
        IRenderableData()          = default;
        virtual ~IRenderableData() = default;

        explicit IRenderableData(const SpriteSetT set) : set(set) {}

    public:
        SpriteSetT set = 0;


        CEREAL_SERIALIZE(archive) {
            archive(set);
        }
    };

    ///
    /// For inheriting by IPrototypeRenderable, thus enabling usage by render
    class IRenderable
    {
    protected:
        IRenderable()          = default;
        virtual ~IRenderable() = default;

        IRenderable(const IRenderable& other)     = default;
        IRenderable(IRenderable&& other) noexcept = default;
        IRenderable& operator=(const IRenderable& other) = default;
        IRenderable& operator=(IRenderable&& other) noexcept = default;

    public:
        ///
        /// Gets a sprite corresponding to the provided set
        J_NODISCARD virtual Sprite* OnRGetSprite(SpriteSetT set) const = 0;

        ///
        /// Maps a orientation to a <set, frame>
        J_NODISCARD virtual SpriteSetT OnRGetSpriteSet(Orientation orientation,
                                                       game::World& world,
                                                       const WorldCoord& coord) const = 0;

        ///
        /// Gets frame for sprite corresponding to provided game tick
        J_NODISCARD virtual SpriteFrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data,
                                                           GameTickT game_tick) const = 0;

        ///
        /// Displays the menu associated with itself with the provided data
        virtual bool OnRShowGui(const render::GuiRenderer& g_rendr, game::ChunkTileLayer* tile_layer) const = 0;

        ///
        /// \param pixel_offset Pixels to top left of current tile
        virtual void OnRDrawUniqueData(render::RendererLayer& layer,
                                       const SpriteUvCoordsT& uv_coords,
                                       const Position2<float>& pixel_offset,
                                       const UniqueDataBase* unique_data) const {}

    protected:
        // ======================================================================
        // Methods for OnRGetSpriteFrame

        using AnimationSpeed = double;

        ///
        /// Every set / frame of a sprite is part of the same animation
        static SpriteFrameT AllOfSprite(Sprite& sprite, GameTickT game_tick, AnimationSpeed speed = 1);

        ///
        /// Every set / frame of a sprite is part of the same animation, plays forwards then backwards
        static SpriteFrameT AllOfSpriteReversing(Sprite& sprite, GameTickT game_tick, AnimationSpeed speed = 1);

        ///
        /// Every frame of a set
        static SpriteFrameT AllOfSet(Sprite& sprite, GameTickT game_tick, AnimationSpeed speed = 1);
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_INTERFACE_RENDERABLE_H
