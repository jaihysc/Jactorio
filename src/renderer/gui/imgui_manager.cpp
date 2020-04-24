// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include "renderer/gui/imgui_manager.h"

#include <cassert>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include "jactorio.h"

#include "data/prototype/entity/entity.h"
#include "game/event/event.h"
#include "game/player/player_data.h"
#include "renderer/gui/gui_colors.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/gui/gui_menus_debug.h"
#include "renderer/gui/imgui_glfw.h"
#include "renderer/gui/imgui_opengl3.h"
#include "renderer/rendering/renderer.h"
#include "renderer/window/window_manager.h"

ImGuiWindowFlags release_window_flags = 0;

// Inventory

const std::unordered_map<unsigned, jactorio::core::Quad_position>* sprite_positions = nullptr;
unsigned int tex_id = 0;  // Assigned by openGL

void jactorio::renderer::imgui_manager::setup_character_data(Renderer_sprites& renderer_sprites) {
	sprite_positions = &renderer_sprites.get_spritemap(data::Sprite::spriteGroup::gui).sprite_positions;
	tex_id = renderer_sprites.get_texture(data::Sprite::spriteGroup::gui)->get_id();
}

jactorio::renderer::imgui_manager::Menu_data jactorio::renderer::imgui_manager::get_menu_data() {
	return {*sprite_positions, tex_id};
}


// Errors
void jactorio::renderer::imgui_manager::show_error_prompt(const std::string& err_title,
                                                          const std::string& err_message) {
	bool quit = false;

	while (!quit) {
		Renderer::g_clear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPosCenter();

		const ImGuiWindowFlags flags = 0;
		ImGui::Begin("Error", nullptr, flags);

		ImGui::TextWrapped("%s", err_title.c_str());
		ImGui::TextWrapped("%s", err_message.c_str());
		ImGui::NewLine();
		quit = ImGui::Button("Close");

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window_manager::get_window());  // Done rendering
		glfwPollEvents();
	}
}


void jactorio::renderer::imgui_manager::setup(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	io.IniFilename = nullptr;  // Disables imgui saving
	io.ConfigWindowsMoveFromTitleBarOnly = true;  //

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();


	// Factorio inspired Imgui style
	release_window_flags |= ImGuiWindowFlags_NoCollapse;
	release_window_flags |= ImGuiWindowFlags_NoResize;

	auto& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.TabRounding = 0.0f;

	// Borders
	style.FrameBorderSize = 0.f;

	// Padding
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
	                    ImVec2(J_GUI_STYLE_WINDOW_PADDING_X, J_GUI_STYLE_WINDOW_PADDING_Y));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
	                    ImVec2(J_GUI_STYLE_FRAME_PADDING_X, J_GUI_STYLE_FRAME_PADDING_Y));

	// Window colors
	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, J_GUI_COL_WINDOW_BG);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, J_GUI_COL_TITLE_BG);
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, J_GUI_COL_TITLE_BG_ACTIVE);

	// Inventory boxes
	ImGui::PushStyleColor(ImGuiCol_FrameBg, J_GUI_COL_FRAME_BG);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, J_GUI_COL_FRAME_BG_HOVER);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, J_GUI_COL_FRAME_BG_ACTIVE);

	// Buttons
	ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, J_GUI_COL_BUTTON_HOVER);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, J_GUI_COL_BUTTON_ACTIVE);

	ImGui::PushStyleColor(ImGuiCol_CheckMark, J_GUI_COL_CHECKMARK);

	// The large horizontal row button like
	ImGui::PushStyleColor(ImGuiCol_Header, J_GUI_COL_HEADER);
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, J_GUI_COL_HEADER_HOVER);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, J_GUI_COL_HEADER_ACTIVE);

	// Menu tabs
	ImGui::PushStyleColor(ImGuiCol_Tab, J_GUI_COL_TAB);
	ImGui::PushStyleColor(ImGuiCol_TabHovered, J_GUI_COL_TAB_HOVER);
	ImGui::PushStyleColor(ImGuiCol_TabActive, J_GUI_COL_TAB_ACTIVE);

	// Resize tab
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, J_GUI_COL_RESIZE_GRIP);
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, J_GUI_COL_RESIZE_GRIP_HOVER);
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, J_GUI_COL_RESIZE_GRIP_ACTIVE);

	// Separators
	ImGui::PushStyleColor(ImGuiCol_Separator, J_GUI_COL_SEPARATOR);
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, J_GUI_COL_SEPARATOR_HOVER);
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, J_GUI_COL_SEPARATOR_ACTIVE);

	// Popup
	ImGui::PushStyleColor(ImGuiCol_PopupBg, J_GUI_COL_POPUP_BG);

	LOG_MESSAGE(info, "Imgui initialized");
}

