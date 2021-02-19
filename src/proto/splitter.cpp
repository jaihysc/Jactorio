// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/splitter.h"

#include "core/coordinate_tuple.h"
#include "game/logic/conveyor_utility.h"
#include "game/world/chunk_tile.h"
#include "game/world/world.h"
#include "proto/sprite.h"

using namespace jactorio;

static constexpr game::LogicGroup kSplitterLogicGroup = game::LogicGroup::splitter;

void proto::Splitter::OnBuild(game::World& world,
                              game::Logic& /*logic*/,
                              const WorldCoord& coord,
                              const game::TileLayer tlayer,
                              const Orientation orientation) const {

    world.GetTile(coord, tlayer)->MakeUniqueData<SplitterData>(orientation);

    auto build_conveyor = [&world, orientation](const WorldCoord& side_coord) {
        auto* con_data = GetConData(world, side_coord);
        assert(con_data != nullptr);

        BuildConveyor(world, side_coord, *con_data, orientation, kSplitterLogicGroup);
    };

    build_conveyor(coord);
    build_conveyor(GetNonTopLeftCoord(world, coord));
}

void proto::Splitter::OnNeighborUpdate(game::World& world,
                                       game::Logic& /*logic*/,
                                       const WorldCoord& /*emit_coord*/,
                                       const WorldCoord& receive_coord,
                                       Orientation /*emit_orientation*/) const {
    ConveyorUpdateNeighborTermination(world, receive_coord);
}

void proto::Splitter::OnRemove(game::World& world,
                               game::Logic& /*logic*/,
                               const WorldCoord& coord,
                               game::TileLayer /*tlayer*/) const {

    RemoveConveyor(world, coord, kSplitterLogicGroup);
    RemoveConveyor(world, GetNonTopLeftCoord(world, coord), kSplitterLogicGroup);
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

void proto::Splitter::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteE->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteS->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteW->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}

// ======================================================================

WorldCoord proto::Splitter::GetNonTopLeftCoord(const game::World& world, const WorldCoord& coord) {
    // Get top left coord

    const auto* tile = world.GetTile(coord, game::TileLayer::entity);
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
