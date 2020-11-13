// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/inserter.h"

#include "game/world/world_data.h"
#include "proto/sprite.h"
#include "render/proto_renderer.h"

using namespace jactorio;

void proto::Inserter::OnRDrawUniqueData(render::RendererLayer& layer,
                                        const SpriteUvCoordsT& uv_coords,
                                        const core::Position2<float>& pixel_offset,
                                        const UniqueDataBase* unique_data) const {
    DrawInserterArm(layer, uv_coords, pixel_offset, *this, *static_cast<const InserterData*>(unique_data));
}

SpriteSetT proto::Inserter::OnRGetSpriteSet(const Orientation orientation,
                                            game::WorldData& /*world_data*/,
                                            const WorldCoord& /*world_coords*/) const {
    switch (orientation) {

    case Orientation::up:
        return 0;
    case Orientation::right:
        return 1;
    case Orientation::down:
        return 2;
    case Orientation::left:
        return 3;

    default:;
        assert(false);
        break;
    }

    return 0;
}

void proto::Inserter::OnBuild(game::WorldData& world_data,
                              game::LogicData& /*logic_data*/,
                              const WorldCoord& world_coords,
                              game::ChunkTileLayer& tile_layer,
                              Orientation orientation) const {
    auto* inserter_data = tile_layer.MakeUniqueData<InserterData>(orientation);
    inserter_data->set  = OnRGetSpriteSet(orientation, world_data, world_coords);

    InitPickupDropoff(world_data, world_coords, orientation);
}

void proto::Inserter::OnTileUpdate(game::WorldData& world_data,
                                   const WorldCoord& emit_coords,
                                   const WorldCoord& receive_coords,
                                   UpdateType /*type*/) const {
    auto& inserter_layer = world_data.GetTile(receive_coords)->GetLayer(game::TileLayer::entity);
    auto& inserter_data  = *inserter_layer.GetUniqueData<InserterData>();

    auto& target_layer = world_data.GetTile(emit_coords)->GetLayer(game::TileLayer::entity);
    auto* target_data  = target_layer.GetUniqueData();

    //

    const auto pickup_coords  = GetPickupCoord(receive_coords, inserter_data.orientation);
    const auto dropoff_coords = GetDropoffCoord(receive_coords, inserter_data.orientation);

    // Neighbor was removed, Uninitialize removed item handler
    if (target_data == nullptr) {
        if (emit_coords == pickup_coords) {
            inserter_data.pickup.Uninitialize();
        }
        else if (emit_coords == dropoff_coords) {
            inserter_data.dropoff.Uninitialize();
        }

        world_data.LogicRemove(game::Chunk::LogicGroup::inserter, receive_coords, game::TileLayer::entity);
        return;
    }


    if (emit_coords == pickup_coords) {
        inserter_data.pickup.Initialize(world_data, emit_coords);
    }
    else if (emit_coords == dropoff_coords) {
        inserter_data.dropoff.Initialize(world_data, emit_coords);
    }


    // Add to logic updates if initialized, remove if not
    if (inserter_data.pickup.IsInitialized() && inserter_data.dropoff.IsInitialized()) {
        world_data.LogicRegister(game::Chunk::LogicGroup::inserter, receive_coords, game::TileLayer::entity);
    }
}

void proto::Inserter::OnRemove(game::WorldData& world_data,
                               game::LogicData& /*logic_data*/,
                               const WorldCoord& world_coords,
                               game::ChunkTileLayer& tile_layer) const {
    world_data.LogicRemove(game::Chunk::LogicGroup::inserter, world_coords, game::TileLayer::entity);

    const auto* inserter_data = tile_layer.GetUniqueData<InserterData>();

    world_data.updateDispatcher.Unregister({world_coords, GetDropoffCoord(world_coords, inserter_data->orientation)});
    world_data.updateDispatcher.Unregister({world_coords, GetPickupCoord(world_coords, inserter_data->orientation)});
}

void proto::Inserter::OnDeserialize(game::WorldData& world_data,
                                    const WorldCoord& world_coord,
                                    game::ChunkTileLayer& tile_layer) const {
    auto* inserter_data = tile_layer.GetUniqueData<InserterData>();
    assert(inserter_data != nullptr);

    InitPickupDropoff(world_data, world_coord, inserter_data->orientation);
}

void proto::Inserter::PostLoadValidate(const data::PrototypeManager& /*proto_manager*/) const {
    J_DATA_ASSERT(tileReach != 0, "Invalid tileReach, > 0");
    J_DATA_ASSERT(armSprite != nullptr, "Arm sprite not provided");
    J_DATA_ASSERT(handSprite != nullptr, "Hand sprite not provided");
}

void proto::Inserter::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}

// ======================================================================

WorldCoord proto::Inserter::GetDropoffCoord(WorldCoord world_coord, const Orientation orientation) const {
    OrientationIncrement(orientation, world_coord.x, world_coord.y, this->tileReach);
    return world_coord;
}

WorldCoord proto::Inserter::GetPickupCoord(WorldCoord world_coord, const Orientation orientation) const {
    OrientationIncrement(orientation, world_coord.x, world_coord.y, this->tileReach * -1);
    return world_coord;
}

void proto::Inserter::InitPickupDropoff(game::WorldData& world_data,
                                        const WorldCoord& world_coord,
                                        const Orientation orientation) const {
    // Dropoff side
    {
        auto emit_coords = GetDropoffCoord(world_coord, orientation);
        world_data.updateDispatcher.Register(world_coord, emit_coords, *this);
        world_data.UpdateDispatch(emit_coords, UpdateType::place);
    }
    // Pickup side
    {
        auto emit_coords = GetPickupCoord(world_coord, orientation);
        world_data.updateDispatcher.Register(world_coord, emit_coords, *this);
        world_data.UpdateDispatch(emit_coords, UpdateType::place);
    }
}
