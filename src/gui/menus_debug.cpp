// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/menus_debug.h"

#include <imgui.h>
#include <ostream>

#include "jactorio.h"

#include "core/execution_timer.h"
#include "core/resource_guard.h"
#include "game/input/mouse_selection.h"
#include "game/logic/conveyor_utility.h"
#include "game/logic/logic.h"
#include "game/logistic/inventory.h"
#include "game/player/player.h"
#include "game/world/world.h"
#include "gui/colors.h"
#include "gui/context.h"
#include "gui/menus.h"
#include "proto/inserter.h"
#include "proto/label.h"
#include "proto/sprite.h"
#include "proto/transport_belt.h"
#include "render/renderer.h"

using namespace jactorio;

bool show_timings_window      = false;
bool show_demo_window         = false;
bool show_item_spawner_window = false;

// Game
bool show_tile_info     = false;
bool show_conveyor_info = false;
bool show_inserter_info = false;

bool show_world_info = false;

void gui::DebugMenuLogic(GameWorlds& worlds,
                         game::Logic& logic,
                         game::Player& player,
                         const data::PrototypeManager& proto,
                         render::Renderer& renderer) {
    if (show_tile_info)
        DebugTileInfo(worlds, player);

    if (show_conveyor_info)
        DebugConveyorInfo(worlds, player, proto, renderer);

    if (show_inserter_info)
        DebugInserterInfo(worlds, player);

    if (show_demo_window)
        ImGui::ShowDemoWindow();

    if (show_timings_window)
        DebugTimings();

    if (show_item_spawner_window)
        DebugItemSpawner(player, proto);

    if (show_world_info) {
        DebugWorldInfo(worlds, player);
        DebugLogicInfo(logic);
    }
}

std::string MemoryAddressToStr(const void* ptr) {
    std::ostringstream sstream;
    sstream << ptr;
    return sstream.str();
}

void gui::DebugMenu(const Context& context,
                    const proto::FrameworkBase* /*prototype*/,
                    proto::UniqueDataBase* /*unique_data*/) {
    auto& player = context.player;
    auto& world  = context.worlds[player.world.GetId()];
    auto& logic  = context.logic;

    ImGuiWindowFlags main_window_flags = 0;
    main_window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

    ImGuard guard;
    guard.Begin("Debug menu", nullptr, main_window_flags);

    ImGui::Text(
        "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (ImGui::CollapsingHeader("Rendering")) {
        // glm::vec3* view_translation = GetViewTransform();
        // ImGui::Text("Camera translation %f %f", view_translation->x, view_translation->y);

        ImGui::Text("Layer count | Tile: %d", game::kTileLayerCount);
    }

    if (ImGui::CollapsingHeader("Data")) {
        if (ImGui::Button("Mark localization labels")) {
            // Appends a ~ so localized text can be identified from hard coded text

            auto labels = context.proto.GetAll<proto::Label>();
            for (auto* label : labels) {
                label->SetLocalizedName(label->GetLocalizedName() + "~");
            }
        }
    }

    if (ImGui::CollapsingHeader("Game")) {
        ImGui::Text("Cursor position: %f, %f", game::MouseSelection::GetCursorX(), game::MouseSelection::GetCursorY());
        ImGui::Text(
            "Cursor world position: %d, %d", player.world.GetMouseTileCoords().x, player.world.GetMouseTileCoords().y);

        ImGui::Text("Player position %f %f", player.world.GetPositionX(), player.world.GetPositionY());

        ImGui::Text("Game tick: %llu", logic.GameTick());
        ImGui::Text("Chunk updates: %zu", world.LogicGetChunks().size());

        ImGui::Separator();

        int seed = world.GetWorldGeneratorSeed();
        ImGui::InputInt("World generator seed", &seed);
        world.SetWorldGeneratorSeed(seed);

        // Options
        ImGui::Checkbox("Item spawner", &show_item_spawner_window);

        ImGui::Checkbox("Show tile info", &show_tile_info);
        ImGui::Checkbox("Show conveyor info", &show_conveyor_info);
        ImGui::Checkbox("Show inserter info", &show_inserter_info);

        ImGui::Checkbox("World info", &show_world_info);
    }

    ImGui::Separator();

    // Window options
    ImGui::Checkbox("Timings", &show_timings_window);
    ImGui::SameLine();
    ImGui::Checkbox("Demo Window", &show_demo_window);
}

