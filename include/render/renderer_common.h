// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDERER_COMMON_H
#define JACTORIO_INCLUDE_RENDER_RENDERER_COMMON_H
#pragma once

#include "render/opengl/mvp_manager.h"

namespace jactorio::render
{
    // Shared between the tile and imgui renderer
    struct RendererCommon
    {
        MvpManager mvpManager;
    };
} // namespace jactorio::render
#endif // JACTORIO_INCLUDE_RENDER_RENDERER_COMMON_H
