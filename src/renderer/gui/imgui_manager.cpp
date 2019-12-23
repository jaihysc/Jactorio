#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/gui/imgui_manager.h"
#include "core/debug/execution_timer.h"
#include "core/logger.h"
#include "data/data_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_generator.h"
#include "renderer/gui/imgui_glfw.h"
#include "renderer/gui/imgui_opengl3.h"
#include "renderer/rendering/mvp_manager.h"
#include "renderer/rendering/renderer.h"
#include "renderer/window/window_manager.h"

ImGuiWindowFlags debug_window_flags = 0;
ImGuiWindowFlags release_window_flags = 0;


// Inventory
std::unordered_map<unsigned, jactorio::renderer::renderer_sprites::Image_position> inventory_sprite_positions;
// Assigned by openGL
unsigned int inventory_tex_id;

void jactorio::renderer::imgui_manager::setup_character_data() {
	inventory_sprite_positions = 
		renderer_sprites::get_spritemap(data::Sprite::sprite_group::gui).sprite_positions;
	inventory_tex_id = renderer_sprites::get_texture(data::Sprite::sprite_group::gui)->get_id();
}

void draw_inventory_menu() {
	ImGui::SetNextWindowPosCenter();

	const int inventory_size = 80;
	jactorio::data::item_stack* inventory = jactorio::game::player_manager::player_inventory;

	// TODO REMOVE | Test data
	{
		using namespace jactorio::data;
		auto x = data_manager::data_raw_get_all<Item>(data_category::item);
		inventory[0] = std::pair(x[0], 10);
		inventory[1] = std::pair(x[0], 8);
		inventory[4] = std::pair(x[0], 100);
		inventory[5] = std::pair(x[1], 2000);
	}
	
	ImGui::Begin("Character", nullptr, 
	             ImVec2(20 + 10 * 38, inventory_size / 10 * 38 + 80), 
	             -1, release_window_flags);
	
	bool quit = false;
	int y = 0;
	int inventory_drawn = 0;
	while (!quit) {
		for (int x = 0; x < 10; ++x) {
			const auto& item = inventory[y * 10 + x];

			ImGui::SameLine(10.f + x * 38);

			// Does the item exist? A count of 0 indicates it does not
			if (item.second != 0) {
				const auto& positions = inventory_sprite_positions[item.first->sprite->internal_id];
				ImGui::ImageButton(
					reinterpret_cast<void*>(inventory_tex_id),
					ImVec2(32, 32),

					ImVec2(positions.top_left.x, positions.top_left.y),
					ImVec2(positions.bottom_right.x, positions.bottom_right.y),
					2
				);

				// Tooltip
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(item.first->localized_name.c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				// Stack size
				ImGui::SameLine(10.f + x * 38);
				ImGui::Text("%d", item.second);
			}
			else {
				// Empty button
				ImGui::ImageButton(
					nullptr,
					ImVec2(0, 0),
					ImVec2(-1, -1),
					ImVec2(-1, -1),
					18 // 32 / 2 + 2
				);
			}

			// Break once the inventory has been drawn
			if (++inventory_drawn >= inventory_size) {
				quit = true;
				break;
			}
		}
		
		ImGui::NewLine();
		y++;
	}

	// int counter = 1;
	// for (auto& sprite_position : inventory_sprite_positions) {
	// 	ImGui::SameLine();
	// 	ImGui::ImageButton(
	// 		reinterpret_cast<void*>(inventory_tex_id),
	// 		ImVec2(32, 32),
	//
	// 		ImVec2(sprite_position.second.top_left.x, sprite_position.second.top_left.y),
	// 		ImVec2(sprite_position.second.bottom_right.x, sprite_position.second.bottom_right.y),
	// 		2
	// 	);
	//
	// 	if (counter % 10 == 0)
	// 		ImGui::NewLine();
	// 	counter++;
	// }
	
	ImGui::End();
}

//
bool show_timings_window = false;
bool show_demo_window = false;

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

void draw_debug_menu() {
	using namespace jactorio;
	
	ImGui::Begin("Debug menu", nullptr, debug_window_flags);
	// Settings
	glm::vec3* view_translation = renderer::mvp_manager::get_view_transform();
	ImGui::SliderFloat2("Camera translation", &view_translation->x, -100.0f,
	                    100.0f);

	ImGui::Text("Cursor position: %f, %f", 
	            game::mouse_selection::get_position_x(),
	            game::mouse_selection::get_position_y());

	ImGui::Text("Player position %f %f",
	            game::player_manager::get_player_position_x(),
	            game::player_manager::get_player_position_y());

	ImGui::NewLine();
	ImGui::Text("Layer count: %d", game::Chunk_tile::tile_prototypes_count);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// Window options
	ImGui::Checkbox("Timings", &show_timings_window); ImGui::SameLine();
	ImGui::Checkbox("Demo Window", &show_demo_window);

	// World gen seed
	int seed = game::world_generator::get_world_generator_seed();
	ImGui::InputInt("World generator seed", &seed);
	game::world_generator::set_world_generator_seed(seed);

	
	ImGui::End();
}

void draw_timings_menu() {
	using namespace jactorio::core;
	
	ImGui::Begin("Timings", nullptr, debug_window_flags);
	ImGui::Text("%fms (%.1f/s) Frame time", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
	for (auto& time : Execution_timer::measured_times) {
		ImGui::Text("%fms (%.1f/s) %s", time.second, 1000 / time.second, time.first.c_str());
	}
	ImGui::End();
}

void jactorio::renderer::imgui_manager::setup(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Disables imgui saving
	io.IniFilename = NULL;
	
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

	if (show_inventory_menu)
		draw_inventory_menu();
	
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

	LOG_MESSAGE(debug, "Imgui terminated");
}
