// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/container_entity.h"

#include "game/world/world.h"
#include "gui/menus.h"
#include "proto/sprite.h"

using namespace jactorio;

void proto::ContainerEntity::OnBuild(game::World& world,
                                     game::Logic& /*logic*/,
                                     const WorldCoord& coord,
                                     const game::TileLayer tlayer,
                                     Orientation /*orientation*/) const {
    world.GetTile(coord, tlayer)->MakeUniqueData<ContainerEntityData>(inventorySize);
}

bool proto::ContainerEntity::OnRShowGui(const gui::Context& context, game::ChunkTile* tile) const {
    gui::ContainerEntity(context, this, tile->GetUniqueData<ContainerEntityData>());
    return true;
}

void proto::ContainerEntity::PostLoadValidate(const data::PrototypeManager& proto) const {
    HealthEntity::PostLoadValidate(proto);
}
