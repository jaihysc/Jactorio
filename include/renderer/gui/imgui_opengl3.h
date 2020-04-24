// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_OPENGL3_H
#define JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_OPENGL3_H
#pragma once

// dear imgui: Renderer for modern OpenGL with shaders / programmatic pipeline
// - Desktop GL: 3.x 4.x
// - Embedded GL: ES 2.0 (WebGL 1.0), ES 3.0 (WebGL 2.0)
// This needs to be used along with a Platform Binding (e.g. GLFW, SDL, Win32, custom..)

// Set default OpenGL3 loader to be gl3w
#if !defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)     \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)     \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)     \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#endif

IMGUI_IMPL_API bool ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_IMPL_API void ImGui_ImplOpenGL3_Shutdown();
IMGUI_IMPL_API void ImGui_ImplOpenGL3_NewFrame();
IMGUI_IMPL_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

// Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateFontsTexture();
IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyFontsTexture();
IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyDeviceObjects();


#endif //JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_OPENGL3_H
