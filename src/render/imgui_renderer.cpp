// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

// ReSharper disable CppCStyleCast
// ReSharper disable CppClangTidyCppcoreguidelinesInitVariables
// ReSharper disable CppClangTidyCppcoreguidelinesProTypeCstyleCast
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppClangTidyReadabilityIsolateDeclaration
// ReSharper disable CppClangTidyReadabilityImplicitBoolConversion
// ReSharper disable CppParameterMayBeConst

#include "render/imgui_renderer.h"

#include <cstdint> // intptr_t

#include "core/convert.h"
#include "render/opengl/error.h"
#include "render/renderer_common.h"

using namespace jactorio;

render::ImGuiRenderer::ImGuiRenderer(RendererCommon& common) : common_(&common) {}

void render::ImGuiRenderer::Init() {
    // Setup back-end capabilities flags
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_opengl3";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field,
                                                               // allowing for large meshes.

    shader_.Init({{"data/core/shaders/im_vs.vert", GL_VERTEX_SHADER}, //
                  {"data/core/shaders/im_fs.frag", GL_FRAGMENT_SHADER}});

    attribLocationTex_      = shader_.GetUniformLocation("u_texture");
    attribLocationProjMtx_  = shader_.GetUniformLocation("u_proj_mtx");
    attribLocationVtxPos_   = (GLuint)shader_.GetAttribLocation("Position");
    attribLocationVtxUV_    = (GLuint)shader_.GetAttribLocation("UV");
    attribLocationVtxColor_ = (GLuint)shader_.GetAttribLocation("Color");

    vbo_.Init();
    index_.Init();
    vertexArray_.Init();

    // Setup attributes for ImDrawVert
    vertexArray_.Bind();
    vbo_.Bind();
    index_.Bind();

    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(attribLocationVtxPos_));
    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(attribLocationVtxUV_));
    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(attribLocationVtxColor_));
    DEBUG_OPENGL_CALL(glVertexAttribPointer(attribLocationVtxPos_, //
                                            2,
                                            GL_FLOAT,
                                            GL_FALSE,
                                            sizeof(ImDrawVert),
                                            (GLvoid*)IM_OFFSETOF(ImDrawVert, pos)));
    DEBUG_OPENGL_CALL(glVertexAttribPointer(attribLocationVtxUV_, //
                                            2,
                                            GL_FLOAT,
                                            GL_FALSE,
                                            sizeof(ImDrawVert),
                                            (GLvoid*)IM_OFFSETOF(ImDrawVert, uv)));
    DEBUG_OPENGL_CALL(glVertexAttribPointer(attribLocationVtxColor_,
                                            4,
                                            GL_UNSIGNED_BYTE,
                                            GL_TRUE,
                                            sizeof(ImDrawVert),
                                            (GLvoid*)IM_OFFSETOF(ImDrawVert, col)));
    VertexArray::Unbind();
}

void render::ImGuiRenderer::Terminate() {
    DestroyFontsTexture();
}

void render::ImGuiRenderer::Bind() const noexcept {
    shader_.Bind();
    vertexArray_.Bind();
    vbo_.Bind();
    index_.Bind();
}

void render::ImGuiRenderer::RenderWorld(const unsigned tex_id) const noexcept {
    DEBUG_OPENGL_CALL(
        glUniformMatrix4fv(attribLocationProjMtx_, 1, GL_FALSE, &common_->mvpManager.GetMvpMatrix()[0][0]));

    DEBUG_OPENGL_CALL(glBufferData(GL_ARRAY_BUFFER, //
                                   SafeCast<GLsizeiptr>(worldVert.size()) * sizeof(ImDrawVert),
                                   worldVert.data(),
                                   GL_STREAM_DRAW));

    // This can be generated ahead of time?
    DEBUG_OPENGL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, //
                                   SafeCast<GLsizeiptr>(worldIndices.size()) * sizeof(ImDrawIdx),
                                   worldIndices.data(),
                                   GL_STREAM_DRAW));

    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, tex_id));
    DEBUG_OPENGL_CALL(
        glDrawElements(GL_TRIANGLES,                           //
                       SafeCast<GLsizei>(worldIndices.size()), // Count is indices in index array, NOT triangle number
                       sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                       nullptr));
}

void render::ImGuiRenderer::RenderGui(ImDrawData* draw_data) const {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer
    // coordinates)
    int fb_width  = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;
    SetupRenderState(draw_data);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert),
                     (const GLvoid*)cmd_list->VtxBuffer.Data,
                     GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx),
                     (const GLvoid*)cmd_list->IdxBuffer.Data,
                     GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr) {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer
                // to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState(draw_data);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
                    // Apply scissor/clipping rectangle
                    glScissor((int)clip_rect.x,
                              (int)(fb_height - clip_rect.w),
                              (int)(clip_rect.z - clip_rect.x),
                              (int)(clip_rect.w - clip_rect.y));

                    // Bind texture, Draw
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                    glDrawElementsBaseVertex(GL_TRIANGLES,
                                             (GLsizei)pcmd->ElemCount,
                                             sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                             (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)),
                                             (GLint)pcmd->VtxOffset);
                }
            }
        }
    }

    // Scissor must be disabled PRIOR to gl clear or it does not clear the entire screen
    glDisable(GL_SCISSOR_TEST);
}

bool render::ImGuiRenderer::InitFontsTexture() {
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(
        &pixels, &width, &height); // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small)
                                   // because it is more likely to be compatible with user's existing shaders. If your
                                   // ImTextureId represent a higher-level concept than just a GL texture id, consider
                                   // calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Upload texture to graphics system
    glGenTextures(1, &fontTexture_);
    glBindTexture(GL_TEXTURE_2D, fontTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)fontTexture_;

    return true;
}

void render::ImGuiRenderer::DestroyFontsTexture() {
    if (fontTexture_) {
        ImGuiIO& io = ImGui::GetIO();
        glDeleteTextures(1, &fontTexture_);
        io.Fonts->TexID = 0;
        fontTexture_    = 0;
    }
}

void render::ImGuiRenderer::SetupRenderState(ImDrawData* draw_data) const {
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    glEnable(GL_SCISSOR_TEST);

    // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
    bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
    GLenum current_clip_origin = 0;
    glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&current_clip_origin);
    if (current_clip_origin == GL_UPPER_LEFT)
        clip_origin_lower_left = false;
#endif

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    if (!clip_origin_lower_left) {
        float tmp = T;
        T         = B;
        B         = tmp;
    } // Swap top and bottom if origin is upper left
    const float ortho_projection[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
    };
    glUniform1i(attribLocationTex_, 0);
    glUniformMatrix4fv(attribLocationProjMtx_, 1, GL_FALSE, &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
    glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif
}
