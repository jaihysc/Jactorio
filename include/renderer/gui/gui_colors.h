#ifndef RENDERER_GUI_GUI_COLORS_H
#define RENDERER_GUI_GUI_COLORS_H

#include "imgui/imgui.h"

// Defines macros for the color scheme used in Jactorio

// RAII style color pop
#define J_GUI_RAII_STYLE_POP(count) \
	jactorio::core::Resource_guard<void> imgui_style_guard([]() {ImGui::PopStyleColor(count); })


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

// Title bar
#define J_GUI_STYLE_FRAME_PADDING_X 10
#define J_GUI_STYLE_FRAME_PADDING_Y 10

#endif // RENDERER_GUI_GUI_COLORS_H
