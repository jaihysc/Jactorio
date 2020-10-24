// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/gui_layout.h"

#include "game/player/player_data.h"
#include "gui/gui_colors.h"
#include "render/renderer.h"

using namespace jactorio;

float gui::GetFontHeight() {
    const auto* font = ImGui::GetFont();
    return font->Ascent - font->Descent;
}

void gui::AddVerticalSpace(const float y) {
    AddVerticalSpaceAbsolute(y);
    ImGui::Dummy({0, 0});
}

void gui::AddVerticalSpaceAbsolute(const float y) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
}

// ======================================================================

ImVec2 gui::GetWindowSize() {
    constexpr auto item_slot_per_row = 10;
    constexpr auto extra_vert_px     = 80; // Some extra space, for appearance purposes only


    constexpr auto size_x = GetTotalItemSlotWidth(item_slot_per_row) + GetTotalWindowPaddingX();
    constexpr auto size_y = GetTotalItemSlotWidth(
        core::SafeCast<unsigned int>(game::PlayerData::Inventory::kDefaultInventorySize / item_slot_per_row));

    return {size_x, size_y + extra_vert_px};
}

ImVec2 gui::GetWindowCenter() {
    return {core::SafeCast<float>(render::Renderer::GetWindowWidth()) / 2,
            core::SafeCast<float>(render::Renderer::GetWindowHeight()) / 2};
}

void gui::SetupNextWindowLeft(const ImVec2& window_size) {
    // Uses pixel coordinates, top left is 0, 0, bottom right x, x
    // Character window is left of the center
    auto window_center = GetWindowCenter();
    window_center.x -= window_size.x;
    window_center.y -= window_size.y / 2;

    ImGui::SetNextWindowPos(window_center);
    ImGui::SetNextWindowSize(window_size);
}

void gui::SetupNextWindowLeft() {
    SetupNextWindowLeft(GetWindowSize());
}

void gui::SetupNextWindowRight(const ImVec2& window_size) {
    // Uses pixel coordinates, top left is 0, 0, bottom right x, x
    // Character window is left of the center
    auto window_center = GetWindowCenter();
    window_center.y -= window_size.y / 2;

    ImGui::SetNextWindowPos(window_center);
    ImGui::SetNextWindowSize(window_size);
}

void gui::SetupNextWindowRight() {
    SetupNextWindowRight(GetWindowSize());
}

void gui::SetupNextWindowCenter(const ImVec2& window_size) {
    auto window_center = GetWindowCenter();
    window_center.x -= window_size.x / 2;
    window_center.y -= window_size.y / 2;

    ImGui::SetNextWindowPos(window_center);
    ImGui::SetNextWindowSize(window_size);
}

void gui::SetupNextWindowCenter() {
    SetupNextWindowCenter(GetWindowSize());
}
