// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "data/prototype/abstract_proto/entity.h"
#include "data/prototype/sprite.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

using namespace jactorio;

double x_position = 0.;
double y_position = 0.;

void game::SetCursorPosition(const double x_pos, const double y_pos) {
    x_position = x_pos;
    y_position = y_pos;
}


double game::MouseSelection::GetCursorX() {
    return x_position;
}

double game::MouseSelection::GetCursorY() {
    return y_position;
}


void game::MouseSelection::DrawCursorOverlay(GameWorlds& worlds,
                                             PlayerData& player_data,
                                             const data::PrototypeManager& proto_manager) {
    const auto cursor_position = player_data.world.GetMouseTileCoords();
    const auto* stack          = player_data.inventory.GetSelectedItem();

    const auto* sprite = proto_manager.DataRawGet<data::Sprite>(
        player_data.world.MouseSelectedTileInRange() ? "__core__/cursor-select" : "__core__/cursor-invalid");
    assert(sprite != nullptr);


    if (stack != nullptr) {
        DrawOverlay(worlds[player_data.world.GetId()],
                    cursor_position,
                    player_data.placement.orientation,
                    stack->item->entityPrototype,
                    *sprite);
    }
    else {
        DrawOverlay(
            worlds[player_data.world.GetId()], cursor_position, player_data.placement.orientation, nullptr, *sprite);
    }
}

void game::MouseSelection::DrawOverlay(WorldData& world,
                                       const WorldCoord& coord,
                                       const data::Orientation orientation,
                                       const data::Entity* selected_entity,
                                       const data::Sprite& cursor_sprite) {
    // Clear last overlay
    if (lastOverlayElementIndex_ != UINT64_MAX) {
        auto* last_chunk = world.GetChunkC(lastChunkPos_);
        assert(last_chunk);

        auto& overlay_layer = last_chunk->GetOverlay(kCursorOverlayLayer_);
        overlay_layer.erase(overlay_layer.begin() + lastOverlayElementIndex_);

        lastOverlayElementIndex_ = UINT64_MAX;
    }

    // Draw new overlay
    auto* chunk = world.GetChunkW(coord.x, coord.y);
    if (chunk == nullptr)
        return;

    auto& overlay_layer = chunk->GetOverlay(kCursorOverlayLayer_);

    auto* tile = world.GetTile(coord.x, coord.y);
    if (tile == nullptr)
        return;

    // Saves such that can be found and removed in the future
    auto save_overlay_info = [&]() {
        lastOverlayElementIndex_ = overlay_layer.size() - 1;
        lastChunkPos_            = {WorldData::WorldCToChunkC(coord.x), WorldData::WorldCToChunkC(coord.y)};
    };


    if (selected_entity != nullptr && selected_entity->placeable) {
        // Has item selected
        const auto set = selected_entity->OnRGetSpriteSet(orientation, world, {coord.x, coord.y});

        OverlayElement element{
            *selected_entity->OnRGetSprite(set),
            {WorldData::WorldCToOverlayC(coord.x), WorldData::WorldCToOverlayC(coord.y)},
            {core::SafeCast<float>(selected_entity->tileWidth), core::SafeCast<float>(selected_entity->tileHeight)},
            kCursorOverlayLayer_};

        element.spriteSet = set;

        overlay_layer.push_back(element);
        save_overlay_info();
    }
    else if (tile->GetLayer(TileLayer::entity).prototypeData.Get() != nullptr ||
             tile->GetLayer(TileLayer::resource).prototypeData.Get() != nullptr) {

        // Is hovering over entity
        overlay_layer.push_back({cursor_sprite,
                                 {WorldData::WorldCToOverlayC(coord.x), WorldData::WorldCToOverlayC(coord.y)},
                                 {1, 1},
                                 kCursorOverlayLayer_});
        save_overlay_info();
    }
}

void game::MouseSelection::SkipErasingLastOverlay() noexcept {
    lastOverlayElementIndex_ = UINT64_MAX;
}
