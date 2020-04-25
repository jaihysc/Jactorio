// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#include "renderer/gui/gui_menus_debug.h"

#include <ostream>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include "jactorio.h"

#include "data/data_manager.h"
#include "data/prototype/entity/transport/transport_line.h"
#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/transport_line_structure.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/rendering/mvp_manager.h"

bool show_timings_window = false;
bool show_demo_window = false;
bool show_item_spawner_window = false;

// Game
bool show_transport_line_info = false;

void jactorio::renderer::gui::debug_menu_logic(game::Player_data& player_data) {
	auto& world_data = player_data.get_player_world();

	if (show_transport_line_info) {
		debug_transport_line_info(player_data);

		// Sprite representing the update point
		auto* sprite_stop =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/rect-red");
		auto* sprite_moving =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/rect-green");
		auto* sprite_left_moving =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/rect-aqua");
		auto* sprite_right_moving =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/rect-pink");

		auto* sprite_up =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/arrow-up");
		auto* sprite_right =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/arrow-right");
		auto* sprite_down =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/arrow-down");
		auto* sprite_left =
			data::data_raw_get<data::Sprite>(data::dataCategory::sprite, "__core__/arrow-left");

		// Get all update points and add it to the chunk's objects for drawing
		for (auto& pair : world_data.logic_get_all_chunks()) {
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

	if (show_demo_window)
		ImGui::ShowDemoWindow();

	if (show_timings_window)
		debug_timings();

	if (show_item_spawner_window)
		debug_item_spawner(player_data);
}

void jactorio::renderer::gui::debug_menu(game::Player_data& player_data, const data::Unique_data_base*) {
	using namespace jactorio;

	ImGuiWindowFlags main_window_flags = 0;
	main_window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::Begin("Debug menu", nullptr, main_window_flags);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (ImGui::CollapsingHeader("Rendering")) {
		glm::vec3* view_translation = get_view_transform();
		ImGui::Text("Camera translation %f %f", view_translation->x, view_translation->y);

		ImGui::Text("Layer count | Tile: %d   Object: %d",
		            game::Chunk_tile::tile_layer_count, game::Chunk::object_layer_count);
	}

	if (ImGui::CollapsingHeader("Game")) {
		auto& world_data = player_data.get_player_world();

		ImGui::Text("Cursor position: %f, %f",
		            game::Mouse_selection::get_cursor_x(),
		            game::Mouse_selection::get_cursor_y());
		ImGui::Text("Cursor world position: %d, %d",
		            player_data.get_mouse_tile_coords().first,
		            player_data.get_mouse_tile_coords().second);

		ImGui::Text("Player position %f %f",
		            player_data.get_player_position_x(),
		            player_data.get_player_position_y());

		ImGui::Text("Game tick: %llu", world_data.game_tick());
		ImGui::Text("Chunk updates: %llu", world_data.logic_get_all_chunks().size());

		ImGui::Separator();

		int seed = world_data.get_world_generator_seed();
		ImGui::InputInt("World generator seed", &seed);
		world_data.set_world_generator_seed(seed);

		// Options
		ImGui::Checkbox("Item spawner", &show_item_spawner_window);

		ImGui::Checkbox("Show transport line info", &show_transport_line_info);

		if (ImGui::Button("Make all belt items visible")) {
			for (auto& pair : world_data.logic_get_all_chunks()) {
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


	ImGui::End();
}

void jactorio::renderer::gui::debug_timings() {
	using namespace core;

	ImGui::Begin("Timings");
	ImGui::Text("%fms (%.1f/s) Frame time", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	for (auto& time : Execution_timer::measured_times) {
		ImGui::Text("%fms (%.1f/s) %s", time.second, 1000 / time.second, time.first.c_str());
	}
	ImGui::End();
}

int give_amount = 100;

void jactorio::renderer::gui::debug_item_spawner(game::Player_data& player_data) {
	using namespace core;

	ImGui::Begin("Item spawner");

	auto game_items = data::data_raw_get_all<data::Item>(data::dataCategory::item);
	for (auto& item : game_items) {
		ImGui::PushID(item->name.c_str());

		if (ImGui::Button(item->get_localized_name().c_str())) {
			data::item_stack item_stack = {item, give_amount};
			game::add_stack(
				player_data.inventory_player, game::Player_data::inventory_size, item_stack);
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::InputInt("Give amount", &give_amount);
	if (give_amount <= 0)
		give_amount = 1;

	if (ImGui::Button("Clear inventory")) {
		for (auto& i : player_data.inventory_player) {
			i = {nullptr, 0};
		}
	}

	ImGui::End();
}

std::pair<int32_t, int32_t> last_valid_line_segment{};
bool use_last_valid_line_segment = false;

void jactorio::renderer::gui::debug_transport_line_info(game::Player_data& player_data) {
	ImGui::Begin("Transport Line Info");

	const auto selected_tile = player_data.get_mouse_tile_coords();
	data::Transport_line_data* data = data::Transport_line::get_line_data(player_data.get_player_world(),
	                                                                      selected_tile.first, selected_tile.second);

	// Try to use current selected line segment first, otherwise used the last valid if checked
	game::Transport_line_segment* segment_ptr = nullptr;

	ImGui::Checkbox("Use last valid tile", &use_last_valid_line_segment);
	if (data) {
		last_valid_line_segment = selected_tile;
		segment_ptr = &data->line_segment;
	}
	else {
		if (use_last_valid_line_segment) {
			data::Transport_line_data* data =
				data::Transport_line::get_line_data(player_data.get_player_world(),
				                                    last_valid_line_segment.first,
				                                    last_valid_line_segment.second);
			if (data)
				segment_ptr = &data->line_segment;
		}
	}

	if (!segment_ptr) {
		ImGui::Text("Selected tile is not a transport line");
	}
	else {
		game::Transport_line_segment& segment = *segment_ptr;

		// Show transport line properties
		// Show memory addresses
		{
			std::ostringstream sstream;
			sstream << segment_ptr;
			ImGui::Text("Segment: %s", sstream.str().c_str());


			std::ostringstream sstream2;
			sstream2 << segment.target_segment;
			ImGui::Text("Target segment: %s", segment.target_segment ? sstream2.str().c_str() : "NULL");
		}

		ImGui::Text("Segment length: %d", segment.segment_length);

		{
			std::string s;
			switch (segment.termination_type) {
			case game::Transport_line_segment::terminationType::straight:
				s = "Straight";
				break;
			case game::Transport_line_segment::terminationType::bend_left:
				s = "Bend left";
				break;
			case game::Transport_line_segment::terminationType::bend_right:
				s = "Bend right";
				break;
			case game::Transport_line_segment::terminationType::left_only:
				s = "Left side";
				break;
			case game::Transport_line_segment::terminationType::right_only:
				s = "Right side";
				break;
			default:
				assert(false);  // Missing switch case
				break;
			}

			ImGui::Text("Termination Type: %s", s.c_str());
		}
		{
			std::string s;
			switch (segment.direction) {
			case game::Transport_line_segment::moveDir::up:
				s = "Up";
				break;
			case game::Transport_line_segment::moveDir::right:
				s = "Right";
				break;
			case game::Transport_line_segment::moveDir::down:
				s = "Down";
				break;
			case game::Transport_line_segment::moveDir::left:
				s = "Left";
				break;
			default:
				assert(false);  // Missing switch case
				break;
			}

			ImGui::Text("Direction: %s", s.c_str());
		}

		// Appending item
		const std::string iname = "__base__/wooden-chest-item";
		if (ImGui::Button("Append Item Left"))
			segment.append_item(true,
			                    0.2,
			                    data::data_raw_get<data::Item>(data::dataCategory::item, iname));

		if (ImGui::Button("Append Item Right"))
			segment.append_item(false,
			                    0.2,
			                    data::data_raw_get<data::Item>(data::dataCategory::item, iname));


		// Display items
		ImGui::Text("Left ----------");
		ImGui::Text("Status: %s", segment.is_active_left() ? "Active" : "Stopped");
		for (auto& item : segment.left) {
			ImGui::Text("%s %5.5f", item.second->name.c_str(), item.first.getAsDouble());
		}

		ImGui::Separator();
		ImGui::Text("Right ----------");
		ImGui::Text("Status: %s", segment.is_active_right() ? "Active" : "Stopped");
		for (auto& item : segment.right) {
			ImGui::Text("%s %5.5f", item.second->name.c_str(), item.first.getAsDouble());
		}

	}

	ImGui::End();
}
