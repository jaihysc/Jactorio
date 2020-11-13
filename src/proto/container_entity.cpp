// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/container_entity.h"

#include "game/world/world_data.h"
#include "gui/menus.h"
#include "proto/sprite.h"

using namespace jactorio;

void proto::ContainerEntity::OnBuild(game::WorldData& /*world_data*/,
                                     game::LogicData& /*logic_data*/,
                                     const WorldCoord& /*world_coords*/,
                                     game::ChunkTileLayer& tile_layer,
                                     Orientation /*orientation*/) const {
    tile_layer.MakeUniqueData<ContainerEntityData>(inventorySize);
}

bool proto::ContainerEntity::OnRShowGui(const render::GuiRenderer& g_rendr, game::ChunkTileLayer* tile_layer) const {
    gui::ContainerEntity({g_rendr, this, tile_layer->GetUniqueData<ContainerEntityData>()});
    return true;
}

void proto::ContainerEntity::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}
