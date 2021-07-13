// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

// ReSharper disable CppInconsistentNaming

#ifndef JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
#pragma once

#include "imgui.h"

namespace jactorio::render
{
    // The name is ImGui Renderer because it is adapted from ImGui's renderer, but it can be used
    // for Jactorio rendering as well
    // It is useful for rendering more complex geometry, which our renderer highly optimized for tiles
    // cannot do (inserter arms, conveyor items, text, ...)

    bool ImGui_ImplOpenGL3_Init();
    void ImGui_ImplOpenGL3_Shutdown();
    void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

    // (Optional) Called by Init/Shutdown
    bool ImGui_ImplOpenGL3_CreateFontsTexture();
    void ImGui_ImplOpenGL3_DestroyFontsTexture();
    bool ImGui_ImplOpenGL3_CreateDeviceObjects();
    void ImGui_ImplOpenGL3_DestroyDeviceObjects();
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_IMGUI_RENDERER_H
