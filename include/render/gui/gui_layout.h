// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_GUI_GUI_LAYOUT_H
#define JACTORIO_INCLUDE_RENDER_GUI_GUI_LAYOUT_H
#pragma once

#include <functional>
#include <sstream>

#include <imgui.h>

#include "jactorio.h"

#include "render/gui/gui_colors.h"

namespace jactorio::render
{
    constexpr int kInventorySlotWidth = 36;
    /// Between slots
    constexpr int kInventorySlotPadding = 2;
    /// Between image and edge of slot
    constexpr int kInventorySlotImagePadding = 2;

    constexpr int kInventoryItemCountXOffset = 0;
    constexpr int kInventoryItemCountYOffset = 0;

    // ======================================================================
    // Widget Positioning

    ///
    /// Gets height of current ImGui font
    float GetFontHeight();

    constexpr unsigned GetTotalItemSlotWidth(const unsigned slots, const unsigned scale = 1) {
        assert(scale > 0);

        if (slots == 0)
            return 0;
        return slots * scale * (kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding;
    }

    constexpr unsigned GetTotalWindowPaddingX() {
        return 2 * kGuiStyleWindowPaddingX;
    }

    ///
    /// \param items Items after the first item
    constexpr unsigned GetTotalWindowItemSpacingX(const unsigned items) {
        return items * kGuiStyleItemSpacingX;
    };

    ///
    /// Adds additional vertical space
    void AddVerticalSpace(float y);

    ///
    /// Adds additional vertical space, disregarding the imgui style var "itemSpacing"
    void AddVerticalSpaceAbsolute(float y);


    // ======================================================================
    // Window positioning

    ///
    /// The window size is calculated on the size of the player's inventory
    J_NODISCARD ImVec2 GetWindowSize();

    ///
    /// Gets the center window position
    J_NODISCARD ImVec2 GetWindowCenter();


    ///
    /// The next window drawn will be on the left center of the screen
    void SetupNextWindowLeft(const ImVec2& window_size);
    void SetupNextWindowLeft();

    ///
    /// The next window drawn will be on the left center of the screen
    void SetupNextWindowRight(const ImVec2& window_size);
    void SetupNextWindowRight();

    ///
    /// The next window drawn will be centered on the screen
    void SetupNextWindowCenter(const ImVec2& window_size);
    void SetupNextWindowCenter();
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_GUI_GUI_LAYOUT_H
