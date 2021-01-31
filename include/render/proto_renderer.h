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
    class RendererLayer;

    void DrawConveyorSegmentItems(RendererLayer& layer,
                                  const SpriteUvCoordsT& uv_coords,
                                  const core::Position2<float>& pixel_offset,
                                  game::ConveyorStruct& line_segment);

    void DrawInserterArm(RendererLayer& layer,
                         const SpriteUvCoordsT& uv_coords,
                         const core::Position2<float>& pixel_offset,
                         const proto::Inserter& inserter_proto,
                         const proto::InserterData& inserter_data);
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_PROTO_RENDERER_H
