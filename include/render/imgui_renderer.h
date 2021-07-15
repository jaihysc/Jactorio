// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
#pragma once

#include <imgui.h>
#include <vector>

#include "render/opengl/index_buffer.h"
#include "render/opengl/shader.h"
#include "render/opengl/vertex_array.h"
#include "render/opengl/vertex_buffer.h"

namespace jactorio::render
{
    struct RendererCommon;

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
        explicit ImGuiRenderer(RendererCommon& common);

        ImGuiRenderer(const ImGuiRenderer& other)     = delete;
        ImGuiRenderer(ImGuiRenderer&& other) noexcept = delete;

        void Init();
        void Terminate();

        /// Binds the buffers used for rendering world and gui
        void Bind() const noexcept;

        /// \param tex_id Texture which will be used to render to world
        void RenderWorld(unsigned tex_id) const noexcept;
        void RenderGui(ImDrawData* draw_data) const;

        bool InitFontsTexture();

        // For drawing to the world
        // TODO make this a class, buffers can be mapped to avoid copying

        // This can be modified from const the same way imgui can rendered from const
        mutable std::vector<ImDrawVert> worldVert;
        mutable std::vector<ImDrawIdx> worldIndices;

    private:
        void DestroyFontsTexture();

        void SetupRenderState(ImDrawData* draw_data) const;

        RendererCommon* common_ = nullptr;

        Shader shader_;

        VertexBuffer vbo_;
        IndexBuffer index_;
        VertexArray vertexArray_;

        GLuint fontTexture_ = 0;
        // Uniforms location
        GLint attribLocationTex_ = 0, attribLocationProjMtx_ = 0;
        // Vertex attributes location
        GLuint attribLocationVtxPos_ = 0, attribLocationVtxUV_ = 0, attribLocationVtxColor_ = 0;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
