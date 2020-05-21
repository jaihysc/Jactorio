// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/20/2020

#ifndef JACTORIO_RENDERER_RENDERING_DATA_RENDERER_H
#define JACTORIO_RENDERER_RENDERING_DATA_RENDERER_H
#pragma once

#include "game/logic/transport_segment.h"

namespace jactorio::renderer
{
	void DrawTransportSegmentItems(RendererLayer& layer,
	                               float x_offset, float y_offset,
	                               game::TransportSegment& line_segment);
}

#endif // JACTORIO_RENDERER_RENDERING_DATA_RENDERER_H
