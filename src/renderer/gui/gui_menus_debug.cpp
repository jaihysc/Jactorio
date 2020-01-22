#include "renderer/gui/gui_menus_debug.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "jactorio.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_generator.h"
#include "renderer/rendering/mvp_manager.h"
#include "data/data_manager.h"
#include "game/logic/inventory_controller.h"

bool show_timings_window = false;
bool show_demo_window = false;
bool show_item_spawner_window = false;

void jactorio::renderer::gui::debug_menu_main(const ImGuiWindowFlags window_flags) {
	using namespace jactorio;

	ImGui::Begin("Debug menu", nullptr, window_flags);
	// Settings
	glm::vec3* view_translation = mvp_manager::get_view_transform();
	ImGui::SliderFloat2("Camera translation", &view_translation->x, -100.0f,
	                    100.0f);

	ImGui::Text("Cursor position: %f, %f",
	            game::mouse_selection::get_position_x(),
	            game::mouse_selection::get_position_y());

	ImGui::Text("Player position %f %f",
	            game::player_manager::get_player_position_x(),
	            game::player_manager::get_player_position_y());

	ImGui::NewLine();
	ImGui::Text("Layer count: %d", game::Chunk_tile::layer_count);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// Window options	
	ImGui::Checkbox("Timings", &show_timings_window);
	ImGui::SameLine();
	ImGui::Checkbox("Demo Window", &show_demo_window);

	ImGui::Checkbox("Item spawner", &show_item_spawner_window);
	
	// World gen seed
	int seed = game::world_generator::get_world_generator_seed();
	ImGui::InputInt("World generator seed", &seed);
	game::world_generator::set_world_generator_seed(seed);


	ImGui::End();

	if (show_demo_window)
		ImGui::ShowDemoWindow();
	
	if (show_timings_window)
		debug_timings(window_flags);

	if (show_item_spawner_window)
		debug_item_spawner(window_flags);
}

void jactorio::renderer::gui::debug_timings(const ImGuiWindowFlags window_flags) {
	using namespace core;

	ImGui::Begin("Timings", nullptr, window_flags);
	ImGui::Text("%fms (%.1f/s) Frame time", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	for (auto& time : Execution_timer::measured_times) {
		ImGui::Text("%fms (%.1f/s) %s", time.second, 1000 / time.second, time.first.c_str());
	}
	ImGui::End();
}

int give_amount = 1;
void jactorio::renderer::gui::debug_item_spawner(const ImGuiWindowFlags window_flags) {
	using namespace core;

	ImGui::Begin("Item spawner", nullptr, window_flags);

	auto game_items = data::data_manager::data_raw_get_all<data::Item>(data::data_category::item);
	for (auto& item : game_items) {
		ImGui::PushID(item->name.c_str());
		if (ImGui::Button(item->get_localized_name().c_str())) {
			data::item_stack item_stack = {item, give_amount};
			game::inventory_c::add_itemstack_to_inv(
				game::player_manager::player_inventory, game::player_manager::player_inventory_size, item_stack);
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::InputInt("Give amount", &give_amount);
	if (ImGui::Button("Clear inventory")) {
		for (auto& i : game::player_manager::player_inventory) {
			i = {nullptr, 0};
		}
	}
	
	ImGui::End();
}
