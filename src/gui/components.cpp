// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include "gui/components.h"

#include <imgui.h>

#include "core/convert.h"
#include "core/utility.h"

#include "proto/item.h"
#include "proto/sprite.h"

#include "game/input/mouse_selection.h"

#include "gui/gui_colors.h"
#include "gui/menu_data.h"
#include "render/gui_renderer.h"

using namespace jactorio;

// ======================================================================

gui::GuiMenu::~GuiMenu() {
    ImGui::End();
}

void gui::GuiMenu::Begin(const char* name) const {
    ImGui::Begin(name, nullptr, flags_);
}

// ======================================================================


void gui::GuiItemSlots::Begin(const std::size_t slot_count, const BeginCallbackT& callback) const {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - kInventorySlotPadding);

    const auto original_cursor_x = ImGui::GetCursorPosX();

    float y_offset       = ImGui::GetCursorPosY();
    const float x_offset = ImGui::GetCursorPosX();

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

void gui::GuiItemSlots::DrawSlot(const PrototypeIdT sprite_id,
                                 const uint16_t item_count,
                                 const DrawSlotCallbackT& callback) const {
    const float original_x_offset = ImGui::GetCursorPosX();
    const float original_y_offset = ImGui::GetCursorPosY();

    // Backing button sticks out on all sides
    ImGui::SetCursorPos({original_x_offset - core::SafeCast<float>(kInventorySlotPadding / 2),
                         original_y_offset - core::SafeCast<float>(kInventorySlotPadding / 2)});

    DrawBackingButton();
    const bool backing_button_hover = ImGui::IsItemHovered();
    callback(); // Register events with backing button


    // Visible button, lower width such that a border is visible between slots

    // Give visible button hover style if back is hovered
    ImGuard guard;
    if (backing_button_hover) {
        guard.PushStyleColor(ImGuiCol_Button, kGuiColButtonHover);
    }


    ImGui::SetCursorPos({original_x_offset, original_y_offset});

    if (sprite_id == 0) {
        // Blank button
        const auto button_width  = core::SafeCast<float>(kInventorySlotWidth * scale);
        const auto padding_width = kInventorySlotPadding * (scale - 1);

        const auto total_width   = button_width + padding_width;
        const auto frame_padding = total_width / 2;

        ImGui::ImageButton(nullptr, {0, 0}, {-1, -1}, {-1, -1}, frame_padding);
    }
    else {
        const auto button_size = scale * kInventorySlotWidth +
            (scale - 1) * kInventorySlotPadding // To align with other scales, account for the padding between slots
            - 2 * kInventorySlotImagePadding;

        const auto& menu_data = guiRenderer_->menuData;

        const auto& uv = menu_data.spritePositions.at(sprite_id);
        ImGui::ImageButton(reinterpret_cast<void*>(menu_data.texId),
                           {core::SafeCast<float>(button_size), core::SafeCast<float>(button_size)},
                           {uv.topLeft.x, uv.topLeft.y},
                           {uv.bottomRight.x, uv.bottomRight.y},
                           kInventorySlotImagePadding);
    }


    // Total items count
    if (item_count != 0) {
        ImGui::SetCursorPos(
            {original_x_offset + kInventoryItemCountXOffset, original_y_offset + kInventoryItemCountYOffset});
        ImGui::Text("%d", item_count);
    }
}

void gui::GuiItemSlots::DrawSlot(const PrototypeIdT sprite_id, const DrawSlotCallbackT& callback) const {
    DrawSlot(sprite_id, 0, callback);
}

void gui::GuiItemSlots::DrawSlot(const proto::ItemStack& item_stack, const DrawSlotCallbackT& callback) const {
    DrawSlot(item_stack.item.Get() == nullptr ? 0 : item_stack.item->sprite->internalId, item_stack.count, callback);
}

void gui::GuiItemSlots::DrawBackingButton() const {
    ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Button, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_ButtonHovered, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_ButtonActive, kGuiColNone);

    // kInventorySlotWidth not multiplied by 2 so that the backing button is tile-able
    const auto width         = core::SafeCast<float>((kInventorySlotWidth + kInventorySlotPadding) * this->scale);
    const auto frame_padding = width / 2;

    assert(frame_padding * 2 == width); // Slots will not line up if does not halve evenly

    ImGui::ImageButton(nullptr, {0, 0}, {-1, -1}, {-1, -1}, frame_padding);
}

// ======================================================================

void gui::GuiTitle::Begin(const std::string& title, const CallbackT& callback) const {
    AddVerticalSpaceAbsolute(topPadding);

    ImGui::Text("%s", title.c_str());
    callback();

    AddVerticalSpaceAbsolute(bottomPadding);
}

void gui::DrawCursorTooltip(const bool has_selected_item,
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
