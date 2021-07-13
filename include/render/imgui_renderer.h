// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
#pragma once

#include "render/opengl/shader.h"

struct ImDrawData;

namespace jactorio::render
{
    /// The name is ImGui Renderer because it is adapted from ImGui's renderer, but it can be used
    /// for Jactorio rendering as well
    /// It is useful for rendering more complex geometry, which our renderer highly optimized for tiles
    /// cannot do (inserter arms, conveyor items, text, ...)
    /// \remark Must be in imgui context for lifetime of this object
    class ImGuiRenderer
    {
        using GLuint = unsigned;
        using GLint  = int;

    public:
        void Init();
        void Terminate();
        void Render(ImDrawData* draw_data) const;

        bool InitFontsTexture();

    private:
        void DestroyFontsTexture();

        bool InitDeviceObjects();
        void DestroyDeviceObjects();

        void SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object) const;

        Shader shader_;

        GLuint fontTexture_ = 0;
        // Uniforms location
        GLint attribLocationTex_ = 0, attribLocationProjMtx_ = 0;
        // Vertex attributes location
        GLuint attribLocationVtxPos_ = 0, attribLocationVtxUV_ = 0, attribLocationVtxColor_ = 0;

        unsigned int vboHandle_ = 0, elementsHandle_ = 0;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
