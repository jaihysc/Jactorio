// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDERER_EXCEPTION_H
#define JACTORIO_INCLUDE_RENDER_RENDERER_EXCEPTION_H
#pragma once

#include <stdexcept>

namespace jactorio::render
{
    // These are raised by render classes if an error occurred
    class RendererException : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };
} // namespace jactorio::render


#endif // JACTORIO_INCLUDE_RENDER_RENDERER_EXCEPTION_H
