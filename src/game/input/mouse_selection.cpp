// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "game/player/player.h"
#include "game/world/world.h"
#include "proto/abstract/entity.h"
#include "proto/sprite.h"
#include "render/renderer.h"

using namespace jactorio;

Position2<int32_t> game::MouseSelection::cursorPos_;

void game::MouseSelection::SetCursor(const Position2<int32_t>& cursor_pos) noexcept {
    cursorPos_ = cursor_pos;
}

Position2<int32_t> game::MouseSelection::GetCursor() noexcept {
    return cursorPos_;
}


void game::MouseSelection::DrawCursorOverlay(render::Renderer& renderer,
                                             GameWorlds& worlds,
                                             Player& player,
                                             const data::PrototypeManager& proto) {
    const auto* stack         = player.inventory.GetSelectedItem();
    const auto* cursor_sprite = proto.Get<proto::Sprite>(
        player.world.MouseSelectedTileInRange() ? "__core__/cursor-select" : "__core__/cursor-invalid");
    assert(cursor_sprite != nullptr);


    const auto cursor_coord = player.world.GetMouseTileCoords();
    auto& world             = worlds[player.world.GetId()];
    const auto orientation  = player.placement.orientation;


    if (stack != nullptr) {
        auto* selected_entity = stack->item->entityPrototype;
        assert(selected_entity != nullptr);

        if (selected_entity != nullptr && selected_entity->placeable) {
            // Has item selected
            renderer.PrepareSprite(cursor_coord,
                                   selected_entity->OnGetTexCoordId(world, cursor_coord, orientation),
                                   selected_entity->GetDimension(orientation));
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
            renderer.PrepareSprite(cursor_coord, cursor_sprite->texCoordId);
        }
    }
}
