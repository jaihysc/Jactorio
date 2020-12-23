// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/abstract/conveyor.h"

#include <array>
#include <cmath>

#include "game/logic/conveyor_struct.h"
#include "game/logic/conveyor_utility.h"
#include "game/world/world_data.h"
#include "proto/sprite.h"
#include "render/proto_renderer.h"

using namespace jactorio;

proto::Orientation proto::ConveyorData::ToOrientation(const LineOrientation line_orientation) {
    switch (line_orientation) {
    case LineOrientation::up:
    case LineOrientation::right_up:
    case LineOrientation::left_up:
        return Orientation::up;

    case LineOrientation::right:
    case LineOrientation::up_right:
    case LineOrientation::down_right:
        return Orientation::right;

    case LineOrientation::down:
    case LineOrientation::right_down:
    case LineOrientation::left_down:
        return Orientation::down;

    case LineOrientation::left:
    case LineOrientation::up_left:
    case LineOrientation::down_left:
        return Orientation::left;
    }

    assert(false); // Missing switch case
    return Orientation::up;
}

// ======================================================================
// Game events


void proto::Conveyor::OnRDrawUniqueData(render::RendererLayer& layer,
                                        const SpriteUvCoordsT& uv_coords,
                                        const core::Position2<float>& pixel_offset,
                                        const UniqueDataBase* unique_data) const {
    const auto& line_data = *static_cast<const ConveyorData*>(unique_data);

    // Only draw for the head of segments
    if (line_data.structure->terminationType == game::ConveyorStruct::TerminationType::straight &&
        line_data.structIndex != 0)
        return;

    if (line_data.structure->terminationType != game::ConveyorStruct::TerminationType::straight &&
        line_data.structIndex != 1)
        return;

    DrawConveyorSegmentItems(layer, uv_coords, pixel_offset, *line_data.structure);
}

SpriteSetT proto::Conveyor::OnRGetSpriteSet(const Orientation orientation,
                                            game::WorldData& world_data,
                                            const WorldCoord& world_coords) const {
    return static_cast<uint16_t>(ConveyorCalcLineOrien(world_data, world_coords, orientation));
}

SpriteFrameT proto::Conveyor::OnRGetSpriteFrame(const UniqueDataBase& /*unique_data*/,
                                                const GameTickT game_tick) const {
    return AllOfSet(*sprite, game_tick);
}


// ======================================================================
// Build / Remove / Neighbor update

void proto::Conveyor::OnBuild(game::WorldData& world,
                              game::LogicData& /*logic*/,
                              const WorldCoord& coord,
                              game::ChunkTileLayer& tile_layer,
                              const Orientation orientation) const {

    auto& con_data = tile_layer.MakeUniqueData<ConveyorData>();
    ConveyorCreate(world, coord, con_data, orientation);

    const auto line_orientation = ConveyorCalcLineOrien(world, coord, orientation);

    con_data.set    = static_cast<uint16_t>(line_orientation);
    con_data.lOrien = line_orientation;


    ConveyorUpdateNeighborLineOrien(world, coord);
    ConveyorUpdateNeighborTermination(world, coord);

    ConveyorConnect(world, coord);
}

void proto::Conveyor::OnNeighborUpdate(game::WorldData& world,
                                       game::LogicData& /*logic*/,
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

void proto::Conveyor::OnRemove(game::WorldData& world,
                               game::LogicData& /*logic*/,
                               const WorldCoord& coord,
                               game::ChunkTileLayer& /*tile_layer*/) const {
    ConveyorDisconnect(world, coord);
    ConveyorRemove(world, coord);
    ConveyorUpdateNeighborLineOrien(world, coord);
}

void proto::Conveyor::OnDeserialize(game::WorldData& world_data,
                                    const WorldCoord& world_coord,
                                    game::ChunkTileLayer& tile_layer) const {
    auto* origin_data = tile_layer.GetUniqueData<ConveyorData>();
    assert(origin_data != nullptr);

    ConveyorConnect(world_data, world_coord);
}

void proto::Conveyor::PostLoad() {
    // Convert floating point speed to fixed precision decimal speed
    speed = LineDistT(speedFloat);
}

void proto::Conveyor::PostLoadValidate(const data::PrototypeManager& /*proto_manager*/) const {
    J_DATA_ASSERT(speedFloat >= 0.001, "Conveyor speed below minimum 0.001");
    // Cannot exceed item_width because of limitations in the logic
    J_DATA_ASSERT(speedFloat < 0.25, "Conveyor speed equal or above maximum of 0.25");
}

void proto::Conveyor::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}
