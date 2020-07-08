// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 02/07/2020

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_GUI_COLORS_H
#define JACTORIO_INCLUDE_RENDERER_GUI_GUI_COLORS_H
#pragma once

#include <imgui.h>

// Defines macros for the color scheme used in Jactorio

namespace jactorio::renderer
{
	class ImGuard
	{
		bool windowBegun_ = false;
		uint8_t styleColorPushed_ = 0;
		uint8_t styleVarPushed_ = 0;

	public:
		ImGuard() = default;

		~ImGuard() {
			ImGui::PopStyleColor(styleColorPushed_);
			ImGui::PopStyleVar(styleVarPushed_);
			if (windowBegun_)
				ImGui::End();
		}

		ImGuard(const ImGuard& other) = delete;
		ImGuard(ImGuard&& other) noexcept = delete;


		template <typename ... Args>
		void Begin(Args&& ... args) {
			ImGui::Begin(std::forward<Args>(args) ...);
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
	
}

#define J_GUI_COL_NONE                    IM_COL32(0, 0, 0, 0)

// Text
#define J_GUI_COL_TEXT                    IM_COL32(255, 230, 192, 255)
#define J_GUI_COL_TEXT_WARNING            IM_COL32(212, 145, 52, 255)
#define J_GUI_COL_TEXT_ERROR              IM_COL32(194, 101, 99, 255)

// Tooltip
#define J_GUI_COL_TOOLTIP_TITLE_BG        IM_COL32(224, 202, 169, 255)
#define J_GUI_COL_TOOLTIP_TITLE_TEXT      IM_COL32(49, 48, 49, 255)

// Window colors
#define J_GUI_COL_WINDOW_BG               IM_COL32(49, 48, 49, 255)
#define J_GUI_COL_TITLE_BG                IM_COL32(49, 48, 49, 255)
#define J_GUI_COL_TITLE_BG_ACTIVE         IM_COL32(49, 48, 49, 255)

// Message / display boxes
#define J_GUI_COL_FRAME_BG                IM_COL32(128, 129, 129, 255)
#define J_GUI_COL_FRAME_BG_HOVER          IM_COL32(144, 144, 145, 255)
#define J_GUI_COL_FRAME_BG_ACTIVE         IM_COL32(144, 144, 145, 255)

// Buttons - Inventory boxes
#define J_GUI_COL_BUTTON                  IM_COL32(128, 129, 129, 255)
#define J_GUI_COL_BUTTON_HOVER            IM_COL32(227, 152, 39, 255)
#define J_GUI_COL_BUTTON_ACTIVE           IM_COL32(227, 152, 39, 255)
#define J_GUI_COL_CHECKMARK               IM_COL32(0, 0, 0, 255)

// Large horizontal row button
#define J_GUI_COL_HEADER                  IM_COL32(128, 129, 129, 255)
#define J_GUI_COL_HEADER_HOVER            IM_COL32(227, 152, 39, 255)
#define J_GUI_COL_HEADER_ACTIVE           IM_COL32(227, 152, 39, 255)

// Menu tabs
#define J_GUI_COL_TAB                     IM_COL32(128, 129, 129, 255)
#define J_GUI_COL_TAB_HOVER               IM_COL32(227, 152, 39, 255)
#define J_GUI_COL_TAB_ACTIVE              IM_COL32(49, 48, 49, 255)

// Resize tab
#define J_GUI_COL_RESIZE_GRIP             IM_COL32(98, 98, 98, 255)
#define J_GUI_COL_RESIZE_GRIP_HOVER       IM_COL32(128, 129, 129, 255)
#define J_GUI_COL_RESIZE_GRIP_ACTIVE      IM_COL32(128, 129, 129, 255)

// Separators
#define J_GUI_COL_SEPARATOR               IM_COL32(128, 129, 129, 255)
#define J_GUI_COL_SEPARATOR_HOVER         IM_COL32(128, 129, 129, 255)
#define J_GUI_COL_SEPARATOR_ACTIVE        IM_COL32(128, 129, 129, 255)

// Popup
#define J_GUI_COL_POPUP_BG                IM_COL32(49, 48, 49, 200)


// Progress bar
#define J_GUI_COL_PROGRESS                IM_COL32(250, 168, 56, 255)
#define J_GUI_COL_PROGRESS_BG             IM_COL32(0, 0, 0, 255)

//
// Formatting
// 

// Body
#define J_GUI_STYLE_WINDOW_PADDING_X 10
#define J_GUI_STYLE_WINDOW_PADDING_Y 0

#define J_GUI_STYLE_TITLEBAR_PADDING_Y 10

#define J_GUI_STYLE_FRAME_PADDING_X 10
#define J_GUI_STYLE_FRAME_PADDING_Y 10

#define J_GUI_VAR_ITEM_SPACING_X 8
#define J_GUI_VAR_ITEM_SPACING_Y 4

#endif //JACTORIO_INCLUDE_RENDERER_GUI_GUI_COLORS_H
