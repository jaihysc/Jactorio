// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_RENDERER_RENDERING_DATA_RENDERER_H
#define JACTORIO_RENDERER_RENDERING_DATA_RENDERER_H
#pragma once

#include "data/prototype/inserter.h"
#include "data/prototype/interface/renderable.h"
#include "game/logic/transport_segment.h"

namespace jactorio::render
{
    void DrawTransportSegmentItems(RendererLayer& layer,
                                   const SpriteUvCoordsT& uv_coords,
                                   const core::Position2<float>& pixel_offset,
                                   game::TransportSegment& line_segment);

    void DrawInserterArm(RendererLayer& layer,
                         const SpriteUvCoordsT& uv_coords,
                         const core::Position2<float>& pixel_offset,
                         const data::Inserter& inserter_proto,
                         const data::InserterData& inserter_data);
} // namespace jactorio::render

#endif // JACTORIO_RENDERER_RENDERING_DATA_RENDERER_H
