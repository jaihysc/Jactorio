// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_INTERFACE_RENDERABLE_H
#define JACTORIO_INCLUDE_PROTO_INTERFACE_RENDERABLE_H
#pragma once

#include "jactorio.h"

#include "core/data_type.h"
#include "core/orientation.h"
#include "proto/detail/python_prop.h"

namespace jactorio::game
{
    class World;
    class Chunk;
    class ChunkTile;
} // namespace jactorio::game

namespace jactorio::gui
{
    struct Context;
}

namespace jactorio::proto
{
    class Sprite;
    struct UniqueDataBase;
} // namespace jactorio::proto

namespace jactorio::render
{
    class RendererLayer;
} // namespace jactorio::render

namespace jactorio::proto
{
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
        PYTHON_PROP_I(Sprite*, sprite, nullptr);
        PYTHON_PROP_I(Sprite*, spriteE, nullptr);
        PYTHON_PROP_I(Sprite*, spriteS, nullptr);
        PYTHON_PROP_I(Sprite*, spriteW, nullptr);

        /// Gets the top left tex coord id at coord with orientation
        J_NODISCARD virtual SpriteTexCoordIndexT OnGetTexCoordId(const game::World& world,
                                                                 const WorldCoord& coord,
                                                                 Orientation orientation) const = 0;

        /// Displays the menu associated with itself with the provided data
        virtual bool OnRShowGui(const gui::Context& context, game::ChunkTile* tile) const = 0;

        /// \param pixel_offset Pixels to top left of current tile
        virtual void OnRDrawUniqueData(render::RendererLayer& layer,
                                       const SpriteTexCoords& uv_coords,
                                       const Position2<float>& pixel_offset,
                                       const UniqueDataBase* unique_data) const {}
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_INTERFACE_RENDERABLE_H
