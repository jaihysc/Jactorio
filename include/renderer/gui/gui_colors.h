// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_GUI_COLORS_H
#define JACTORIO_INCLUDE_RENDERER_GUI_GUI_COLORS_H
#pragma once

#include <imgui.h>

namespace jactorio::renderer
{
    /// Defines macros for the color scheme used in Jactorio

    constexpr ImU32 kGuiColNone = IM_COL32(0, 0, 0, 0);

    // Text
    constexpr ImU32 kGuiColText        = IM_COL32(255, 230, 192, 255);
    constexpr ImU32 kGuiColTextWarning = IM_COL32(212, 145, 52, 255);
    constexpr ImU32 kGuiColTextError   = IM_COL32(194, 101, 99, 255);

    // Tooltip
    constexpr ImU32 kGuiColTooltipTitleBg   = IM_COL32(224, 202, 169, 255);
    constexpr ImU32 kGuiColTooltipTitleText = IM_COL32(49, 48, 49, 255);

    // Window colors
    constexpr ImU32 kGuiColWindowBg      = IM_COL32(49, 48, 49, 255);
    constexpr ImU32 kGuiColTitleBg       = IM_COL32(49, 48, 49, 255);
    constexpr ImU32 kGuiColTitleBgActive = IM_COL32(49, 48, 49, 255);

    // Message / display boxes
    constexpr ImU32 kGuiColFrameBg       = IM_COL32(128, 129, 129, 255);
    constexpr ImU32 kGuiColFrameBgHover  = IM_COL32(144, 144, 145, 255);
    constexpr ImU32 kGuiColFrameBgActive = IM_COL32(144, 144, 145, 255);

    // Buttons - Inventory boxes
    constexpr ImU32 kGuiColButton       = IM_COL32(128, 129, 129, 255);
    constexpr ImU32 kGuiColButtonHover  = IM_COL32(227, 152, 39, 255);
    constexpr ImU32 kGuiColButtonActive = IM_COL32(227, 152, 39, 255);
    constexpr ImU32 kGuiColCheckmark    = IM_COL32(0, 0, 0, 255);

    // Large horizontal row button
    constexpr ImU32 kGuiColHeader       = IM_COL32(128, 129, 129, 255);
    constexpr ImU32 kGuiColHeaderHover  = IM_COL32(227, 152, 39, 255);
    constexpr ImU32 kGuiColHeaderActive = IM_COL32(227, 152, 39, 255);

    // Menu tabs
    constexpr ImU32 kGuiColTab       = IM_COL32(128, 129, 129, 255);
    constexpr ImU32 kGuiColTabHover  = IM_COL32(227, 152, 39, 255);
    constexpr ImU32 kGuiColTabActive = IM_COL32(49, 48, 49, 255);

    // Resize tab
    constexpr ImU32 kGuiColResizeGrip       = IM_COL32(98, 98, 98, 255);
    constexpr ImU32 kGuiColResizeGripHover  = IM_COL32(128, 129, 129, 255);
    constexpr ImU32 kGuiColResizeGripActive = IM_COL32(128, 129, 129, 255);

    // Separators
    constexpr ImU32 kGuiColSeparator       = IM_COL32(128, 129, 129, 255);
    constexpr ImU32 kGuiColSeparatorHover  = IM_COL32(128, 129, 129, 255);
    constexpr ImU32 kGuiColSeparatorActive = IM_COL32(128, 129, 129, 255);

    // Popup
    constexpr ImU32 kGuiColPopupBg = IM_COL32(49, 48, 49, 200);


    // Progress bar
    constexpr ImU32 kGuiColProgress   = IM_COL32(250, 168, 56, 255);
    constexpr ImU32 kGuiColProgressBg = IM_COL32(0, 0, 0, 255);

    //
    // Formatting

    // Body
    constexpr ImU32 kGuiStyleWindowPaddingX = 10;
    constexpr ImU32 kGuiStyleWindowPaddingY = 0;

    constexpr ImU32 kGuiStyleTitlebarPaddingY = 10;

    constexpr ImU32 kGuiStyleFramePaddingX = 10;
    constexpr ImU32 kGuiStyleFramePaddingY = 10;

    constexpr ImU32 kGuiStyleItemSpacingX = 8;
    constexpr ImU32 kGuiStyleItemSpacingY = 4;


    ///
    /// \brief RAII wrapper for imgui features
    class ImGuard
    {
        bool windowBegun_         = false;
        uint8_t styleColorPushed_ = 0;
        uint8_t styleVarPushed_   = 0;

    public:
        ImGuard() = default;

        ~ImGuard() {
            ImGui::PopStyleColor(styleColorPushed_);
            ImGui::PopStyleVar(styleVarPushed_);
            if (windowBegun_)
                ImGui::End();
        }

        ImGuard(const ImGuard& other)     = delete;
        ImGuard(ImGuard&& other) noexcept = delete;


        template <typename... Args>
        void Begin(Args&&... args) {
            ImGui::Begin(std::forward<Args>(args)...);
            windowBegun_ = true;
        }


        // Style color
        void PushStyleColor(const ImGuiCol idx, const ImU32 col) {
            ImGui::PushStyleColor(idx, col);
            styleColorPushed_++;
        }

        void PushStyleColor(const ImGuiCol idx, const ImVec4& col) {
            ImGui::PushStyleColor(idx, col);
            styleColorPushed_++;
        }

        // Style var
        void PushStyleVar(const ImGuiStyleVar idx, const float val) {
            ImGui::PushStyleVar(idx, val);
            styleVarPushed_++;
        }

        void PushStyleVar(const ImGuiStyleVar idx, const ImVec2& val) {
            ImGui::PushStyleVar(idx, val);
            styleVarPushed_++;
        }
    };
} // namespace jactorio::renderer


#endif // JACTORIO_INCLUDE_RENDERER_GUI_GUI_COLORS_H
