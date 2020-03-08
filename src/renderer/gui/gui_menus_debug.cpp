// 
// gui_menus_debug.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/01/2020
// Last modified: 03/08/2020
// 

#include "renderer/gui/gui_menus_debug.h"

#include <glm/glm.hpp>

#include "jactorio.h"

#include "data/data_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/transport_line_structure.h"
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
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-red");
		auto* sprite_moving =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-green");
		auto* sprite_left_moving =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-aqua");
		auto* sprite_right_moving =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/rect-pink");

		auto* sprite_up =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/arrow-up");
		auto* sprite_right =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/arrow-right");
		auto* sprite_down =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/arrow-down");
		auto* sprite_left =
			data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, "__core__/arrow-left");

		// Get all update points and add it to the chunk's objects for drawing
		for (auto& pair : game::world_manager::logic_get_all_chunks()) {
			auto& l_chunk = pair.second;

			auto& object_layer = l_chunk.chunk->get_object(game::Chunk::objectLayer::debug_overlay);
			object_layer.clear();

			for (auto& l_struct : l_chunk.get_struct(game::Logic_chunk::structLayer::transport_line)) {
				auto* line_segment = static_cast<game::Transport_line_segment*>(l_struct.unique_data);

				float pos_x;
				float pos_y;
				float segment_len_x;
				float segment_len_y;

				data::Sprite* direction_sprite;
				data::Sprite* outline_sprite;

				// Correspond the direction with a sprite representing the direction
				switch (line_segment->direction) {
				default:
					assert(false);  // Missing case label

				case game::Transport_line_segment::moveDir::up:
					pos_x = l_struct.position_x;
					pos_y = l_struct.position_y;
					segment_len_x = 1;
					segment_len_y = line_segment->segment_length;

					direction_sprite = sprite_up;
					break;
				case game::Transport_line_segment::moveDir::right:
					pos_x = l_struct.position_x - line_segment->segment_length + 1;
					pos_y = l_struct.position_y;
					segment_len_x = line_segment->segment_length;
					segment_len_y = 1;

					direction_sprite = sprite_right;
					break;
				case game::Transport_line_segment::moveDir::down:
					pos_x = l_struct.position_x;
					pos_y = l_struct.position_y - line_segment->segment_length + 1;
					segment_len_x = 1;
					segment_len_y = line_segment->segment_length;

					direction_sprite = sprite_down;
					break;
				case game::Transport_line_segment::moveDir::left:
					pos_x = l_struct.position_x;
					pos_y = l_struct.position_y;
					segment_len_x = line_segment->segment_length;
					segment_len_y = 1;

					direction_sprite = sprite_left;
					break;
				}


				// Correspond a color of rectangle
				if (line_segment->is_active_left() && line_segment->is_active_right())
					outline_sprite = sprite_moving;  // Both moving
				else if (line_segment->is_active_left())
					outline_sprite = sprite_left_moving;  // Only left move
				else if (line_segment->is_active_right())
					outline_sprite = sprite_right_moving;  // Only right moving
				else
					outline_sprite = sprite_stop;  // None moving

				object_layer.emplace_back(direction_sprite, pos_x, pos_y, segment_len_x, segment_len_y);
				object_layer.emplace_back(outline_sprite, pos_x, pos_y, segment_len_x, segment_len_y);


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

		// Options
		ImGui::Checkbox("Show belt structures", &show_belt_structure);
		if (ImGui::Button("Make all belt items visible")) {
			for (auto& pair : game::world_manager::logic_get_all_chunks()) {
				auto& l_chunk = pair.second;
				for (auto& transport_line : l_chunk.get_struct(game::Logic_chunk::structLayer::transport_line)) {
					static_cast<game::Transport_line_segment*>(transport_line.unique_data)->item_visible = true;
				}
			}
		}
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
