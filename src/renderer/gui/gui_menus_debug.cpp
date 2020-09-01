// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/gui/gui_menus_debug.h"

#include <imgui.h>
#include <ostream>
#include <glm/glm.hpp>

#include "jactorio.h"

#include "data/prototype_manager.h"
#include "data/prototype/inserter.h"
#include "data/prototype/abstract_proto/transport_line.h"

#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/transport_segment.h"
#include "game/player/player_data.h"

#include "renderer/gui/gui_colors.h"
#include "renderer/gui/gui_menus.h"

using namespace jactorio;

bool show_timings_window      = false;
bool show_demo_window         = false;
bool show_item_spawner_window = false;

// Game
bool show_transport_line_info = false;
bool show_inserter_info       = false;

bool show_world_info = false;

void renderer::DebugMenuLogic(game::PlayerData& player_data, const data::PrototypeManager& data_manager) {
	if (show_transport_line_info)
		DebugTransportLineInfo(player_data, data_manager);

	if (show_inserter_info)
		DebugInserterInfo(player_data);

	if (show_demo_window)
		ImGui::ShowDemoWindow();

	if (show_timings_window)
		DebugTimings();

	if (show_item_spawner_window)
		DebugItemSpawner(player_data, data_manager);

	if (show_world_info) {
		DebugWorldInfo(player_data);
		DebugLogicInfo(player_data.GetPlayerLogicData());
	}
}

void renderer::DebugMenu(game::PlayerData& player_data, const data::PrototypeManager&,
                         const data::FrameworkBase*, data::UniqueDataBase*) {
	using namespace jactorio;

	ImGuiWindowFlags main_window_flags = 0;
	main_window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGuard guard{};
	guard.Begin("Debug menu", nullptr, main_window_flags);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
	            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (ImGui::CollapsingHeader("Rendering")) {
		// glm::vec3* view_translation = GetViewTransform();
		// ImGui::Text("Camera translation %f %f", view_translation->x, view_translation->y);

		ImGui::Text("Layer count | Tile: %d", game::ChunkTile::kTileLayerCount);

		if (ImGui::Button("Clear debug overlays")) {
			for (auto* chunk : player_data.GetPlayerWorldData().LogicGetChunks()) {

				auto& object_layer = chunk->GetOverlay(game::OverlayLayer::debug);
				object_layer.clear();
			}
		}
	}

	if (ImGui::CollapsingHeader("Game")) {
		auto& world_data = player_data.GetPlayerWorldData();

		ImGui::Text("Cursor position: %f, %f",
		            game::MouseSelection::GetCursorX(),
		            game::MouseSelection::GetCursorY());
		ImGui::Text("Cursor world position: %d, %d",
		            player_data.GetMouseTileCoords().x,
		            player_data.GetMouseTileCoords().y);

		ImGui::Text("Player position %f %f",
		            player_data.GetPlayerPositionX(),
		            player_data.GetPlayerPositionY());

		ImGui::Text("Game tick: %llu", player_data.GetPlayerLogicData().GameTick());
		ImGui::Text("Chunk updates: %llu", world_data.LogicGetChunks().size());

		ImGui::Separator();

		int seed = world_data.GetWorldGeneratorSeed();
		ImGui::InputInt("World generator seed", &seed);
		world_data.SetWorldGeneratorSeed(seed);

		// Options
		ImGui::Checkbox("Item spawner", &show_item_spawner_window);

		ImGui::Checkbox("Show transport line info", &show_transport_line_info);
		ImGui::Checkbox("Show inserter info", &show_inserter_info);

		ImGui::Checkbox("World info", &show_world_info);
	}

	ImGui::Separator();

	// Window options	
	ImGui::Checkbox("Timings", &show_timings_window);
	ImGui::SameLine();
	ImGui::Checkbox("Demo Window", &show_demo_window);
}

