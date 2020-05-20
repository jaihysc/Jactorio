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
#include "game/logic/transport_segment.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile.h"
#include "renderer/gui/gui_menus.h"
#include "renderer/rendering/mvp_manager.h"

bool show_timings_window      = false;
bool show_demo_window         = false;
bool show_item_spawner_window = false;

// Game
bool show_transport_line_info     = false;

void jactorio::renderer::DebugMenuLogic(game::PlayerData& player_data) {
	if (show_transport_line_info)
		DebugTransportLineInfo(player_data);

	if (show_demo_window)
		ImGui::ShowDemoWindow();

	if (show_timings_window)
		DebugTimings();

	if (show_item_spawner_window)
		DebugItemSpawner(player_data);
}

void jactorio::renderer::DebugMenu(game::PlayerData& player_data, const data::UniqueDataBase*) {
	using namespace jactorio;

	ImGuiWindowFlags main_window_flags = 0;
	main_window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::Begin("Debug menu", nullptr, main_window_flags);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (ImGui::CollapsingHeader("Rendering")) {
		glm::vec3* view_translation = GetViewTransform();
		ImGui::Text("Camera translation %f %f", view_translation->x, view_translation->y);

		ImGui::Text("Layer count | Tile: %d   Object: %d",
		            game::ChunkTile::kTileLayerCount, game::Chunk::kObjectLayerCount);

		if (ImGui::Button("Clear debug overlays")) {
			for (auto& pair : player_data.GetPlayerWorld().LogicGetAllChunks()) {
				auto& l_chunk = pair.second;

				auto& object_layer = l_chunk.chunk->GetObject(game::Chunk::ObjectLayer::debug_overlay);
				object_layer.clear();
			}
		}
	}

	if (ImGui::CollapsingHeader("Game")) {
		auto& world_data = player_data.GetPlayerWorld();

		ImGui::Text("Cursor position: %f, %f",
		            game::MouseSelection::GetCursorX(),
		            game::MouseSelection::GetCursorY());
		ImGui::Text("Cursor world position: %d, %d",
		            player_data.GetMouseTileCoords().first,
		            player_data.GetMouseTileCoords().second);

		ImGui::Text("Player position %f %f",
		            player_data.GetPlayerPositionX(),
		            player_data.GetPlayerPositionY());

		ImGui::Text("Game tick: %llu", world_data.GameTick());
		ImGui::Text("Chunk updates: %llu", world_data.LogicGetAllChunks().size());

		ImGui::Separator();

		int seed = world_data.GetWorldGeneratorSeed();
		ImGui::InputInt("World generator seed", &seed);
		world_data.SetWorldGeneratorSeed(seed);

		// Options
		ImGui::Checkbox("Item spawner", &show_item_spawner_window);

		ImGui::Checkbox("Show transport line info", &show_transport_line_info);

		if (ImGui::Button("Make all belt items visible")) {
			for (auto& pair : world_data.LogicGetAllChunks()) {
				auto& l_chunk = pair.second;
				for (auto& transport_line : l_chunk.GetStruct(game::LogicChunk::StructLayer::transport_line)) {
					auto* segment = static_cast<game::TransportSegment*>(transport_line.uniqueData);
					segment->left.visible = true;
					segment->right.visible = true;
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

void jactorio::renderer::DebugTimings() {
	using namespace core;

	ImGui::Begin("Timings");
	ImGui::Text("%fms (%.1f/s) Frame time", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	for (auto& time : ExecutionTimer::measuredTimes) {
		ImGui::Text("%fms (%.1f/s) %s", time.second, 1000 / time.second, time.first.c_str());
	}
	ImGui::End();
}

int give_amount = 100;

void jactorio::renderer::DebugItemSpawner(game::PlayerData& player_data) {
	using namespace core;

	ImGui::Begin("Item spawner");

	auto game_items = data::DataRawGetAll<data::Item>(data::DataCategory::item);
	for (auto& item : game_items) {
		ImGui::PushID(item->name.c_str());

		if (ImGui::Button(item->GetLocalizedName().c_str())) {
			data::ItemStack item_stack = {item, give_amount};
			game::AddStack(
				player_data.inventoryPlayer, game::PlayerData::kInventorySize, item_stack);
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::InputInt("Give amount", &give_amount);
	if (give_amount <= 0)
		give_amount = 1;

	if (ImGui::Button("Clear inventory")) {
		for (auto& i : player_data.inventoryPlayer) {
			i = {nullptr, 0};
		}
	}

	ImGui::End();
}

std::pair<int32_t, int32_t> last_valid_line_segment{};
bool use_last_valid_line_segment = true;
bool show_transport_segments = false;

void ShowTransportSegments(jactorio::game::WorldData& world_data) {
	using namespace jactorio;

	// Sprite representing the update point
	auto* sprite_stop =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/rect-red");
	auto* sprite_moving =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/rect-green");
	auto* sprite_left_moving =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/rect-aqua");
	auto* sprite_right_moving =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/rect-pink");

	auto* sprite_up =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/arrow-up");
	auto* sprite_right =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/arrow-right");
	auto* sprite_down =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/arrow-down");
	auto* sprite_left =
		data::DataRawGet<data::Sprite>(data::DataCategory::sprite, "__core__/arrow-left");

	// Get all update points and add it to the chunk's objects for drawing
	for (auto& pair : world_data.LogicGetAllChunks()) {
		auto& l_chunk = pair.second;

		auto& object_layer = l_chunk.chunk->GetObject(game::Chunk::ObjectLayer::debug_overlay);
		object_layer.clear();

		for (auto& l_struct : l_chunk.GetStruct(game::LogicChunk::StructLayer::transport_line)) {
			auto* line_segment = static_cast<game::TransportSegment*>(l_struct.uniqueData);

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

			case data::Orientation::up:
				pos_x = l_struct.positionX;
				pos_y         = l_struct.positionY;
				segment_len_x = 1;
				segment_len_y = line_segment->length;

				direction_sprite = sprite_up;
				break;
			case data::Orientation::right:
				pos_x = l_struct.positionX - line_segment->length + 1;
				pos_y         = l_struct.positionY;
				segment_len_x = line_segment->length;
				segment_len_y = 1;

				direction_sprite = sprite_right;
				break;
			case data::Orientation::down:
				pos_x = l_struct.positionX;
				pos_y         = l_struct.positionY - line_segment->length + 1;
				segment_len_x = 1;
				segment_len_y = line_segment->length;

				direction_sprite = sprite_down;
				break;
			case data::Orientation::left:
				pos_x = l_struct.positionX;
				pos_y         = l_struct.positionY;
				segment_len_x = line_segment->length;
				segment_len_y = 1;

				direction_sprite = sprite_left;
				break;
			}


			// Correspond a color of rectangle
			if (line_segment->left.IsActive() && line_segment->right.IsActive())
				outline_sprite = sprite_moving;  // Both moving
			else if (line_segment->left.IsActive())
				outline_sprite = sprite_left_moving;  // Only left move
			else if (line_segment->right.IsActive())
				outline_sprite = sprite_right_moving;  // Only right moving
			else
				outline_sprite = sprite_stop;  // None moving

			object_layer.emplace_back(direction_sprite, pos_x, pos_y, segment_len_x, segment_len_y);
			object_layer.emplace_back(outline_sprite, pos_x, pos_y, segment_len_x, segment_len_y);
		}
	}
}

void jactorio::renderer::DebugTransportLineInfo(game::PlayerData& player_data) {
	ImGui::Begin("Transport Line Info");

	const auto selected_tile      = player_data.GetMouseTileCoords();
	data::TransportLineData* data = data::TransportLine::GetLineData(player_data.GetPlayerWorld(),
	                                                                 selected_tile.first, selected_tile.second);

	// Try to use current selected line segment first, otherwise used the last valid if checked
	game::TransportSegment* segment_ptr = nullptr;

	ImGui::Checkbox("Show transport line segments", &show_transport_segments);
	ImGui::Checkbox("Use last valid tile", &use_last_valid_line_segment);

	if (show_transport_segments)
		ShowTransportSegments(player_data.GetPlayerWorld());

	if (data) {
		last_valid_line_segment = selected_tile;
		segment_ptr             = &data->lineSegment.get();
	}
	else {
		if (use_last_valid_line_segment) {
			data = data::TransportLine::GetLineData(player_data.GetPlayerWorld(),
			                                        last_valid_line_segment.first,
			                                        last_valid_line_segment.second);
			if (data)
				segment_ptr = &data->lineSegment.get();
		}
	}

	if (!segment_ptr) {
		ImGui::Text("Selected tile is not a transport line");
	}
	else {
		assert(data != nullptr);
		game::TransportSegment& segment = *segment_ptr;

		// Show transport line properties
		// Show memory addresses
		{
			std::ostringstream sstream;
			sstream << segment_ptr;
			ImGui::Text("Segment: %s", sstream.str().c_str());


			std::ostringstream sstream2;
			sstream2 << segment.targetSegment;
			ImGui::Text("Target segment: %s", segment.targetSegment ? sstream2.str().c_str() : "NULL");
		}

		ImGui::Text("Item offset %d", segment.itemOffset);
		ImGui::Text("Target insertion offset %f", segment.targetInsertOffset);
		ImGui::Text("Length, Index: %d %d", segment.length, data->lineSegmentIndex);

		{
			std::string s;
			switch (segment.terminationType) {
			case game::TransportSegment::TerminationType::straight:
				s = "Straight";
				break;
			case game::TransportSegment::TerminationType::bend_left:
				s = "Bend left";
				break;
			case game::TransportSegment::TerminationType::bend_right:
				s = "Bend right";
				break;
			case game::TransportSegment::TerminationType::left_only:
				s = "Left side";
				break;
			case game::TransportSegment::TerminationType::right_only:
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
			case data::Orientation::up:
				s = "Up";
				break;
			case data::Orientation::right:
				s = "Right";
				break;
			case data::Orientation::down:
				s = "Down";
				break;
			case data::Orientation::left:
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
			segment.AppendItem(true,
			                   0.2,
			                   data::DataRawGet<data::Item>(data::DataCategory::item, iname));

		if (ImGui::Button("Append Item Right"))
			segment.AppendItem(false,
			                   0.2,
			                   data::DataRawGet<data::Item>(data::DataCategory::item, iname));


		// Display items
		ImGui::Text("Left ----------");
		ImGui::Text("Status: %s", segment.left.IsActive() ? "Active" : "Stopped");
		for (auto& item : segment.left.lane) {
			ImGui::Text("%s %5.5f", item.second->name.c_str(), item.first.getAsDouble());
		}

		ImGui::Separator();
		ImGui::Text("Right ----------");
		ImGui::Text("Status: %s", segment.right.IsActive() ? "Active" : "Stopped");
		for (auto& item : segment.right.lane) {
			ImGui::Text("%s %5.5f", item.second->name.c_str(), item.first.getAsDouble());
		}

	}

	ImGui::End();
}
