// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "game/player/player.h"
#include "game/world/world.h"
#include "proto/abstract/entity.h"
#include "proto/sprite.h"

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


void game::MouseSelection::DrawCursorOverlay(GameWorlds& worlds, Player& player, const data::PrototypeManager& proto) {
    const auto cursor_position = player.world.GetMouseTileCoords();
    const auto* stack          = player.inventory.GetSelectedItem();

    const auto* sprite = proto.Get<proto::Sprite>(player.world.MouseSelectedTileInRange() ? "__core__/cursor-select"
                                                                                          : "__core__/cursor-invalid");
    assert(sprite != nullptr);


    if (stack != nullptr) {
        DrawOverlay(worlds[player.world.GetId()],
                    cursor_position,
                    player.placement.orientation,
                    stack->item->entityPrototype,
                    *sprite);
    }
    else {
        DrawOverlay(worlds[player.world.GetId()], cursor_position, player.placement.orientation, nullptr, *sprite);
    }
}

void game::MouseSelection::DrawOverlay(World& world,
                                       const WorldCoord& coord,
                                       const Orientation orientation,
                                       const proto::Entity* selected_entity,
                                       const proto::Sprite& cursor_sprite) {
    // Clear last overlay
    if (lastOverlayElementIndex_ != UINT64_MAX) {
        auto* last_chunk = world.GetChunkC(lastChunkPos_);
        assert(last_chunk);

        auto& overlay_layer = last_chunk->GetOverlay(kCursorOverlayLayer_);
        overlay_layer.erase(overlay_layer.begin() + lastOverlayElementIndex_);

        lastOverlayElementIndex_ = UINT64_MAX;
    }

    // Draw new overlay
    auto* chunk = world.GetChunkW(coord);
    if (chunk == nullptr)
        return;

    auto& overlay_layer = chunk->GetOverlay(kCursorOverlayLayer_);


    // Saves such that can be found and removed in the future
    auto save_overlay_info = [&]() {
        lastOverlayElementIndex_ = overlay_layer.size() - 1;
        lastChunkPos_            = {World::WorldCToChunkC(coord.x), World::WorldCToChunkC(coord.y)};
    };


    if (selected_entity != nullptr && selected_entity->placeable) {
        // Has item selected
        const auto set = selected_entity->OnRGetSpriteSet(orientation, world, {coord.x, coord.y});

        OverlayElement element{*selected_entity->OnRGetSprite(set),
                               {World::WorldCToOverlayC(coord.x), World::WorldCToOverlayC(coord.y)},
                               {SafeCast<float>(selected_entity->GetWidth(orientation)),
                                SafeCast<float>(selected_entity->GetHeight(orientation))},
                               kCursorOverlayLayer_};

        element.spriteSet = set;

        overlay_layer.push_back(element);
        save_overlay_info();
    }
    else {
        if (world.GetTile(coord, TileLayer::entity)->GetPrototype() != nullptr ||
            world.GetTile(coord, TileLayer::resource)->GetPrototype() != nullptr) {

            // Is hovering over entity
            overlay_layer.push_back({cursor_sprite,
                                     {World::WorldCToOverlayC(coord.x), World::WorldCToOverlayC(coord.y)},
                                     {1, 1},
                                     kCursorOverlayLayer_});
            save_overlay_info();
        }
    }
}

void game::MouseSelection::SkipErasingLastOverlay() noexcept {
    lastOverlayElementIndex_ = UINT64_MAX;
}
