// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/container_entity.h"

#include "data/prototype/sprite.h"
#include "game/world/world_data.h"
#include "render/gui/gui_menus.h"

void jactorio::data::ContainerEntity::OnBuild(game::WorldData& /*world_data*/,
                                              game::LogicData& /*logic_data*/,
                                              const WorldCoord& /*world_coords*/,
                                              game::ChunkTileLayer& tile_layer,
                                              Orientation /*orientation*/) const {
    tile_layer.MakeUniqueData<ContainerEntityData>(inventorySize);
}

bool jactorio::data::ContainerEntity::OnRShowGui(const render::GuiRenderer& g_rendr,
                                                 game::ChunkTileLayer* tile_layer) const {
    render::ContainerEntity({g_rendr, this, tile_layer->GetUniqueData<ContainerEntityData>()});
    return true;
}

void jactorio::data::ContainerEntity::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}
