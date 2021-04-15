// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "game/player/player.h"
#include "game/world/world.h"
#include "proto/abstract/entity.h"
#include "proto/sprite.h"

using namespace jactorio;

int32_t x_position = 0;
int32_t y_position = 0;

void game::MouseSelection::SetCursor(const int32_t x_pos, const int32_t y_pos) noexcept {
    x_position = x_pos;
    y_position = y_pos;
}

int32_t game::MouseSelection::GetCursorX() noexcept {
    return x_position;
}

int32_t game::MouseSelection::GetCursorY() noexcept {
    return y_position;
}

Position2<int32_t> game::MouseSelection::GetCursor() noexcept {
    return {GetCursorX(), GetCursorY()};
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
    RemoveLastOverlay(world);

    // Draw new overlay
    auto* chunk = world.GetChunkW(coord);
    if (chunk == nullptr)
        return;

    auto& overlay_layer = chunk->GetOverlay(kCursorOverlayLayer_);


    // Saves such that can be found and removed in the future
    auto save_overlay_info = [&](const OverlayElement& element) {
        lastChunkPos_     = World::WorldCToChunkC(coord);
        lastCursorSprite_ = element.sprite;
    };


    if (selected_entity != nullptr && selected_entity->placeable) {
        // Has item selected
        const auto set = selected_entity->OnRGetSpriteSet(orientation, world, coord);

        OverlayElement element{*selected_entity->OnRGetSprite(set),
                               World::WorldCToOverlayC(coord),
                               {SafeCast<float>(selected_entity->GetWidth(orientation)),
                                SafeCast<float>(selected_entity->GetHeight(orientation))},
                               kCursorOverlayLayer_};
        element.spriteSet = set;

        overlay_layer.push_back(element);
        save_overlay_info(element);
    }
    else if (world.GetTile(coord, TileLayer::entity)->GetPrototype() != nullptr ||
             world.GetTile(coord, TileLayer::resource)->GetPrototype() != nullptr) {

        // Is hovering over entity

        const OverlayElement element{cursor_sprite, World::WorldCToOverlayC(coord), {1, 1}, kCursorOverlayLayer_};

        overlay_layer.push_back(element);
        save_overlay_info(element);
    }
}

void game::MouseSelection::RemoveLastOverlay(World& world) const {
    auto* last_chunk = world.GetChunkC(lastChunkPos_);

    if (last_chunk == nullptr) {
        return;
    }

    auto& overlay_layer = last_chunk->GetOverlay(kCursorOverlayLayer_);

    for (auto it = overlay_layer.begin(); it != overlay_layer.end(); ++it) {
        if (it->sprite == lastCursorSprite_) {
            overlay_layer.erase(it);
            return;
        }
    }
}
