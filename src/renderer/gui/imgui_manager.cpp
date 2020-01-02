#include "renderer/gui/imgui_manager.h"

#include <imgui/imgui.h>

#include <unordered_map>
#include <cassert>

#include "jactorio.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/gui/gui_menus_debug.h"
#include "renderer/gui/imgui_glfw.h"
#include "renderer/gui/imgui_opengl3.h"
#include "renderer/rendering/renderer.h"
#include "renderer/window/window_manager.h"
#include "game/event/event.h"

ImGuiWindowFlags debug_window_flags = 0;
ImGuiWindowFlags release_window_flags = 0;

// Inventory
jactorio::renderer::imgui_manager::Character_menu_data menu_data;

void jactorio::renderer::imgui_manager::setup_character_data() {
	menu_data.sprite_positions =
		renderer_sprites::get_spritemap(data::Sprite::sprite_group::gui).sprite_positions;
	menu_data.tex_id = renderer_sprites::get_texture(data::Sprite::sprite_group::gui)->get_id();
}

// Errors
void jactorio::renderer::imgui_manager::show_error_prompt(const std::string& err_title,
                                                          const std::string& err_message) {
	bool quit = false;

	while (!quit) {
		Renderer::clear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("Error", nullptr, debug_window_flags);
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

	io.IniFilename = NULL;  // Disables imgui saving
	io.ConfigWindowsMoveFromTitleBarOnly = true;  //

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();


	// Factorio inspired Imgui style
	debug_window_flags |= ImGuiWindowFlags_NoCollapse;

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
	style.FrameBorderSize = 1.f;

	// Padding
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));

	// Window colors
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 230, 192, 255));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(49, 48, 49, 255));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, IM_COL32(49, 48, 49, 255));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(49, 48, 49, 255));

	// Inventory boxes
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(128, 129, 129, 255));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(144, 144, 145, 255));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(144, 144, 145, 255));

	// Buttons
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(128, 129, 129, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(227, 152, 39, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(227, 152, 39, 255));

	ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(0, 0, 0, 255));

	// The large horizontal row button like
	ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(128, 129, 129, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(227, 152, 39, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(227, 152, 39, 255));

	// Menu tabs
	ImGui::PushStyleColor(ImGuiCol_Tab, IM_COL32(128, 129, 129, 255));
	ImGui::PushStyleColor(ImGuiCol_TabHovered, IM_COL32(227, 152, 39, 255));
	ImGui::PushStyleColor(ImGuiCol_TabActive, IM_COL32(49, 48, 49, 255));

	// Resize tab
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, IM_COL32(98, 98, 98, 255));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, IM_COL32(128, 129, 129, 255));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, IM_COL32(128, 129, 129, 255));

	// Separators
	ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(128, 129, 129, 255));
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, IM_COL32(128, 129, 129, 255));
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, IM_COL32(128, 129, 129, 255));

	// Popup
	ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(49, 48, 49, 200));

	LOG_MESSAGE(info, "Imgui initialized");
}


#define WINDOW_PTR(function) (reinterpret_cast<void*>(jactorio::renderer::gui::function))
void* windows[]{
	WINDOW_PTR(character_menu),
	WINDOW_PTR(debug_menu_main)
};
#undef WINDOW_PTR

bool window_visibility[sizeof(windows) / sizeof(windows[0])];


void jactorio::renderer::imgui_manager::set_window_visibility(const gui_window window, const bool visibility) {
	const auto index = static_cast<int>(window);
	assert(index != -1);
	
	bool& old_visibility = window_visibility[index];

	if (visibility && !old_visibility) {
		// Window opened
		game::Event::raise<game::Gui_opened>(game::event_type::game_gui_open);
	}
	
	old_visibility = visibility;
}

bool jactorio::renderer::imgui_manager::get_window_visibility(gui_window window) {
	const auto index = static_cast<int>(window);
	assert(index != -1);

	return window_visibility[index];
}

/**
 * Draws windows conditionally
 * @param gui_window
 * @param window_flags
 * @param params Parameters to pass to the window function after the window flags
 */
template <typename ... ArgsT>
void draw_window(jactorio::renderer::imgui_manager::gui_window gui_window, const ImGuiWindowFlags window_flags, 
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

void jactorio::renderer::imgui_manager::imgui_draw() {
	EXECUTION_PROFILE_SCOPE(imgui_draw_timer, "Imgui draw");

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Make the font bigger
	// auto font = ImGui::GetFont();
	// font->Scale = 1.f;
	// ImGui::PushFont(font);
	// ImGui::PopFont();

	gui::cursor_window(release_window_flags, menu_data);

	draw_window(gui_window::character, release_window_flags, menu_data);
	draw_window(gui_window::debug, debug_window_flags);

	// Render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void jactorio::renderer::imgui_manager::imgui_terminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	LOG_MESSAGE(debug, "Imgui terminated");
}
