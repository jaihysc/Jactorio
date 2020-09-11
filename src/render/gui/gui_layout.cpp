// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/gui/gui_layout.h"

#include "game/input/mouse_selection.h"
#include "render/gui/gui_colors.h"
#include "render/rendering/renderer.h"

using namespace jactorio;

float render::GetFontHeight() {
    const auto* font = ImGui::GetFont();
    return font->Ascent - font->Descent;
}

void render::AddVerticalSpace(const float y) {
    AddVerticalSpaceAbsolute(y);
    ImGui::Dummy({0, 0});
}

void render::AddVerticalSpaceAbsolute(const float y) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
}

// ======================================================================

ImVec2 render::GetWindowSize() {
    constexpr auto item_slot_per_row = 10;
    constexpr auto extra_vert_px     = 80; // Some extra space, for appearance purposes only


    constexpr auto size_x = GetTotalItemSlotWidth(item_slot_per_row) + GetTotalWindowPaddingX();
    constexpr auto size_y = GetTotalItemSlotWidth(
        core::SafeCast<unsigned int>(game::PlayerData::Inventory::kDefaultInventorySize / item_slot_per_row));

    return {size_x, size_y + extra_vert_px};
}

ImVec2 render::GetWindowCenter() {
    return {core::SafeCast<float>(Renderer::GetWindowWidth()) / 2,
            core::SafeCast<float>(Renderer::GetWindowHeight()) / 2};
}

void render::SetupNextWindowLeft(const ImVec2& window_size) {
    // Uses pixel coordinates, top left is 0, 0, bottom right x, x
    // Character window is left of the center
    auto window_center = GetWindowCenter();
    window_center.x -= window_size.x;
    window_center.y -= window_size.y / 2;

    ImGui::SetNextWindowPos(window_center);
    ImGui::SetNextWindowSize(window_size);
}

void render::SetupNextWindowLeft() {
    SetupNextWindowLeft(GetWindowSize());
}

void render::SetupNextWindowRight(const ImVec2& window_size) {
    // Uses pixel coordinates, top left is 0, 0, bottom right x, x
    // Character window is left of the center
    auto window_center = GetWindowCenter();
    window_center.y -= window_size.y / 2;

    ImGui::SetNextWindowPos(window_center);
    ImGui::SetNextWindowSize(window_size);
}

void render::SetupNextWindowRight() {
    SetupNextWindowRight(GetWindowSize());
}

void render::SetupNextWindowCenter(const ImVec2& window_size) {
    auto window_center = GetWindowCenter();
    window_center.x -= window_size.x / 2;
    window_center.y -= window_size.y / 2;

    ImGui::SetNextWindowPos(window_center);
    ImGui::SetNextWindowSize(window_size);
}

void render::SetupNextWindowCenter() {
    SetupNextWindowCenter(GetWindowSize());
}
