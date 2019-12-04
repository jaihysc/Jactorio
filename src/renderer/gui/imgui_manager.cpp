#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/logger.h"
#include "core/debug/execution_timer.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/gui/imgui_glfw.h"
#include "renderer/gui/imgui_opengl3.h"
#include "renderer/rendering/mvp_manager.h"
#include "renderer/rendering/renderer.h"
#include "renderer/rendering/world_renderer.h"
#include "renderer/rendering/renderer_manager.h"

bool show_timings_window = false;
bool show_demo_window = false;

ImGuiWindowFlags window_flags = 0;

void draw_debug_menu() {
	using namespace jactorio::renderer;

	ImGui::Begin("Debug menu", nullptr, window_flags);
	ImGui::Text("Units are pixels");

	// Settings
	glm::vec3* view_translation = mvp_manager::get_view_transform();
	ImGui::SliderFloat3("Camera translation", &view_translation->x, -100.0f,
	                    100.0f);

	ImGui::Text("Player position %lld %lld",
	            world_renderer::player_position_x,
	            world_renderer::player_position_y);

	ImGui::NewLine();
	ImGui::Text("Renderer");
	ImGui::Text("Layer count: %d", renderer_manager::prototype_layer_count);

	
	ImGui::NewLine();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// Window options
	ImGui::Checkbox("Timings", &show_timings_window); ImGui::SameLine();
	ImGui::Checkbox("Demo Window", &show_demo_window);
	 
	ImGui::End();
}

void draw_timings_menu() {
	using namespace jactorio::core;
	
	ImGui::Begin("Timings", nullptr, window_flags);
	ImGui::Text("%fms Frame time", 1000.0f / ImGui::GetIO().Framerate);
	
	for (auto& time : Execution_timer::measured_times) {
		ImGui::Text("%fms %s", time.second, time.first.c_str());
	}
	ImGui::End();
}

void jactorio::renderer::imgui_manager::setup(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	// Factorio inspired Imgui style
	window_flags |= ImGuiWindowFlags_NoCollapse;
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

	LOG_MESSAGE(info, "Imgui initialized");
}

void jactorio::renderer::imgui_manager::imgui_draw() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Make the font bigger
	// auto font = ImGui::GetFont();
	// font->Scale = 1.f;
	// ImGui::PushFont(font);
	// ImGui::PopFont();

	// Debug menu is ` key
	if (show_debug_menu)
		draw_debug_menu();
	
	if (show_demo_window)
		ImGui::ShowDemoWindow();
	if (show_timings_window)
		draw_timings_menu();
	
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
