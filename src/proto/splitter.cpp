// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/splitter.h"

#include "core/coordinate_tuple.h"
#include "game/logic/conveyor_utility.h"
#include "game/world/chunk_tile.h"
#include "game/world/world.h"
#include "proto/sprite.h"

using namespace jactorio;

SpriteTexCoordIndexT proto::Splitter::OnGetTexCoordId(const game::World& world,
                                                      const WorldCoord& coord,
                                                      const Orientation orientation) const {
    return FWorldObject::OnGetTexCoordId(world, coord, orientation);
}

void proto::Splitter::OnBuild(game::World& world,
                              game::Logic& /*logic*/,
                              const WorldCoord& coord,
                              const Orientation orientation) const {
    world.GetTile(coord, game::TileLayer::entity)->MakeUniqueData<SplitterData>(orientation);

    auto build_conveyor = [&world, orientation](const WorldCoord& side_coord) {
        auto [proto, con_data] = GetConveyorInfo(world, side_coord);
        assert(con_data != nullptr);

        BuildConveyor(world, side_coord, *con_data, orientation, game::LogicGroup::splitter);
    };

    build_conveyor(coord);
    build_conveyor(GetNonTopLeftCoord(world, coord, game::TileLayer::entity));
}

void proto::Splitter::OnNeighborUpdate(game::World& world,
                                       game::Logic& /*logic*/,
                                       const WorldCoord& /*emit_coord*/,
                                       const WorldCoord& receive_coord,
                                       Orientation /*emit_orientation*/) const {
    ConveyorUpdateNeighborTermination(world, receive_coord);
}

void proto::Splitter::OnRemove(game::World& world, game::Logic& /*logic*/, const WorldCoord& coord) const {

    RemoveConveyor(world, coord, game::LogicGroup::splitter);
    RemoveConveyor(world, GetNonTopLeftCoord(world, coord, game::TileLayer::entity), game::LogicGroup::splitter);
}


void proto::Splitter::PostLoad() {
    // Convert floating point speed to fixed precision decimal speed
    speed = LineDistT(speedFloat);
}

void proto::Splitter::PostLoadValidate(const data::PrototypeManager& proto) const {
    Conveyor::PostLoadValidate(proto);

    J_PROTO_ASSERT(this->GetWidth(Orientation::up) == 2, "Tile width must be 2");
    J_PROTO_ASSERT(this->GetHeight(Orientation::up) == 1, "Tile height must be 1");
}

// ======================================================================

WorldCoord proto::Splitter::GetNonTopLeftCoord(const game::World& world,
                                               const WorldCoord& coord,
                                               const game::TileLayer tlayer) {
    // Get top left coord

    const auto* tile = world.GetTile(coord, tlayer);
    assert(tile != nullptr);

    const auto tl_coord = coord.Incremented(*tile);

    // Increment to the other side depending on splitter 's orientation

    switch (tile->GetUniqueData<SplitterData>()->orientation) {
    case Orientation::up:
    case Orientation::down:
        return {tl_coord.x + 1, tl_coord.y};

    case Orientation::right:
    case Orientation::left:
        return {tl_coord.x, tl_coord.y + 1};

    default:
        assert(false);
        return {0, 0};
    }
}