void gui::DebugTimings() {
    ImGuard guard;
    guard.Begin("Timings");
    ImGui::Text("%fms (%.1f/s) Frame time", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    for (auto& [key, value] : ExecutionTimer::measuredTimes) {
        ImGui::Text("%fms (%.1f/s) %s", value, 1000 / value, key.c_str());
    }
}

int give_amount  = 100;
int new_inv_size = game::Player::Inventory::kDefaultInventorySize;

void gui::DebugItemSpawner(game::Player& player, const data::PrototypeManager& proto) {
    ImGuard guard;
    guard.Begin("Item spawner");

    ImGui::InputInt("Give amount", &give_amount);
    if (give_amount <= 0)
        give_amount = 1;

    if (ImGui::Button("Clear inventory")) {
        for (auto& i : player.inventory.inventory) {
            i = {nullptr, 0};
        }
    }

    ImGui::InputInt("Inventory size", &new_inv_size);
    if (new_inv_size < 0)
        new_inv_size = 0;

    if (new_inv_size != player.inventory.inventory.Size()) {
        player.inventory.inventory.Resize(new_inv_size);
    }


    ImGui::Separator();


    auto game_items = proto.GetAll<proto::Item>();
    for (auto& item : game_items) {
        ImGui::PushID(item->name.c_str());

        if (ImGui::Button(item->GetLocalizedName().c_str())) {
            game::ItemStack item_stack = {item, SafeCast<proto::Item::StackCount>(give_amount)};
            player.inventory.inventory.Add(item_stack);
        }
        ImGui::PopID();
    }
}

void gui::DebugTileInfo(GameWorlds& worlds, game::Player& player) {
    auto& world = worlds[player.world.GetId()];

    ImGuard guard;
    guard.Begin("Tile info");

    ImGui::Text(
        "Cursor world position: %d, %d", player.world.GetMouseTileCoords().x, player.world.GetMouseTileCoords().y);

    for (int layer_index = 0; layer_index < game::kTileLayerCount; ++layer_index) {
        auto* tile = world.GetTile(player.world.GetMouseTileCoords(), static_cast<game::TileLayer>(layer_index));
        if (tile == nullptr) {
            ImGui::TextUnformatted("Layer null");
            continue;
        }

        ImGui::TextUnformatted("------------------------------------");
        ImGui::Text("Layer %d", layer_index);

        ImGui::Text("%s", tile->IsMultiTile() ? "Multi-tile" : "Non multi-tile");
        ImGui::Text("%s", tile->IsTopLeft() ? "Top left" : "Non top left");

        ImGui::Text("Multi-tile index: %d", tile->GetMultiTileIndex());

        ImGui::Text("Orientation: %s", tile->GetOrientation().ToCstr());
        ImGui::Text("Dimensions: %d, %d", tile->GetDimension().x, tile->GetDimension().y);

        ImGui::Text("Prototype: %s", MemoryAddressToStr(tile->GetPrototype()).c_str());
        ImGui::Text("Unique data: %s", MemoryAddressToStr(tile->GetUniqueData()).c_str());
    }
}

WorldCoord last_valid_line_segment{};
bool use_last_valid_line_segment = true;
bool show_conveyor_structs       = false;

static void ShowConveyorSegments(game::World& world,
                                 game::Player& player,
                                 const data::PrototypeManager& proto,
                                 render::Renderer& renderer) {
    const auto* sprite_stop         = proto.Get<proto::Sprite>("__core__/rect-red");
    const auto* sprite_moving       = proto.Get<proto::Sprite>("__core__/rect-green");
    const auto* sprite_left_moving  = proto.Get<proto::Sprite>("__core__/rect-aqua");
    const auto* sprite_right_moving = proto.Get<proto::Sprite>("__core__/rect-pink");

    const auto* sprite_up    = proto.Get<proto::Sprite>("__core__/arrow-up");
    const auto* sprite_right = proto.Get<proto::Sprite>("__core__/arrow-right");
    const auto* sprite_down  = proto.Get<proto::Sprite>("__core__/arrow-down");
    const auto* sprite_left  = proto.Get<proto::Sprite>("__core__/arrow-left");

    renderer.GlPrepareBegin();
    for (auto* chunk : world.LogicGetChunks()) {
        const auto tl_coord = game::World::ChunkCToWorldC(chunk->GetPosition());

        for (int i = 0; i < game::Chunk::kChunkArea; ++i) {
            auto& tile = chunk->Tiles(game::TileLayer::entity)[i];
            if (tile.GetPrototype() == nullptr || tile.GetPrototype()->GetCategory() != proto::Category::transport_belt)
                continue;

            auto& line_data    = *tile.GetUniqueData<proto::ConveyorData>();
            auto& line_segment = *line_data.structure;

            // Only draw for the head of segments
            if (line_segment.terminationType == game::ConveyorStruct::TerminationType::straight &&
                line_data.structIndex != 0)
                continue;

            if (line_segment.terminationType != game::ConveyorStruct::TerminationType::straight &&
                line_data.structIndex != 1)
                continue;

            const auto position_x = i % game::Chunk::kChunkWidth;
            const auto position_y = i / game::Chunk::kChunkWidth;

            auto coord = tl_coord;
            Position2<float> segment_len;
            const proto::Sprite* direction_sprite;
            switch (line_segment.direction) {
            case Orientation::up:
                coord.x += position_x;
                coord.y += position_y;
                segment_len.x = 1;
                segment_len.y = line_segment.length;

                direction_sprite = sprite_up;
                break;
            case Orientation::right:
                coord.x += position_x - line_segment.length + 1;
                coord.y += position_y;
                segment_len.x = line_segment.length;
                segment_len.y = 1;

                direction_sprite = sprite_right;
                break;
            case Orientation::down:
                coord.x += position_x;
                coord.y += position_y - line_segment.length + 1;
                segment_len.x = 1;
                segment_len.y = line_segment.length;

                direction_sprite = sprite_down;
                break;
            case Orientation::left:
                coord.x += position_x;
                coord.y += position_y;
                segment_len.x = line_segment.length;
                segment_len.y = 1;

                direction_sprite = sprite_left;
                break;

            default:
                assert(false); // Missing case label
            }

            // Shift items 1 tile forwards if segment bends
            if (line_segment.terminationType != game::ConveyorStruct::TerminationType::straight) {
                Position2Increment(line_segment.direction, coord, 1);
            }


            const proto::Sprite* outline_sprite;
            // Correspond a color of rectangle
            if (line_segment.left.IsActive() && line_segment.right.IsActive())
                outline_sprite = sprite_moving; // Both moving
            else if (line_segment.left.IsActive())
                outline_sprite = sprite_left_moving; // Only left move
            else if (line_segment.right.IsActive())
                outline_sprite = sprite_right_moving; // Only right moving
            else
                outline_sprite = sprite_stop; // None moving

            renderer.PrepareSprite(coord, player.world.GetPosition(), *direction_sprite, 0, segment_len);
            renderer.PrepareSprite(coord, player.world.GetPosition(), *outline_sprite, 0, segment_len);
        }
    }
    renderer.GlPrepareEnd();
}

void gui::DebugConveyorInfo(GameWorlds& worlds,
                            game::Player& player,
                            const data::PrototypeManager& proto,
                            render::Renderer& renderer) {
    auto& world = worlds[player.world.GetId()];

    ImGuard guard;
    guard.Begin("Conveyor Info");

    const auto selected_tile = player.world.GetMouseTileCoords();
    auto* con_data           = GetConData(world, {selected_tile.x, selected_tile.y});

    // Try to use current selected line segment first, otherwise used the last valid if checked
    game::ConveyorStruct* segment_ptr = nullptr;


    if (ImGui::Button("Make all belt items visible")) {
        for (auto* chunk : world.LogicGetChunks()) {
            for (auto* conveyor : chunk->GetLogicGroup(game::LogicGroup::conveyor)) {
                auto& segment         = *conveyor->GetUniqueData<proto::ConveyorData>()->structure;
                segment.left.visible  = true;
                segment.right.visible = true;
            }
        }
    }

    ImGui::Checkbox("Show conveyor segments", &show_conveyor_structs);
    ImGui::Checkbox("Use last valid tile", &use_last_valid_line_segment);

    if (show_conveyor_structs)
        ShowConveyorSegments(world, player, proto, renderer);

    if (con_data != nullptr) {
        last_valid_line_segment = selected_tile;
        segment_ptr             = con_data->structure.get();
    }
    else {
        if (use_last_valid_line_segment) {
            con_data = GetConData(world, {last_valid_line_segment.x, last_valid_line_segment.y});
            if (con_data != nullptr)
                segment_ptr = con_data->structure.get();
        }
    }

    if (segment_ptr == nullptr) {
        ImGui::Text("Selected tile is not a conveyor");
    }
    else {
        assert(con_data != nullptr);
        game::ConveyorStruct& segment = *segment_ptr;

        // Show conveyor properties
        ImGui::Text("Segment: %s", MemoryAddressToStr(segment_ptr).c_str());
        ImGui::Text("Target segment: %s",
                    segment.target != nullptr ? MemoryAddressToStr(segment.target).c_str() : "NULL");

        ImGui::Text("Head offset %d", segment.headOffset);
        ImGui::Text("Side insertion index %d", segment.sideInsertIndex);
        ImGui::Text("Length, Index: %d %d", segment.length, con_data->structIndex);

        {
            std::string s;
            switch (segment.terminationType) {
            case game::ConveyorStruct::TerminationType::straight:
                s = "Straight";
                break;
            case game::ConveyorStruct::TerminationType::bend_left:
                s = "Bend left";
                break;
            case game::ConveyorStruct::TerminationType::bend_right:
                s = "Bend right";
                break;
            case game::ConveyorStruct::TerminationType::left_only:
                s = "Left side";
                break;
            case game::ConveyorStruct::TerminationType::right_only:
                s = "Right side";
                break;
            default:
                assert(false); // Missing switch case
                break;
            }

            ImGui::Text("Termination Type: %s", s.c_str());
        }

        ImGui::Text("Direction: %s", segment.direction.ToCstr());

        ImGui::Text("Item update index: %d %d", segment.left.index, segment.right.index);

        // Appending item
        const std::string iname = "__base__/wooden-chest-item";
        if (ImGui::Button("Append Item Left"))
            segment.AppendItem(true, 0.2, *proto.Get<proto::Item>(iname));

        if (ImGui::Button("Append Item Right"))
            segment.AppendItem(false, 0.2, *proto.Get<proto::Item>(iname));


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

void gui::DebugInserterInfo(GameWorlds& worlds, game::Player& player) {
    auto& world = worlds[player.world.GetId()];

    ImGuard guard;
    guard.Begin("Inserter info");

    const auto selected_tile = player.world.GetMouseTileCoords();

    auto* tile = world.GetTile(selected_tile, game::TileLayer::entity);
    if (tile == nullptr)
        return;

    if (tile->GetPrototype() == nullptr || tile->GetPrototype()->GetCategory() != proto::Category::inserter) {
        ImGui::Text("No inserter at selected tile");
        return;
    }

    auto& inserter_data = *tile->GetUniqueData<proto::InserterData>();

    ImGui::Text("Orientation %s", inserter_data.orientation.ToCstr());

    ImGui::Text("Degree: %f", inserter_data.rotationDegree.getAsDouble());

    switch (inserter_data.status) {
    case proto::InserterData::Status::dropoff:
        ImGui::Text("Status: Dropoff");
        break;
    case proto::InserterData::Status::pickup:
        ImGui::Text("Status: Pickup");
        break;
    }

    ImGui::Text("Pickup  %s", inserter_data.pickup.IsInitialized() ? "true" : "false");
    ImGui::Text("Dropoff %s", inserter_data.dropoff.IsInitialized() ? "true" : "false");
}

void gui::DebugWorldInfo(GameWorlds& worlds, const game::Player& player) {
    auto& world = worlds[player.world.GetId()];

    ImGuard guard;
    guard.Begin("World info");

    if (ImGui::CollapsingHeader("Update dispatchers")) {
        const auto dispatcher_info = world.updateDispatcher.GetDebugInfo();
        ImGui::Text("Update dispatchers: %zu", dispatcher_info.storedEntries.size());

        // Format of data displayed
        ImGui::Text("Registered coordinate > Listener coordinate | Listener prototype");

        size_t id = 0;
        for (const auto& entry : dispatcher_info.storedEntries) {

            const auto world_x = std::get<0>(entry.first);
            const auto world_y = std::get<1>(entry.first);

            if (ImGui::TreeNode(reinterpret_cast<void*>(id), "%d %d | %lld", world_x, world_y, entry.second.size())) {

                ResourceGuard<void> node_guard([]() { ImGui::TreePop(); });

                for (const auto& callback : entry.second) {
                    ImGui::Text("%d %d %s",
                                callback.receiver.x,
                                callback.receiver.y,
                                MemoryAddressToStr(callback.callback.Get()).c_str());
                }
            }

            ++id;
        }
    }

    if (ImGui::CollapsingHeader("Chunks")) {
        auto show_chunk_info = [](game::Chunk& chunk) {
            for (std::size_t i = 0; i < chunk.logicGroups.size(); ++i) {
                auto& logic_group = chunk.logicGroups[i];
                ImGui::Text("Logic group %zu | Size: %zu", i, logic_group.size());
            }

            for (std::size_t i = 0; i < chunk.overlays.size(); ++i) {
                auto& overlay_group = chunk.overlays[i];
                ImGui::Text("Overlay group %zu | Size: %zu", i, overlay_group.size());
            }
        };


        constexpr int chunk_radius = 3; // Chunk radius around the player to display information for
        ImGui::Text("Radius of %d around the player", chunk_radius);

        const auto start_chunk_x = game::World::WorldCToChunkC(player.world.GetPositionX());
        const auto start_chunk_y = game::World::WorldCToChunkC(player.world.GetPositionY());

        for (auto chunk_y = start_chunk_y - chunk_radius; chunk_y < start_chunk_y + chunk_radius; ++chunk_y) {
            for (auto chunk_x = start_chunk_x - chunk_radius; chunk_x < start_chunk_x + chunk_radius; ++chunk_x) {
                auto* chunk = world.GetChunkC({chunk_x, chunk_y});

                if (chunk == nullptr)
                    continue;

                // Unique id to identify tree node
                const auto* node_id =
                    reinterpret_cast<void*>(LossyCast<uint64_t>(chunk_y) * chunk_radius * 2 + chunk_x);

                const bool is_player_chunk = chunk_x == start_chunk_x && chunk_y == start_chunk_y;

                if (ImGui::TreeNode(node_id, "%s %d %d", is_player_chunk ? ">" : " ", chunk_x, chunk_y)) {
                    ResourceGuard<void> node_guard([]() { ImGui::TreePop(); });
                    show_chunk_info(*chunk);
                }
            }
        }
    }
}

void gui::DebugLogicInfo(const game::Logic& logic) {
    ImGuard guard;
    guard.Begin("Logic info");

    const auto timer_info = logic.deferralTimer.GetDebugInfo();

    ImGui::Text("Deferral timers: %zu", timer_info.callbacks.size());
    ImGui::Text("Current game tick: %llu", logic.GameTick());

    // Format of data displayed
    ImGui::Text("Due game tick > Registered prototype");

    size_t id = 0;
    for (const auto& [due_tick, callbacks] : timer_info.callbacks) {
        assert(due_tick >= logic.GameTick());
        const auto time_to_due = due_tick - logic.GameTick();

        if (ImGui::TreeNode(
                reinterpret_cast<void*>(id), "%llu (T- %lld) | %zu", due_tick, time_to_due, callbacks.size())) {
            ResourceGuard<void> node_guard([]() { ImGui::TreePop(); });

            for (const auto& callback : callbacks) {
                ImGui::Text("%s", MemoryAddressToStr(callback.uniqueData.Get()).c_str());
            }
        }

        ++id;
    }
}
