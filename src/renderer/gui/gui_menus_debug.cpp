#include "renderer/gui/gui_menus_debug.h"

#include <glm/glm.hpp>

#include "jactorio.h"

#include "data/data_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/transport_line_controller.h"
#include "game/player/player_manager.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_generator.h"
#include "game/world/world_manager.h"
#include "renderer/rendering/mvp_manager.h"

bool show_timings_window = false;
bool show_demo_window = false;
bool show_item_spawner_window = false;

// Game
bool show_belt_structure = false;

void jactorio::renderer::gui::debug_menu_logic() {
	if (show_belt_structure) {
		// Sprite representing the update point
		auto* sprite_stop =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-stop");

		auto* sprite_up =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-up");
		auto* sprite_right =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-right");
		auto* sprite_down =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-down");
		auto* sprite_left =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-left");

		// Get all update points and add it to the chunk's objects for drawing
		for (auto& pair : game::world_manager::logic_get_all_chunks()) {
			auto& l_chunk = pair.second;

			auto& object_layer = l_chunk.chunk->get_object(game::Chunk::objectLayer::debug_overlay);
			object_layer.clear();

			for (auto& l_struct : l_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line)) {
				auto* line_segment = static_cast<game::Transport_line_segment*>(l_struct.unique_data);

				// Correspond the direction with a sprite representing the direction
				switch (line_segment->direction) {
					default:
						assert(false);  // Missing case label

					case game::Transport_line_segment::moveDir::up:
						object_layer.emplace_back(sprite_up,
												  l_struct.position_x, l_struct.position_y,
												  1, line_segment->segment_length);
						break;
					case game::Transport_line_segment::moveDir::right:
						object_layer.emplace_back(sprite_right,
												  l_struct.position_x - line_segment->segment_length + 1, l_struct.position_y,
												  line_segment->segment_length, 1);
						break;
					case game::Transport_line_segment::moveDir::down:
						object_layer.emplace_back(sprite_down,
												  l_struct.position_x, l_struct.position_y - line_segment->segment_length + 1,
												  1, line_segment->segment_length);
						break;
					case game::Transport_line_segment::moveDir::left:
						object_layer.emplace_back(sprite_left,
												  l_struct.position_x, l_struct.position_y,
												  line_segment->segment_length, 1);
						break;
				}


			}
		}
	}

}

void jactorio::renderer::gui::debug_menu_main(const ImGuiWindowFlags window_flags) {
	using namespace jactorio;

	auto main_window_flags = window_flags;
	main_window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin("Debug menu", nullptr, main_window_flags);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (ImGui::CollapsingHeader("Rendering")) {
		glm::vec3* view_translation = mvp_manager::get_view_transform();
		ImGui::Text("Camera translation %f %f", view_translation->x, view_translation->y);

		ImGui::Text("Layer count | Tile: %d   Object: %d",
		            game::Chunk_tile::tile_layer_count, game::Chunk::object_layer_count);
	}

	if (ImGui::CollapsingHeader("Game")) {
		ImGui::Text("Cursor position: %f, %f",
					game::mouse_selection::get_position_x(),
					game::mouse_selection::get_position_y());

		ImGui::Text("Player position %f %f",
					game::player_manager::get_player_position_x(),
					game::player_manager::get_player_position_y());

		ImGui::Text("Chunk updates: %llu", game::world_manager::logic_get_all_chunks().size());


		int seed = game::world_generator::get_world_generator_seed();
		ImGui::InputInt("World generator seed", &seed);
		game::world_generator::set_world_generator_seed(seed);

		ImGui::Checkbox("Show belt structures", &show_belt_structure);
	}

	ImGui::Separator();

	// Window options	
	ImGui::Checkbox("Timings", &show_timings_window);
	ImGui::SameLine();
	ImGui::Checkbox("Demo Window", &show_demo_window);

	ImGui::Checkbox("Item spawner", &show_item_spawner_window);

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
				game::player_manager::inventory_player, game::player_manager::inventory_size, item_stack);
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::InputInt("Give amount", &give_amount);
	if (give_amount <= 0)
		give_amount = 1;

	if (ImGui::Button("Clear inventory")) {
		for (auto& i : game::player_manager::inventory_player) {
			i = {nullptr, 0};
		}
	}

	ImGui::End();
}
