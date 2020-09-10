// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include "render/gui/components.h"

#include <imgui.h>

#include "core/convert.h"
#include "data/prototype/item.h"
#include "game/input/mouse_selection.h"
#include "render/gui/gui_colors.h"
#include "render/gui/gui_renderer.h"

using namespace jactorio;

// ======================================================================

render::GuiMenu::GuiMenu() {
    ImGui::SetNextWindowSize(render::GetWindowSize());
}

render::GuiMenu::~GuiMenu() {
    ImGui::End();
}

void render::GuiMenu::Begin(const char* name) const {
    ImGui::Begin(name, nullptr, flags_);
}

void render::GuiMenu::DrawTitleBar(const std::string& title, const std::function<void()>& callback) {
    AddVerticalSpaceAbsolute(kGuiStyleFramePaddingY);

    ImGui::Text("%s", title.c_str());
    callback();

    AddVerticalSpaceAbsolute(kGuiStyleTitlebarPaddingY - kGuiStyleItemSpacingY);
}


// ======================================================================


void render::GuiSlotRenderer::Begin(const std::size_t slot_count,
                                    const render::GuiSlotRenderer::BeginCallbackT& callback) const {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - kInventorySlotPadding);

    const auto original_cursor_x = ImGui::GetCursorPosX();

    // Start drawing without left padding
    float y_offset       = ImGui::GetCursorPosY();
    const float x_offset = ImGui::GetCursorPosX() - kInventorySlotPadding;

    std::size_t index = 0;
    while (index < slot_count) {
        const uint16_t x = index % slotSpan;
        ImGui::SameLine(x_offset + core::SafeCast<float>(x * scale * (kInventorySlotWidth + kInventorySlotPadding)));

        ImGui::PushID(core::SafeCast<int>(index)); // Uniquely identifies the button

        ImGui::SetCursorPosY(y_offset);
        callback(index);

        ImGui::PopID();

        if (x == slotSpan - 1) {
            y_offset += core::SafeCast<float>(scale) * (kInventorySlotWidth + kInventorySlotPadding);
        }

        ++index;
    }

    // If final slot is drawn without a newline, add one
    if (slot_count % slotSpan != 0) {
        ImGui::SetCursorPosY(y_offset);

        if (endingVerticalSpace < 0)
            AddVerticalSpaceAbsolute(core::SafeCast<float>(scale) * (kInventorySlotWidth + kInventorySlotPadding));
        else
            AddVerticalSpaceAbsolute(endingVerticalSpace);
    }

    ImGui::SetCursorPosX(original_cursor_x);                              // Reset to original position for next widget
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + kInventorySlotPadding); // Allows consecutive begins to line up
}

void render::GuiSlotRenderer::DrawSlot(const data::PrototypeIdT sprite_id,
                                       const uint16_t item_count,
                                       const DrawSlotCallbackT& callback) {
    const float x_offset = ImGui::GetCursorPosX();
    const float y_offset = ImGui::GetCursorPosY();

    ImGui::SetCursorPos({x_offset, y_offset});

    DrawBackingButton();
    const bool backing_button_hover = ImGui::IsItemHovered();
    callback(); // Register events with backing button


    // Visible button, lower width such that a border is visible between slots

    // Give visible button hover style if back is hovered
    ImGuard guard;
    if (backing_button_hover) {
        guard.PushStyleColor(ImGuiCol_Button, kGuiColButtonHover);
    }


    // Center in backing button
    ImGui::SetCursorPos({x_offset + kInventorySlotPadding, y_offset + kInventorySlotPadding});

    if (sprite_id == 0) {
        // Blank button
        ImGui::ImageButton(nullptr,
                           ImVec2(0, 0),
                           ImVec2(-1, -1),
                           ImVec2(-1, -1),
                           (kInventorySlotWidth / 2 * scale) + ((scale - 1) * kInventorySlotImagePadding));
    }
    else {
        const auto button_size = scale * kInventorySlotWidth +
            (scale - 1) * kInventorySlotPadding // To align with other scales, account for the padding between slots
            - 2 * kInventorySlotImagePadding;

        const auto& menu_data = guiRenderer_->menuData;

        const auto& uv = menu_data.spritePositions.at(sprite_id);
        ImGui::ImageButton(reinterpret_cast<void*>(menu_data.texId),
                           ImVec2(core::SafeCast<float>(button_size), core::SafeCast<float>(button_size)),
                           ImVec2(uv.topLeft.x, uv.topLeft.y),
                           ImVec2(uv.bottomRight.x, uv.bottomRight.y),
                           kInventorySlotImagePadding);
    }


    // Total items count
    if (item_count != 0) {
        ImGui::SetCursorPos({x_offset + kInventoryItemCountXOffset, y_offset + kInventoryItemCountYOffset});
        ImGui::Text("%d", item_count);
    }
}

void render::GuiSlotRenderer::DrawSlot(const data::PrototypeIdT sprite_id,
                                       const render::GuiSlotRenderer::DrawSlotCallbackT& callback) {
    DrawSlot(sprite_id, 0, callback);
}

void render::GuiSlotRenderer::DrawSlot(const data::ItemStack& item_stack,
                                       const render::GuiSlotRenderer::DrawSlotCallbackT& callback) {
    DrawSlot(item_stack.item.Get() == nullptr ? 0 : item_stack.item->sprite->internalId, item_stack.count, callback);
}

void render::GuiSlotRenderer::DrawBackingButton() const {
    render::ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Button, render::kGuiColNone);
    guard.PushStyleColor(ImGuiCol_ButtonHovered, render::kGuiColNone);
    guard.PushStyleColor(ImGuiCol_ButtonActive, render::kGuiColNone);

    const auto width   = core::SafeCast<float>((kInventorySlotWidth + kInventorySlotPadding) * this->scale);
    const auto padding = width / 2;

    assert(padding * 2 == width); // Slots will not line up if does not halve evenly

    ImGui::ImageButton(nullptr, ImVec2(0, 0), ImVec2(-1, -1), ImVec2(-1, -1), padding);
}


// ======================================================================


void render::DrawCursorTooltip(bool has_selected_item,
                               const std::string& title,
                               const std::string& description,
                               const std::function<void()>& draw_func) {

    ImVec2 cursor_pos(core::LossyCast<float>(game::MouseSelection::GetCursorX()),
                      core::LossyCast<float>(game::MouseSelection::GetCursorY()) + 10.f);

    // If an item is currently selected, move the tooltip down to not overlap
    if (has_selected_item)
        cursor_pos.y += kInventorySlotWidth;

    ImGui::SetNextWindowPos(cursor_pos);


    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoInputs;
    flags |= ImGuiWindowFlags_NoScrollbar;
    flags |= ImGuiWindowFlags_AlwaysAutoResize;

    // Draw tooltip
    ImGuard guard;

    guard.PushStyleColor(ImGuiCol_TitleBgActive, kGuiColTooltipTitleBg);
    guard.PushStyleColor(ImGuiCol_TitleBg, kGuiColTooltipTitleBg);

    {
        ImGuard title_text_guard;
        title_text_guard.PushStyleColor(ImGuiCol_Text, kGuiColTooltipTitleText);
        guard.Begin(title.c_str(), nullptr, flags);
    }

    ImGui::Text("%s", core::StrMatchLen(description, title.size()).c_str());

    draw_func();

    // This window is always in front
    ImGui::SetWindowFocus(title.c_str());
}
