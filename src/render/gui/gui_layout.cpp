// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/gui/gui_layout.h"

#include "game/input/mouse_selection.h"
#include "render/gui/gui_colors.h"
#include "render/rendering/renderer.h"

using namespace jactorio;

void render::FitTitle(std::stringstream& description_ss, const std::size_t target_len) {
    while (description_ss.str().size() < target_len)
        description_ss << " ";
}

void render::DrawTitleBar(const std::string& title, const std::function<void()>& draw_func) {
    AddVerticalSpaceAbsolute(kGuiStyleFramePaddingY);

    ImGui::Text("%s", title.c_str());
    draw_func();

    AddVerticalSpaceAbsolute(kGuiStyleTitlebarPaddingY - kGuiStyleItemSpacingY);
}

// ======================================================================

void render::AddVerticalSpace(const float y) {
    AddVerticalSpaceAbsolute(y);
    ImGui::Dummy({0, 0});
}

void render::AddVerticalSpaceAbsolute(const float y) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
}

// ======================================================================

ImVec2 render::GetWindowSize() {
    // 20 is window padding on both sides, 80 for y is to avoid the scrollbar
    auto window_size = ImVec2(2 * kGuiStyleWindowPaddingX, 2 * kGuiStyleWindowPaddingY + 80);

    window_size.x += 10 * (kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding;
    window_size.y += core::SafeCast<unsigned int>(game::PlayerData::Inventory::kDefaultInventorySize / 10) *
            (kInventorySlotWidth + kInventorySlotPadding) -
        kInventorySlotPadding;

    return window_size;
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
