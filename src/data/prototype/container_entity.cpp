// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/container_entity.h"

#include "game/world/world_data.h"
#include "renderer/gui/gui_menus.h"

void jactorio::data::ContainerEntity::OnBuild(
    game::WorldData&, game::LogicData&, const WorldCoord&, game::ChunkTileLayer& tile_layer, Orientation) const {
    tile_layer.MakeUniqueData<ContainerEntityData>(inventorySize);
}

bool jactorio::data::ContainerEntity::OnRShowGui(GameWorlds& worlds,
                                                 game::LogicData& logic,
                                                 game::PlayerData& player,
                                                 const PrototypeManager& data_manager,
                                                 game::ChunkTileLayer* tile_layer) const {
    renderer::ContainerEntity(
        {worlds, logic, player, data_manager, this, tile_layer->GetUniqueData<ContainerEntityData>()});
    return true;
}
