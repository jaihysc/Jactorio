// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/inserter.h"

#include "game/world/world.h"
#include "proto/sprite.h"
#include "render/proto_renderer.h"

using namespace jactorio;

void proto::Inserter::OnRDrawUniqueData(render::RendererLayer& layer,
                                        const SpriteTexCoords& uv_coords,
                                        const Position2<float>& pixel_offset,
                                        const UniqueDataBase* unique_data) const {
    DrawInserterArm(layer, uv_coords, pixel_offset, *this, *SafeCast<const InserterData*>(unique_data));
}

SpriteSetT proto::Inserter::OnRGetSpriteSet(const Orientation orientation,
                                            game::World& /*world*/,
                                            const WorldCoord& /*coord*/) const {
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

void proto::Inserter::OnBuild(game::World& world,
                              game::Logic& /*logic*/,
                              const WorldCoord& coord,
                              const game::TileLayer tlayer,
                              Orientation orientation) const {
    auto& inserter_data = world.GetTile(coord, tlayer)->MakeUniqueData<InserterData>(orientation);
    inserter_data.set   = OnRGetSpriteSet(orientation, world, coord);

    InitPickupDropoff(world, coord, orientation);
}

void proto::Inserter::OnTileUpdate(game::World& world,
                                   const WorldCoord& emit_coord,
                                   const WorldCoord& receive_coord,
                                   UpdateType /*type*/) const {
    auto& inserter_data = *world.GetTile(receive_coord, game::TileLayer::entity)->GetUniqueData<InserterData>();
    auto* target_data   = world.GetTile(emit_coord, game::TileLayer::entity)->GetUniqueData();

    //

    const auto pickup_coords  = GetPickupCoord(receive_coord, inserter_data.orientation);
    const auto dropoff_coords = GetDropoffCoord(receive_coord, inserter_data.orientation);

    // Neighbor was removed, Uninitialize removed item handler
    if (target_data == nullptr) {
        if (emit_coord == pickup_coords) {
            inserter_data.pickup.Uninitialize();
        }
        else if (emit_coord == dropoff_coords) {
            inserter_data.dropoff.Uninitialize();
        }

        world.LogicRemove(game::LogicGroup::inserter, receive_coord, game::TileLayer::entity);
        return;
    }


    if (emit_coord == pickup_coords) {
        inserter_data.pickup.Initialize(world, emit_coord);
    }
    else if (emit_coord == dropoff_coords) {
        inserter_data.dropoff.Initialize(world, emit_coord);
    }


    // Add to logic updates if initialized, remove if not
    if (inserter_data.pickup.IsInitialized() && inserter_data.dropoff.IsInitialized()) {
        world.LogicRegister(game::LogicGroup::inserter, receive_coord, game::TileLayer::entity);
    }
}

void proto::Inserter::OnRemove(game::World& world,
                               game::Logic& /*logic*/,
                               const WorldCoord& coord,
                               const game::TileLayer tlayer) const {
    world.LogicRemove(game::LogicGroup::inserter, coord, game::TileLayer::entity);

    const auto* inserter_data = world.GetTile(coord, tlayer)->GetUniqueData<InserterData>();

    world.updateDispatcher.Unregister({coord, GetDropoffCoord(coord, inserter_data->orientation)});
    world.updateDispatcher.Unregister({coord, GetPickupCoord(coord, inserter_data->orientation)});
}

void proto::Inserter::OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const {
    auto* inserter_data = tile.GetUniqueData<InserterData>();
    assert(inserter_data != nullptr);

    InitPickupDropoff(world, coord, inserter_data->orientation);
}

void proto::Inserter::PostLoadValidate(const data::PrototypeManager& /*proto*/) const {
    J_PROTO_ASSERT(tileReach != 0, "Invalid tileReach, > 0");
    J_PROTO_ASSERT(armSprite != nullptr, "Arm sprite not provided");
    J_PROTO_ASSERT(handSprite != nullptr, "Hand sprite not provided");
}

void proto::Inserter::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}

// ======================================================================

WorldCoord proto::Inserter::GetDropoffCoord(const WorldCoord& coord, const Orientation orientation) const {
    return coord.Incremented(orientation, this->tileReach);
}

WorldCoord proto::Inserter::GetPickupCoord(const WorldCoord& coord, const Orientation orientation) const {
    return coord.Incremented(orientation, this->tileReach * -1);
}

void proto::Inserter::InitPickupDropoff(game::World& world,
                                        const WorldCoord& coord,
                                        const Orientation orientation) const {
    // Dropoff side
    {
        auto emit_coords = GetDropoffCoord(coord, orientation);
        world.updateDispatcher.Register(coord, emit_coords, *this);
        world.UpdateDispatch(emit_coords, UpdateType::place);
    }
    // Pickup side
    {
        auto emit_coords = GetPickupCoord(coord, orientation);
        world.updateDispatcher.Register(coord, emit_coords, *this);
        world.UpdateDispatch(emit_coords, UpdateType::place);
    }
}
