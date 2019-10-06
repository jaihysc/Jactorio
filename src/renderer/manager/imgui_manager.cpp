#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/logger.h"

#include "renderer/manager/mvp_manager.h"
#include "renderer/manager/imgui_manager.h"
#include "renderer/gui/imgui_glfw.h"
#include "renderer/gui/imgui_opengl3.h"
#include "renderer/rendering/renderer.h"

// States
bool show_demo_window = false;

void jactorio::renderer::setup(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	
	logger::log_message(logger::info, "Imgui", "imgui Initialized");
}

void jactorio::renderer::imgui_draw() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Debug menu");

	ImGui::Text("1 translation unit is 1 tile"); // you can use a format strings too
	ImGui::Checkbox("Imgui doc", &show_demo_window);      // Edit bools storing our window open/close state


	glm::vec3* view_translation = Mvp_manager::get_view_transform();
	
	// Settings
	ImGui::SliderFloat3("Camera translation", &view_translation->x, -100.0f, 100.0f);
	ImGui::SliderFloat("Zoom", &Renderer::tile_projection_matrix_offset, 0.f, 100.0f);

	Mvp_manager::update_view_transform();
	Renderer::update_tile_projection_matrix();

	// Buttons return true when clicked (most widgets return true when edited/activated)
	ImGui::NewLine();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();


	// Render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void jactorio::renderer::imgui_terminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	logger::log_message(logger::info, "Imgui", "imgui terminated");
}