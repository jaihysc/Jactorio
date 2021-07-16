// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_PROTO_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_PROTO_RENDERER_H
#pragma once

#include "core/coordinate_tuple.h"
#include "core/data_type.h"

namespace jactorio
{
    namespace proto
    {
        class Inserter;
        struct InserterData;
    } // namespace proto

    namespace game
    {
        class ConveyorStruct;
    }
} // namespace jactorio

namespace jactorio::render
{
    struct IRenderBuffer;

    /// \param buf Prepares data to buf
    /// \param tex_coords Holds tex coord for items on conveyor
    /// \param pixel_offset Offset applied to each item rendered
    /// \param conveyor Conveyor to be rendered
    void PrepareConveyorSegmentItems(IRenderBuffer& buf,
                                     const SpriteTexCoords& tex_coords,
                                     const Position2<float>& pixel_offset,
                                     game::ConveyorStruct& conveyor);

    /// \param buf Prepares data to buf
    /// \param tex_coords Holds tex coord for items on conveyor
    /// \param pixel_offset Offset applied to each item rendered
    /// \param inserter Prototype of inserter to be rendered
    /// \param inserter_data Inserter to be rendered
    void PrepareInserterParts(IRenderBuffer& buf,
                              const SpriteTexCoords& tex_coords,
                              const Position2<float>& pixel_offset,
                              const proto::Inserter& inserter,
                              const proto::InserterData& inserter_data);
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_PROTO_RENDERER_H
