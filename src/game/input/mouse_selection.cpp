// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "game/player/player.h"
#include "game/world/world.h"
#include "proto/abstract/entity.h"
#include "proto/sprite.h"
#include "render/renderer.h"

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


void game::MouseSelection::DrawCursorOverlay(render::Renderer& renderer,
                                             GameWorlds& worlds,
                                             Player& player,
                                             const data::PrototypeManager& proto) {
    const auto* stack  = player.inventory.GetSelectedItem();
    const auto* sprite = proto.Get<proto::Sprite>(player.world.MouseSelectedTileInRange() ? "__core__/cursor-select"
                                                                                          : "__core__/cursor-invalid");
    assert(sprite != nullptr);


    const auto cursor_coord = player.world.GetMouseTileCoords();
    auto& world             = worlds[player.world.GetId()];
    const auto orientation  = player.placement.orientation;


    if (stack != nullptr) {
        auto* selected_entity = stack->item->entityPrototype;
        assert(selected_entity != nullptr);

        if (selected_entity != nullptr && selected_entity->placeable) {
            // Has item selected
            renderer.PrepareSprite(cursor_coord,
                                   *selected_entity->sprite,
                                   selected_entity->OnRGetSpriteSet(orientation, world, cursor_coord),
                                   {SafeCast<float>(selected_entity->GetWidth(orientation)),
                                    SafeCast<float>(selected_entity->GetHeight(orientation))});
        }
    }
    else {
        auto* entity_tile   = world.GetTile(cursor_coord, TileLayer::entity);
        auto* resource_tile = world.GetTile(cursor_coord, TileLayer::resource);
        if (entity_tile == nullptr) { // If entity is valid, resource guaranteed valid
            return;
        }

        if (entity_tile->GetPrototype() != nullptr || resource_tile->GetPrototype() != nullptr) {
            // No item selected, over entity
            renderer.PrepareSprite(cursor_coord, *sprite);
        }
    }
}
