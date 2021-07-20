// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

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
    io.BackendRendererName = "Jactorio impl_opengl3";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field,
                                                               // allowing for large meshes.

    shader_.Init({{"data/core/shaders/im_vs.vert", GL_VERTEX_SHADER}, //
                  {"data/core/shaders/im_fs.frag", GL_FRAGMENT_SHADER}});

    attribLocationTex_      = shader_.GetUniformLocation("u_texture");
    attribLocationProjMtx_  = shader_.GetUniformLocation("u_proj_mtx");
    attribLocationVtxPos_   = SafeCast<GLuint>(shader_.GetAttribLocation("Position"));
    attribLocationVtxUV_    = SafeCast<GLuint>(shader_.GetAttribLocation("UV"));
    attribLocationVtxColor_ = SafeCast<GLuint>(shader_.GetAttribLocation("Color"));

    buffer.GlInit();
    vertexArray_.Init();

    // Setup attributes for ImDrawVert
    vertexArray_.Bind();
    buffer.GlBind();

    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(attribLocationVtxPos_));
    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(attribLocationVtxUV_));
    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(attribLocationVtxColor_));
    DEBUG_OPENGL_CALL(glVertexAttribPointer(attribLocationVtxPos_, //
                                            2,
                                            GL_FLOAT,
                                            GL_FALSE,
                                            sizeof(ImDrawVert),
                                            reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, pos))));
    DEBUG_OPENGL_CALL(glVertexAttribPointer(attribLocationVtxUV_, //
                                            2,
                                            GL_FLOAT,
                                            GL_FALSE,
                                            sizeof(ImDrawVert),
                                            reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, uv))));
    DEBUG_OPENGL_CALL(glVertexAttribPointer(attribLocationVtxColor_,
                                            4,
                                            GL_UNSIGNED_BYTE,
                                            GL_TRUE,
                                            sizeof(ImDrawVert),
                                            reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, col))));
    VertexArray::Unbind();
}

void render::ImGuiRenderer::Terminate() {
    DestroyFontsTexture();
}

void render::ImGuiRenderer::Bind() const noexcept {
    shader_.Bind();
    vertexArray_.Bind();
    buffer.GlBind();
}

void render::ImGuiRenderer::RenderWorld(const unsigned tex_id) const noexcept {
    // On Linux, if a draw call is made with no indices, it covers up the world rendered underneath
    // blanking the screen
    if (buffer.IdxCount() > 0) {
        DEBUG_OPENGL_CALL(
            glUniformMatrix4fv(attribLocationProjMtx_, 1, GL_FALSE, &common_->mvpManager.GetMvpMatrix()[0][0]));

        // Data for drawing is already prepared by mapping buffers

        DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, tex_id));
        DEBUG_OPENGL_CALL(
            glDrawElements(GL_TRIANGLES,                         //
                           SafeCast<GLsizei>(buffer.IdxCount()), // Count is indices in index array, NOT triangle number
                           sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                           nullptr));
    }

    buffer.GlHandleBufferResize(); // May need to resize while gui is not open
}

void render::ImGuiRenderer::RenderGui(ImDrawData* draw_data) const {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer
    // coordinates)
    const auto fb_width  = LossyCast<int>(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    const auto fb_height = LossyCast<int>(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;
    SetupRenderState(draw_data);

    // Will project scissor/clipping rectangles into framebuffer space
    const ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
    const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Check for capacity
        if (SafeCast<uint32_t>(cmd_list->VtxBuffer.Size) > buffer.VtxCapacity()) {
            buffer.ReserveVtx(cmd_list->VtxBuffer.Size);
        }
        if (SafeCast<uint32_t>(cmd_list->IdxBuffer.Size) > buffer.IdxCapacity()) {
            buffer.ReserveIdx(cmd_list->IdxBuffer.Size);
        }
        buffer.GlHandleBufferResize();

        // Upload vertex/index buffers
        buffer.GlWriteBegin();
        for (auto& vtx : cmd_list->VtxBuffer) {
            buffer.UncheckedPushVtx(vtx);
        }
        for (auto& idx : cmd_list->IdxBuffer) {
            buffer.UncheckedPushIdx(idx);
        }
        buffer.GlWriteEnd();

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
                    DEBUG_OPENGL_CALL(glScissor(LossyCast<int>(clip_rect.x),
                                                LossyCast<int>(fb_height - clip_rect.w),
                                                LossyCast<int>(clip_rect.z - clip_rect.x),
                                                LossyCast<int>(clip_rect.w - clip_rect.y)));

                    // Bind texture, Draw
                    DEBUG_OPENGL_CALL(
                        glBindTexture(GL_TEXTURE_2D, SafeCast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId))));
                    DEBUG_OPENGL_CALL(glDrawElementsBaseVertex(
                        GL_TRIANGLES,
                        SafeCast<GLsizei>(pcmd->ElemCount),
                        sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                        reinterpret_cast<void*>(SafeCast<intptr_t>(pcmd->IdxOffset * sizeof(ImDrawIdx))),
                        SafeCast<GLint>(pcmd->VtxOffset)));
                }
            }
        }
    }

    // Scissor must be disabled PRIOR to gl clear or it does not clear the entire screen
    DEBUG_OPENGL_CALL(glDisable(GL_SCISSOR_TEST));
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
    DEBUG_OPENGL_CALL(glGenTextures(1, &fontTexture_));
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, fontTexture_));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
#ifdef GL_UNPACK_ROW_LENGTH
    DEBUG_OPENGL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
#endif
    DEBUG_OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

    // Store our identifier
    io.Fonts->TexID = reinterpret_cast<ImTextureID>(SafeCast<intptr_t>(fontTexture_));

    return true;
}

void render::ImGuiRenderer::DestroyFontsTexture() {
    if (fontTexture_ != 0u) {
        ImGuiIO& io = ImGui::GetIO();
        DEBUG_OPENGL_CALL(glDeleteTextures(1, &fontTexture_));
        io.Fonts->TexID = 0;
        fontTexture_    = 0;
    }
}

void render::ImGuiRenderer::SetupRenderState(ImDrawData* draw_data) const {
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    DEBUG_OPENGL_CALL(glEnable(GL_SCISSOR_TEST));

    // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
    bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
    GLenum current_clip_origin = 0;
    DEBUG_OPENGL_CALL(glGetIntegerv(GL_CLIP_ORIGIN, reinterpret_cast<GLint*>(&current_clip_origin)));
    if (current_clip_origin == GL_UPPER_LEFT)
        clip_origin_lower_left = false;
#endif

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    const auto l = draw_data->DisplayPos.x;
    const auto r = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    auto t       = draw_data->DisplayPos.y;
    auto b       = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    if (!clip_origin_lower_left) {
        const auto tmp = t;
        t              = b;
        b              = tmp;
    } // Swap top and bottom if origin is upper left
    const float ortho_projection[4][4] = {
        {2.0f / (r - l), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (t - b), 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {(r + l) / (l - r), (t + b) / (b - t), 0.0f, 1.0f},
    };
    DEBUG_OPENGL_CALL(glUniform1i(attribLocationTex_, 0));
    DEBUG_OPENGL_CALL(glUniformMatrix4fv(attribLocationProjMtx_, 1, GL_FALSE, &ortho_projection[0][0]));
#ifdef GL_SAMPLER_BINDING
    // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
    DEBUG_OPENGL_CALL(glBindSampler(0, 0));
#endif
}
