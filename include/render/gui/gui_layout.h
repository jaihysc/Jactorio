// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_RENDERER_GUI_GUI_LAYOUT_H
#define JACTORIO_RENDERER_GUI_GUI_LAYOUT_H
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
    /// \param title Title of the tooltip
    /// \param draw_func Code to run while drawing the tooltip
    void DrawCursorTooltip(game::PlayerData& player_data,
                           const data::PrototypeManager&,
                           const char* title,
                           const char* description,
                           const std::function<void()>& draw_func);


    ///
    /// Pad the ingredients: text with trailing whitespace to reach the length of the title
    ///
    /// In order to auto resize to fit the title's text since the title is not accounted
    void FitTitle(std::stringstream& description_ss, std::size_t target_len);

    ///
    /// Call before the first call to DrawSlots() to remove additional vertical padding
    void RemoveItemSlotTopPadding();

    ///
    /// Auto positions slots based on provided specifications, draws using provided callback <br>
    /// Call within a ImGui window
    /// \param slot_span Slots before wrapping onto new line
    /// \param slot_count Number of slots to draw
    /// \param draw_func Draws slot (index, bool button_hovered)
    /// \param ending_vertical_space If < 0 use default, >= 0 use amount provided of vertical space upon finishing
    /// drawing all slots
    void DrawSlots(uint8_t slot_span,
                   std::size_t slot_count,
                   uint8_t scale,
                   const std::function<void(std::size_t, bool&)>& draw_func,
                   float ending_vertical_space = -1.f);

    ///
    /// Positions item slots based on provided specifications
    /// \param sprite_iid Internal id of the sprite to be drawn, if 0, a blank slot will be drawn
    /// \param item_count Number to display on the item, 0 to hide
    /// \param button_hovered Used to ensure hover status is only applied to one item slot out of a set
    /// \param button_events Handle events from the button
    void DrawItemSlot(
        const MenuData& menu_data,
        uint8_t scale,
        uint32_t sprite_iid,
        uint16_t item_count,
        bool& button_hovered,
        const std::function<void()>& button_events = []() {});

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

#endif // JACTORIO_RENDERER_GUI_GUI_LAYOUT_H