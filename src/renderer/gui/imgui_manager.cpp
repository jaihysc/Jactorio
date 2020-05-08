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

// Inventory

const std::unordered_map<unsigned, jactorio::core::Quad_position>* sprite_positions = nullptr;
unsigned int tex_id = 0;  // Assigned by openGL

void jactorio::renderer::imgui_manager::setup_character_data(Renderer_sprites& renderer_sprites) {
	sprite_positions = &renderer_sprites.get_spritemap(data::Sprite::SpriteGroup::gui).sprite_positions;
	tex_id = renderer_sprites.get_texture(data::Sprite::SpriteGroup::gui)->get_id();
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

		ImGui::SetNextWindowPos({0, 0}, 0, {0.5, 0.5});

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

void draw_menu(jactorio::renderer::gui::menu menu, 
			   jactorio::game::Player_data& player_data, const jactorio::data::Unique_data_base* unique_data = nullptr) {
	auto& gui_menu = jactorio::renderer::gui::menus[static_cast<int>(menu)];

	if (gui_menu.visible) {
		gui_menu.draw_ptr(player_data, unique_data);
	}
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

	draw_menu(gui::menu::debug_menu, player_data);
	gui::debug_menu_logic(player_data);

	// Draw gui for active entity
	// Do not draw character and recipe menu while in an entity menu
	auto* layer = player_data.get_activated_layer();
	if (layer != nullptr) {
		gui::set_visible(gui::menu::character_menu, false);

		// Get the top left corner for non top left multi tiles
		if (layer->is_multi_tile() && !layer->is_multi_tile_top_left()) {
			layer = layer->get_multi_tile_parent();
		}

		static_cast<const data::Entity*>(layer->prototype_data)->on_r_show_gui(player_data, layer);
	}
	else {
		draw_menu(gui::menu::character_menu, player_data);
	}
	// Player gui
	gui::cursor_window(player_data);
	gui::crafting_queue(player_data);
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
