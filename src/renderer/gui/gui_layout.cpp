// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 07/08/2020

#include "renderer/gui/gui_layout.h"

#include "renderer/gui/gui_colors.h"
#include "renderer/rendering/renderer.h"

using namespace jactorio;

void renderer::FitTitle(std::stringstream& description_ss, const uint16_t target_len) {
	while (description_ss.str().size() < target_len)
		description_ss << " ";
}

void renderer::RemoveItemSlotTopPadding() {
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - kInventorySlotPadding);
}

void renderer::DrawSlots(const uint8_t slot_span, const uint16_t slot_count, const uint8_t scale,
                         const std::function<void(uint16_t, bool&)>& draw_func, const float ending_vertical_space) {
	uint16_t index      = 0;
	bool button_hovered = false;

	const auto original_cursor_x = ImGui::GetCursorPosX();

	// Start drawing without left padding
	float y_offset       = ImGui::GetCursorPosY();
	const float x_offset = ImGui::GetCursorPosX() - kInventorySlotPadding;

	while (index < slot_count) {
		const uint16_t x = index % slot_span;
		ImGui::SameLine(x_offset + x * scale * (kInventorySlotWidth + kInventorySlotPadding));

		ImGui::PushID(index);  // Uniquely identifies the button

		ImGui::SetCursorPosY(y_offset);
		draw_func(index, button_hovered);

		ImGui::PopID();

		if (x == slot_span - 1) {
			y_offset += static_cast<float>(scale) * (kInventorySlotWidth + kInventorySlotPadding);
		}

		++index;
	}

	// If final slot is drawn without a newline, add one
	if (slot_count % slot_span != 0) {
		ImGui::SetCursorPosY(y_offset);

		if (ending_vertical_space < 0)
			AddVerticalSpaceAbsolute(static_cast<float>(scale) * (kInventorySlotWidth + kInventorySlotPadding));
		else
			AddVerticalSpaceAbsolute(ending_vertical_space);
	}

	// Reset to original position for next widget
	ImGui::SetCursorPosX(original_cursor_x);
}

void renderer::DrawItemSlot(const MenuData& menu_data, const uint8_t scale, const uint32_t sprite_iid,
                            const uint16_t item_count, bool& button_hovered, const std::function<void()>& button_events) {
	const float x_offset = ImGui::GetCursorPosX();
	const float y_offset = ImGui::GetCursorPosY();

	bool backing_button_hover = false;
	// Backing button, detects clicks
	{
		J_GUI_RAII_STYLE_COLOR_POP(3);
		ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_NONE);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, J_GUI_COL_NONE);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, J_GUI_COL_NONE);

		ImGui::SetCursorPos({x_offset, y_offset});

		ImGui::ImageButton(
			nullptr,
			ImVec2(0, 0),
			ImVec2(-1, -1), ImVec2(-1, -1),
			(kInventorySlotWidth / 2 + kInventorySlotPadding) * scale
		);

		button_events();

		if (!button_hovered) {
			if ((backing_button_hover = ImGui::IsItemHovered()) == true)
				button_hovered = true;
		}
	}

	// Visible button, lower width such that a border is visible between slots
	{
		// Give visible button hover style if back is hovered
		if (backing_button_hover) {
			ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON_HOVER);
		}


		// Center in backing button
		ImGui::SetCursorPos({x_offset + kInventorySlotPadding, y_offset + kInventorySlotPadding});

		if (sprite_iid == 0) {
			// Blank button
			ImGui::ImageButton(
				nullptr,
				ImVec2(0, 0),
				ImVec2(-1, -1),
				ImVec2(-1, -1),
				(kInventorySlotWidth / 2 * scale) + ((scale - 1) * kInventorySlotImagePadding)
			);
		}
		else {
			const unsigned int button_size =
				scale * kInventorySlotWidth
				+ (scale - 1) * kInventorySlotPadding  // To align with other scales, account for the padding between slots
				- 2 * kInventorySlotImagePadding;

			const auto& uv = menu_data.spritePositions.at(sprite_iid);
			ImGui::ImageButton(
				reinterpret_cast<void*>(menu_data.texId),
				ImVec2(
					static_cast<float>(button_size),
					static_cast<float>(button_size)
				),
				ImVec2(uv.topLeft.x, uv.topLeft.y),
				ImVec2(uv.bottomRight.x, uv.bottomRight.y),
				kInventorySlotImagePadding
			);
		}


		if (backing_button_hover)
			ImGui::PopStyleColor();

		// Total items count
		if (item_count != 0) {
			ImGui::SetCursorPos({x_offset + kInventoryItemCountXOffset, y_offset + kInventoryItemCountYOffset});
			ImGui::Text("%d", item_count);
		}
	}
}

void renderer::DrawTitleBar(const std::string& title, const std::function<void()>& draw_func) {
	AddVerticalSpaceAbsolute(J_GUI_STYLE_FRAME_PADDING_Y);

	ImGui::Text("%s", title.c_str());
	draw_func();

	AddVerticalSpaceAbsolute(J_GUI_STYLE_TITLEBAR_PADDING_Y - J_GUI_VAR_ITEM_SPACING_Y);
}

// ======================================================================

void renderer::AddVerticalSpace(const float y) {
	AddVerticalSpaceAbsolute(y);
	ImGui::Dummy({0, 0});
}

void renderer::AddVerticalSpaceAbsolute(const float y) {
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
}

// ======================================================================

ImVec2 renderer::GetWindowSize() {
	// 20 is window padding on both sides, 80 for y is to avoid the scrollbar
	auto window_size = ImVec2(
		2 * J_GUI_STYLE_WINDOW_PADDING_X,
		2 * J_GUI_STYLE_WINDOW_PADDING_Y + 80);

	window_size.x += 10 * (kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding;
	window_size.y += static_cast<unsigned int>(game::PlayerData::kDefaultInventorySize / 10) *
		(kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding;

	return window_size;
}

ImVec2 renderer::GetWindowCenter() {
	return {
		static_cast<float>(renderer::Renderer::GetWindowWidth()) / 2,
		static_cast<float>(renderer::Renderer::GetWindowHeight()) / 2
	};
}

void renderer::SetupNextWindowLeft(const ImVec2& window_size) {
	// Uses pixel coordinates, top left is 0, 0, bottom right x, x
	// Character window is left of the center
	auto window_center = GetWindowCenter();
	window_center.x -= window_size.x;
	window_center.y -= window_size.y / 2;

	ImGui::SetNextWindowPos(window_center);
	ImGui::SetNextWindowSize(window_size);
}

void renderer::SetupNextWindowLeft() {
	SetupNextWindowLeft(GetWindowSize());
}

void renderer::SetupNextWindowRight(const ImVec2& window_size) {
	// Uses pixel coordinates, top left is 0, 0, bottom right x, x
	// Character window is left of the center
	auto window_center = GetWindowCenter();
	window_center.y -= window_size.y / 2;

	ImGui::SetNextWindowPos(window_center);
	ImGui::SetNextWindowSize(window_size);
}

void renderer::SetupNextWindowRight() {
	SetupNextWindowRight(GetWindowSize());
}

void renderer::SetupNextWindowCenter(const ImVec2& window_size) {
	auto window_center = GetWindowCenter();
	window_center.x -= window_size.x / 2;
	window_center.y -= window_size.y / 2;

	ImGui::SetNextWindowPos(window_center);
	ImGui::SetNextWindowSize(window_size);
}

void renderer::SetupNextWindowCenter() {
	SetupNextWindowCenter(GetWindowSize());
}