// Pointers to windows are kept in a list defined below
// A boolean array is generated equal to the number of window pointers
// It stores whether or not the window is open or closed
//
// Use set_window_visibility() to set the visibility of the windows
#define WINDOW_PTR(function) (reinterpret_cast<void*>(jactorio::renderer::gui::function))
void* windows[]{
	WINDOW_PTR(character_menu),
	WINDOW_PTR(debug_menu_main)
};
#undef WINDOW_PTR

bool window_visibility[sizeof(windows) / sizeof(windows[0])];


void jactorio::renderer::imgui_manager::set_window_visibility(game::Event_data& event,
															  const guiWindow window, const bool visibility) {
	const auto index = static_cast<int>(window);
	assert(index != -1);

	bool& old_visibility = window_visibility[index];

	if (visibility && !old_visibility) {
		// Window opened
		event.raise<game::Gui_opened_event>(game::eventType::game_gui_open);
	}

	old_visibility = visibility;
}

bool jactorio::renderer::imgui_manager::get_window_visibility(guiWindow window) {
	const auto index = static_cast<int>(window);
	assert(index != -1);

	return window_visibility[index];
}

// TODO get rid of this vvv, it is unsafe with how it erases types
/**
 * Draws windows conditionally
 * @param gui_window
 * @param window_flags
 * @param params Parameters to pass to the window function after the window flags
 */
template <typename ... ArgsT>
void draw_window(jactorio::renderer::imgui_manager::guiWindow gui_window, const ImGuiWindowFlags window_flags,
                 ArgsT& ... params) {
	const int window_index = static_cast<int>(gui_window);
	assert(window_index != -1);

	// Window is hidden?
	if (!window_visibility[window_index])
		return;

	const auto function_ptr = reinterpret_cast<void(*)(ImGuiWindowFlags, ArgsT ...)>
		(windows[window_index]);

	function_ptr(window_flags, params ...);
}

void jactorio::renderer::imgui_manager::imgui_draw(game::Player_data& player_data, game::Event_data& event) {
	EXECUTION_PROFILE_SCOPE(imgui_draw_timer, "Imgui draw");

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Has imgui handled a mouse or keyboard event?
	ImGuiIO& io = ImGui::GetIO();
	input_captured = io.WantCaptureKeyboard || io.WantCaptureMouse;

	// Make the font bigger
	// auto font = ImGui::GetFont();
	// font->Scale = 1.f;
	// ImGui::PushFont(font);
	// ImGui::PopFont();

	draw_window<game::Player_data&>(guiWindow::debug, 0, player_data);
	gui::debug_menu_logic(player_data);

	// Draw gui for active entity
	// Do not draw character and recipe menu while in an entity menu
	auto* layer = player_data.get_activated_layer();
	if (layer != nullptr) {
		set_window_visibility(event, guiWindow::character, false);

		// Get the top left corner for non top left multi tiles
		if (layer->is_multi_tile() && !layer->is_multi_tile_top_left()) {
			layer = layer->get_multi_tile_parent();
		}

		static_cast<const data::Entity*>(layer->prototype_data)->on_r_show_gui(player_data, layer);
	}
	else {
		draw_window<game::Player_data&>(guiWindow::character, release_window_flags, player_data);
	}

	gui::crafting_queue(player_data);
	gui::cursor_window(player_data);
	gui::pickup_progressbar(player_data);

	// Render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void jactorio::renderer::imgui_manager::imgui_terminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	LOG_MESSAGE(info, "Imgui terminated");
}
