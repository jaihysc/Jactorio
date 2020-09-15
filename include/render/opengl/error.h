// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_ERROR_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_ERROR_H
#pragma once

// Different break statements depending on the compiler
#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak();
#endif

#ifdef __GNUC__
#include <signal.h>
#define DEBUG_BREAK raise(SIGTRAP);
#undef _MSC_VER
#endif


// Wrap a function with DEBUG_OPENGL_CALL to automatically call this when an error occurs and pause code execution
#ifdef JACTORIO_DEBUG_BUILD
#define DEBUG_OPENGL_CALL(instructions__)                                           \
    jactorio::render::OpenglClearErrors();                                          \
    instructions__;                                                                 \
    if ((jactorio::render::OpenglPrintErrors(#instructions__, __FILE__, __LINE__))) \
        DEBUG_BREAK                                                                 \
    static_assert(true)

#else
#define DEBUG_OPENGL_CALL(func) func;
#endif

namespace jactorio::render
{
    bool OpenglPrintErrors(const char* function_name, const char* file, int line);
    void OpenglClearErrors();
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_ERROR_H
