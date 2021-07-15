// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/abstract/conveyor.h"

#include <array>

#include "game/logic/conveyor_struct.h"
#include "game/logic/conveyor_utility.h"
#include "game/world/world.h"
#include "proto/sprite.h"
#include "render/proto_renderer.h"

using namespace jactorio;

static constexpr game::LogicGroup kConveyorLogicGroup = game::LogicGroup::conveyor;

SpriteTexCoordIndexT proto::Conveyor::OnGetTexCoordId(const game::World& world,
                                                      const WorldCoord& coord,
                                                      const Orientation orientation) const {
    return sprite->texCoordId + static_cast<int>(ConveyorCalcLineOrien(world, coord, orientation));
}

// Build / Remove / Neighbor update

void proto::Conveyor::OnBuild(game::World& world,
                              game::Logic& /*logic*/,
                              const WorldCoord& coord,
                              const game::TileLayer tlayer,
                              const Orientation orientation) const {

    auto& con_data = world.GetTile(coord, tlayer)->MakeUniqueData<ConveyorData>();
    BuildConveyor(world, coord, con_data, orientation, kConveyorLogicGroup);
}

void proto::Conveyor::OnNeighborUpdate(game::World& world,
                                       game::Logic& /*logic*/,
                                       const WorldCoord& /*emit_coord*/,
                                       const WorldCoord& receive_coord,
                                       Orientation /*emit_orientation*/) const {
    // Run stuff here that on_build and on_remove both calls

    auto* line_data = GetConData(world, {receive_coord.x, receive_coord.y});
    if (line_data == nullptr) // Conveyor does not exist here
        return;

    // Reset segment lane item index to 0, since the head items MAY now have somewhere to go
    line_data->structure->left.index  = 0;
    line_data->structure->right.index = 0;

    ConveyorUpdateNeighborTermination(world, receive_coord);
}

void proto::Conveyor::OnRemove(game::World& world,
                               game::Logic& /*logic*/,
                               const WorldCoord& coord,
                               game::TileLayer /*tlayer*/) const {
    RemoveConveyor(world, coord, kConveyorLogicGroup);
}

void proto::Conveyor::OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const {
    auto* origin_data = tile.GetUniqueData<ConveyorData>();
    assert(origin_data != nullptr);

    ConveyorNeighborConnect(world, coord);
}

void proto::Conveyor::PostLoad() {
    // Convert floating point speed to fixed precision decimal speed
    speed = LineDistT(speedFloat);
}

void proto::Conveyor::PostLoadValidate(const data::PrototypeManager& proto) const {
    HealthEntity::PostLoadValidate(proto);

    J_PROTO_ASSERT(speedFloat >= 0.001, "Conveyor speed below minimum 0.001");
    // Cannot exceed item_width because of limitations in the logic
    J_PROTO_ASSERT(speedFloat < 0.25, "Conveyor speed equal or above maximum of 0.25");
}
