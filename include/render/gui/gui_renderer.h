// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_GUI_GUI_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_GUI_GUI_RENDERER_H
#pragma once

#include <type_traits>

#include "render/gui/component_base.h"

namespace jactorio::render
{
    class GuiRenderer
    {
    public:
        template <typename T>
        T MakeComponent() {
            static_assert(std::is_base_of_v<GuiComponentBase, T>);
            return T(this);
        }
    };

} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_GUI_GUI_RENDERER_H
