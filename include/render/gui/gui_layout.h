// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_GUI_GUI_LAYOUT_H
#define JACTORIO_INCLUDE_RENDER_GUI_GUI_LAYOUT_H
#pragma once

#include <functional>
#include <sstream>

#include <imgui.h>

#include "render/gui/imgui_manager.h"

namespace jactorio::render
{
    constexpr int kInventorySlotWidth        = 36;
    constexpr int kInventorySlotPadding      = 3;
    constexpr int kInventorySlotImagePadding = 2;

    constexpr int kInventoryItemCountXOffset = 0;
    constexpr int kInventoryItemCountYOffset = 0;

    // ======================================================================

    ///
    /// Pad the ingredients: text with trailing whitespace to reach the length of the title
    ///
    /// In order to auto resize to fit the title's text since the title is not accounted
    void FitTitle(std::stringstream& description_ss, std::size_t target_len);

    ///
    /// Emulates the ImGui title bar, but allows for drawing additional widgets other than text with the callback
    void DrawTitleBar(
        const std::string& title, const std::function<void()>& draw_func = []() {});

    // ======================================================================
    // Widget Positioning

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