void renderer::DebugTimings() {
	using namespace core;

	ImGuard guard{};
	guard.Begin("Timings");
	ImGui::Text("%fms (%.1f/s) Frame time", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	for (auto& time : ExecutionTimer::measuredTimes) {
		ImGui::Text("%fms (%.1f/s) %s", time.second, 1000 / time.second, time.first.c_str());
	}
}

int give_amount  = 100;
int new_inv_size = game::PlayerData::kDefaultInventorySize;

void renderer::DebugItemSpawner(game::PlayerData& player_data, const data::PrototypeManager& data_manager) {
	using namespace core;

	ImGuard guard{};
	guard.Begin("Item spawner");

	ImGui::InputInt("Give amount", &give_amount);
	if (give_amount <= 0)
		give_amount = 1;

	if (ImGui::Button("Clear inventory")) {
		for (auto& i : player_data.inventoryPlayer) {
			i = {nullptr, 0};
		}
	}

	ImGui::InputInt("Inventory size", &new_inv_size);
	if (new_inv_size < 0)
		new_inv_size = 0;

	if (new_inv_size != player_data.inventoryPlayer.size()) {
		player_data.inventoryPlayer.resize(new_inv_size);
	}


	ImGui::Separator();


	auto game_items = data_manager.DataRawGetAll<data::Item>(data::DataCategory::item);
	for (auto& item : game_items) {
		ImGui::PushID(item->name.c_str());

		if (ImGui::Button(item->GetLocalizedName().c_str())) {
			data::ItemStack item_stack = {item, core::SafeCast<data::Item::StackCount>(give_amount)};
			game::AddStack(player_data.inventoryPlayer, item_stack);
		}
		ImGui::PopID();
	}
}

WorldCoord last_valid_line_segment{};
bool use_last_valid_line_segment = true;
bool show_transport_segments     = false;

void ShowTransportSegments(game::WorldData& world_data, const data::PrototypeManager& data_manager) {
	using namespace jactorio;

	constexpr game::OverlayLayer draw_overlay_layer = game::OverlayLayer::debug;

	// Sprite representing the update point
	const auto* sprite_stop =
		data_manager.DataRawGet<data::Sprite>("__core__/rect-red");
	const auto* sprite_moving =
		data_manager.DataRawGet<data::Sprite>("__core__/rect-green");
	const auto* sprite_left_moving =
		data_manager.DataRawGet<data::Sprite>("__core__/rect-aqua");
	const auto* sprite_right_moving =
		data_manager.DataRawGet<data::Sprite>("__core__/rect-pink");

	const auto* sprite_up =
		data_manager.DataRawGet<data::Sprite>("__core__/arrow-up");
	const auto* sprite_right =
		data_manager.DataRawGet<data::Sprite>("__core__/arrow-right");
	const auto* sprite_down =
		data_manager.DataRawGet<data::Sprite>("__core__/arrow-down");
	const auto* sprite_left =
		data_manager.DataRawGet<data::Sprite>("__core__/arrow-left");

	// Get all update points and add it to the chunk's objects for drawing
	for (auto* chunk : world_data.LogicGetChunks()) {
		auto& object_layer = chunk->GetOverlay(draw_overlay_layer);
		object_layer.clear();

		for (int i = 0; i < game::Chunk::kChunkArea; ++i) {
			auto& layer = chunk->Tiles()[i].GetLayer(game::TileLayer::entity);
			if (!layer.prototypeData.Get() || layer.prototypeData->Category() != data::DataCategory::transport_belt)
				continue;

			auto& line_data    = *static_cast<data::TransportLineData*>(layer.GetUniqueData());
			auto& line_segment = *line_data.lineSegment;

			// Only draw for the head of segments
			if (line_segment.terminationType == game::TransportSegment::TerminationType::straight &&
				line_data.lineSegmentIndex != 0)
				continue;

			if (line_segment.terminationType != game::TransportSegment::TerminationType::straight &&
				line_data.lineSegmentIndex != 1)
				continue;

			const auto position_x = i % game::Chunk::kChunkWidth;
			const auto position_y = i / game::Chunk::kChunkWidth;

			int pos_x;
			int pos_y;
			int segment_len_x;
			int segment_len_y;

			const data::Sprite* direction_sprite;
			const data::Sprite* outline_sprite;

			// Correspond the direction with a sprite representing the direction
			switch (line_segment.direction) {
			default:
				assert(false);  // Missing case label

			case data::Orientation::up:
				pos_x = position_x;
				pos_y         = position_y;
				segment_len_x = 1;
				segment_len_y = line_segment.length;

				direction_sprite = sprite_up;
				break;
			case data::Orientation::right:
				pos_x = position_x - line_segment.length + 1;
				pos_y         = position_y;
				segment_len_x = line_segment.length;
				segment_len_y = 1;

				direction_sprite = sprite_right;
				break;
			case data::Orientation::down:
				pos_x = position_x;
				pos_y         = position_y - line_segment.length + 1;
				segment_len_x = 1;
				segment_len_y = line_segment.length;

				direction_sprite = sprite_down;
				break;
			case data::Orientation::left:
				pos_x = position_x;
				pos_y         = position_y;
				segment_len_x = line_segment.length;
				segment_len_y = 1;

				direction_sprite = sprite_left;
				break;
			}

			// Shift items 1 tile forwards if segment bends
			if (line_segment.terminationType != game::TransportSegment::TerminationType::straight) {
				OrientationIncrement(line_segment.direction, pos_x, pos_y);
			}


			// Correspond a color of rectangle
			if (line_segment.left.IsActive() && line_segment.right.IsActive())
				outline_sprite = sprite_moving;  // Both moving
			else if (line_segment.left.IsActive())
				outline_sprite = sprite_left_moving;  // Only left move
			else if (line_segment.right.IsActive())
				outline_sprite = sprite_right_moving;  // Only right moving
			else
				outline_sprite = sprite_stop;  // None moving

			object_layer.emplace_back(
				game::OverlayElement{
					*direction_sprite,
					{core::SafeCast<float>(pos_x), core::SafeCast<float>(pos_y)},
					{core::SafeCast<float>(segment_len_x), core::SafeCast<float>(segment_len_y)},
					draw_overlay_layer
				}
			);
			object_layer.emplace_back(
				game::OverlayElement{
					*outline_sprite,
					{core::SafeCast<float>(pos_x), core::SafeCast<float>(pos_y)},
					{core::SafeCast<float>(segment_len_x), core::SafeCast<float>(segment_len_y)},
					draw_overlay_layer
				}
			);
		}
	}
}

void renderer::DebugTransportLineInfo(game::PlayerData& player_data, const data::PrototypeManager& data_manager) {
	ImGuard guard{};
	guard.Begin("Transport Line Info");

	const auto selected_tile      = player_data.GetMouseTileCoords();
	data::TransportLineData* data = data::TransportLine::GetLineData(player_data.GetPlayerWorldData(),
	                                                                 selected_tile.x, selected_tile.y);

	// Try to use current selected line segment first, otherwise used the last valid if checked
	game::TransportSegment* segment_ptr = nullptr;


	if (ImGui::Button("Make all belt items visible")) {
		for (auto* chunk : player_data.GetPlayerWorldData().LogicGetChunks()) {
			for (auto* transport_line : chunk->GetLogicGroup(game::Chunk::LogicGroup::transport_line)) {
				auto& segment         = *transport_line->GetUniqueData<data::TransportLineData>()->lineSegment;
				segment.left.visible  = true;
				segment.right.visible = true;
			}
		}
	}

	ImGui::Checkbox("Show transport line segments", &show_transport_segments);
	ImGui::Checkbox("Use last valid tile", &use_last_valid_line_segment);

	if (show_transport_segments)
		ShowTransportSegments(player_data.GetPlayerWorldData(), data_manager);

	if (data) {
		last_valid_line_segment = selected_tile;
		segment_ptr             = data->lineSegment.get();
	}
	else {
		if (use_last_valid_line_segment) {
			data = data::TransportLine::GetLineData(player_data.GetPlayerWorldData(),
			                                        last_valid_line_segment.x,
			                                        last_valid_line_segment.y);
			if (data)
				segment_ptr = data->lineSegment.get();
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
		ImGui::Text("Target insertion offset %d", segment.targetInsertOffset);
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

		ImGui::Text("Direction: %s", OrientationToStr(segment.direction));

		ImGui::Text("Item update index: %d %d", segment.left.index, segment.right.index);

		// Appending item
		const std::string iname = "__base__/wooden-chest-item";
		if (ImGui::Button("Append Item Left"))
			segment.AppendItem(true,
			                   0.2,
			                   *data_manager.DataRawGet<data::Item>(iname));

		if (ImGui::Button("Append Item Right"))
			segment.AppendItem(false,
			                   0.2,
			                   *data_manager.DataRawGet<data::Item>(iname));


		// Display items
		ImGui::Text("Left ----------");
		ImGui::Text("Status: %s", segment.left.IsActive() ? "Active" : "Stopped");
		for (auto& item : segment.left.lane) {
			ImGui::Text("%s %5.5f", item.item->name.c_str(), item.dist.getAsDouble());
		}

		ImGui::Separator();
		ImGui::Text("Right ----------");
		ImGui::Text("Status: %s", segment.right.IsActive() ? "Active" : "Stopped");
		for (auto& item : segment.right.lane) {
			ImGui::Text("%s %5.5f", item.item->name.c_str(), item.dist.getAsDouble());
		}

	}
}

void renderer::DebugInserterInfo(game::PlayerData& player_data) {
	ImGuard guard{};
	guard.Begin("Inserter info");

	const auto selected_tile = player_data.GetMouseTileCoords();

	auto* tile = player_data.GetPlayerWorldData().GetTile(selected_tile);
	if (!tile)
		return;

	auto& layer = tile->GetLayer(game::TileLayer::entity);
	if (!layer.prototypeData.Get() || layer.prototypeData->Category() != data::DataCategory::inserter) {
		ImGui::Text("No inserter at selected tile");
		return;
	}

	auto& inserter_data = *layer.GetUniqueData<data::InserterData>();

	ImGui::Text("Orientation %s", data::OrientationToStr(inserter_data.orientation));

	ImGui::Text("Degree: %f", inserter_data.rotationDegree.getAsDouble());

	switch (inserter_data.status) {
	case data::InserterData::Status::dropoff:
		ImGui::Text("Status: Dropoff");
		break;
	case data::InserterData::Status::pickup:
		ImGui::Text("Status: Pickup");
		break;
	}

	ImGui::Text("Pickup  %s", inserter_data.pickup.IsInitialized() ? "true" : "false");
	ImGui::Text("Dropoff %s", inserter_data.dropoff.IsInitialized() ? "true" : "false");
}

void renderer::DebugWorldInfo(const game::PlayerData& player_data) {
	ImGuard guard{};
	guard.Begin("World info");

	auto& world_data = player_data.GetPlayerWorldData();

	if (ImGui::CollapsingHeader("Update dispatchers")) {
		const auto dispatcher_info = world_data.updateDispatcher.GetDebugInfo();
		ImGui::Text("Update dispatchers: %lld", dispatcher_info.storedEntries.size());

		// Format of data displayed
		ImGui::Text("Registered coordinate > Listener coordinate | Listener prototype");

		size_t id = 0;
		for (const auto& entry : dispatcher_info.storedEntries) {

			const auto world_x = std::get<0>(entry.first);
			const auto world_y = std::get<1>(entry.first);

			if (ImGui::TreeNode(reinterpret_cast<void*>(id),
			                    "%d %d | %lld",
			                    world_x, world_y, entry.second.size())) {

				core::ResourceGuard<void> node_guard([]() { ImGui::TreePop(); });

				for (const auto& callback : entry.second) {
					std::ostringstream sstream;
					sstream << callback.callback.Get();  // Get pointer address
					ImGui::Text("%d %d %s", callback.receiver.x, callback.receiver.y, sstream.str().c_str());
				}
			}

			++id;
		}
	}

	if (ImGui::CollapsingHeader("Chunks")) {
		auto show_chunk_info = [](game::Chunk& chunk) {
			for (std::size_t i = 0; i < chunk.logicGroups.size(); ++i) {
				auto& logic_group = chunk.logicGroups[i];
				ImGui::Text("Logic group %d | Size: %lld", i, logic_group.size());
			}

			for (std::size_t i = 0; i < chunk.overlays.size(); ++i) {
				auto& overlay_group = chunk.overlays[i];
				ImGui::Text("Overlay group %d | Size: %lld", i, overlay_group.size());
			}
		};

		
		constexpr int chunk_radius = 3;  // Chunk radius around the player to display information for
		ImGui::Text("Radius of %d around the player", chunk_radius);

		const auto start_chunk_x = game::WorldData::WorldCToChunkC(player_data.GetPlayerPositionX());
		const auto start_chunk_y = game::WorldData::WorldCToChunkC(player_data.GetPlayerPositionY());

		for (auto chunk_y = start_chunk_y - chunk_radius; chunk_y < start_chunk_y + chunk_radius; ++chunk_y) {
			for (auto chunk_x = start_chunk_x - chunk_radius; chunk_x < start_chunk_x + chunk_radius; ++chunk_x) {
				auto* chunk = world_data.GetChunkC(chunk_x, chunk_y);

				if (chunk == nullptr)
					continue;

				// Unique id to identify tree node
				const auto* node_id = reinterpret_cast<void*>(core::LossyCast<uint64_t>(chunk_y) * chunk_radius * 2 + chunk_x);

				const bool is_player_chunk = chunk_x == start_chunk_x && chunk_y == start_chunk_y;

				if (ImGui::TreeNode(node_id, "%s %d %d",
				                    is_player_chunk ? ">" : " ", chunk_x, chunk_y)) {
					core::ResourceGuard<void> node_guard([]() { ImGui::TreePop(); });
					show_chunk_info(*chunk);
				}

			}
		}

	}
}

void renderer::DebugLogicInfo(const game::LogicData& logic_data) {
	ImGuard guard;
	guard.Begin("Logic info");

	const auto timer_info = logic_data.deferralTimer.GetDebugInfo();

	ImGui::Text("Deferral timers: %llu", timer_info.callbacks.size());
	ImGui::Text("Current game tick: %llu", logic_data.GameTick());

	// Format of data displayed
	ImGui::Text("Due game tick > Registered prototype");

	size_t id = 0;
	for (const auto& callback_tick : timer_info.callbacks) {

		const auto due_tick = callback_tick.first;

		assert(due_tick >= logic_data.GameTick());
		const auto time_to_due = due_tick - logic_data.GameTick();

		if (ImGui::TreeNode(reinterpret_cast<void*>(id),
		                    "%lld (T- %lld) | %lld",
		                    due_tick, time_to_due, callback_tick.second.size())) {
			core::ResourceGuard<void> node_guard([]() { ImGui::TreePop(); });

			for (const auto& callback : callback_tick.second) {
				std::ostringstream sstream;
				sstream << callback.uniqueData.Get();
				ImGui::Text("%s", sstream.str().c_str());
			}
		}

		++id;
	}
}
