// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/mining_drill.h"

#include <tuple>

#include "game/logic/logic.h"
#include "game/world/world.h"
#include "gui/menus.h"
#include "proto/item.h"
#include "proto/resource_entity.h"
#include "proto/sprite.h"

using namespace jactorio;

bool proto::MiningDrill::OnRShowGui(const gui::Context& context, game::ChunkTile* tile) const {
    gui::MiningDrill(context, this, tile->GetUniqueData<MiningDrillData>());
    return true;
}

proto::Item* proto::MiningDrill::FindOutputItem(const game::World& world,
                                                WorldCoord coord,
                                                const Orientation orien) const {
    coord.x -= this->miningRadius;
    coord.y -= this->miningRadius;

    for (WorldCoordAxis y = 0; y < SafeCast<WorldCoordAxis>(2u * this->miningRadius + this->GetHeight(orien)); ++y) {
        for (WorldCoordAxis x = 0; x < SafeCast<WorldCoordAxis>(2u * this->miningRadius + this->GetWidth(orien)); ++x) {
            const auto* tile = world.GetTile({coord.x + x, coord.y + y}, game::TileLayer::resource);

            if (tile->GetPrototype() != nullptr)
                return tile->GetPrototype<ResourceEntity>()->GetItem();
        }
    }

    return nullptr;
}

void proto::MiningDrill::OnDeferTimeElapsed(game::World& world, game::Logic& logic, UniqueDataBase* unique_data) const {
    // Re-register callback and insert item, remove item from ground for next elapse
    auto* drill_data = SafeCast<MiningDrillData*>(unique_data);

    const bool outputted_item = drill_data->output.DropOff(logic, {drill_data->outputItem, 1});

    if (outputted_item) {
        // Output's orientation is drill's orientation
        if (DeductResource(world, drill_data->output.GetOrientation(), *drill_data)) {
            RegisterMineCallback(logic.deferralTimer, drill_data);
        }
        else {
            drill_data->deferralEntry.Invalidate();
        }
    }
    else {
        RegisterOutputCallback(logic.deferralTimer, drill_data);
    }
}


bool proto::MiningDrill::OnCanBuild(const game::World& world, const WorldCoord& coord, const Orientation orien) const {
    auto coords = coord;
    /*
     * [ ] [ ] [ ] [ ] [ ]
     * [ ] [X] [x] [x] [ ]
     * [ ] [x] [x] [x] [ ]
     * [ ] [x] [x] [x] [ ]
     * [ ] [ ] [ ] [ ] [ ]
     */
    coords.x -= this->miningRadius;
    coords.y -= this->miningRadius;

    for (WorldCoordAxis y = 0; y < SafeCast<WorldCoordAxis>(2u * this->miningRadius + this->GetHeight(orien)); ++y) {
        for (WorldCoordAxis x = 0; x < SafeCast<WorldCoordAxis>(2u * this->miningRadius + this->GetWidth(orien)); ++x) {
            const auto* tile = world.GetTile({coords.x + x, coords.y + y}, game::TileLayer::resource);

            if (tile->GetPrototype() != nullptr)
                return true;
        }
    }

    return false;
}

void proto::MiningDrill::OnBuild(game::World& world,
                                 game::Logic& logic,
                                 const WorldCoord& coord,
                                 const Orientation orientation) const {
    auto& drill_data = world.GetTile(coord, game::TileLayer::entity)->MakeUniqueData<MiningDrillData>(orientation);
    world.DisableAnimation(coord, game::TileLayer::entity);

    drill_data.resourceCoord.x = coord.x - this->miningRadius;
    drill_data.resourceCoord.y = coord.y - this->miningRadius;

    const bool success = SetupResourceDeduction(world, drill_data, orientation);
    assert(success);
    assert(drill_data.outputItem != nullptr); // Should not have been allowed to be placed on no resources

    const auto output_coords = GetOutputCoord(coord, orientation);

    drill_data.outputTile = output_coords;

    OnNeighborUpdate(world, logic, output_coords, coord, orientation);
}

void proto::MiningDrill::OnNeighborUpdate(game::World& world,
                                          game::Logic& logic,
                                          const WorldCoord& emit_coord,
                                          const WorldCoord& receive_coord,
                                          Orientation /*emit_orientation*/) const {
    auto& self_tile = *world.GetTile(receive_coord, game::TileLayer::entity);

    auto* drill_data = self_tile.GetUniqueData<MiningDrillData>();
    assert(drill_data != nullptr);

    // Ignore updates from non output tiles
    if (emit_coord != drill_data->outputTile)
        return;


    // Do not register callback to mine items if there is no valid entity to output items to
    if (InitializeOutput(world, emit_coord, drill_data)) {
        drill_data->miningTicks =
            LossyCast<uint16_t>(SafeCast<float>(kGameHertz) *
                                SafeCast<const ResourceEntity*>(drill_data->outputItem->entityPrototype)->pickupTime);

        const bool success = DeductResource(world, self_tile.GetOrientation(), *drill_data);
        if (success) {
            RegisterMineCallback(logic.deferralTimer, drill_data);
            world.EnableAnimation(receive_coord, game::TileLayer::entity);
        }
    }
    else {
        // Un-register callback if one is registered
        logic.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
        world.DisableAnimation(receive_coord, game::TileLayer::entity);
    }
}


void proto::MiningDrill::OnRemove(game::World& world, game::Logic& logic, const WorldCoord& coord) const {
    auto* drill_data = world.GetTile(coord, game::TileLayer::entity)->GetUniqueData<MiningDrillData>();
    logic.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
}

void proto::MiningDrill::OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const {
    auto* drill_data = tile.GetUniqueData<MiningDrillData>();
    assert(drill_data != nullptr);

    InitializeOutput(world, GetOutputCoord(coord, drill_data->output.GetOrientation()), drill_data);
}

void proto::MiningDrill::PostLoadValidate(const data::PrototypeManager& proto) const {
    HealthEntity::PostLoadValidate(proto);
}

// ======================================================================


bool proto::MiningDrill::InitializeOutput(game::World& world,
                                          const WorldCoord& output_coord,
                                          MiningDrillData* drill_data) {
    return drill_data->output.Initialize(world, output_coord);
}

WorldCoord proto::MiningDrill::GetOutputCoord(const WorldCoord& coord, const Orientation orientation) const {
    WorldCoord output_coords = this->resourceOutput.Get(orientation);
    output_coords.x += coord.x;
    output_coords.y += coord.y;

    return output_coords;
}


int proto::MiningDrill::GetMiningAreaX(const Orientation orien) const {
    return 2 * this->miningRadius + this->GetWidth(orien);
}

int proto::MiningDrill::GetMiningAreaY(const Orientation orien) const {
    return 2 * this->miningRadius + this->GetHeight(orien);
}

bool proto::MiningDrill::SetupResourceDeduction(const game::World& world,
                                                MiningDrillData& drill_data,
                                                const Orientation orien) const {
    const auto x_span = GetMiningAreaX(orien);
    const auto y_span = GetMiningAreaY(orien);

    for (int y = 0; y < y_span; ++y) {
        for (int x = 0; x < x_span; ++x) {
            const auto* tile = world.GetTile({drill_data.resourceCoord.x + x, drill_data.resourceCoord.y + y},
                                             game::TileLayer::resource);

            if (tile->GetPrototype() != nullptr) {
                drill_data.outputItem     = tile->GetPrototype<ResourceEntity>()->GetItem();
                drill_data.resourceOffset = SafeCast<decltype(drill_data.resourceOffset)>(y * x_span + x);
                return true;
            }
        }
    }

    return false;
}

bool proto::MiningDrill::DeductResource(game::World& world,
                                        const Orientation orien,
                                        MiningDrillData& drill_data,
                                        const ResourceEntityData::ResourceCount amount) const {
    auto get_tile_coord = [&]() {
        return WorldCoord{drill_data.resourceCoord.x + drill_data.resourceOffset % GetMiningAreaX(orien),
                          drill_data.resourceCoord.y + drill_data.resourceOffset / GetMiningAreaX(orien)};
    };
    auto get_resource_layer = [&](const WorldCoord& coord) {
        auto* tile = world.GetTile(coord, game::TileLayer::resource);
        assert(tile != nullptr);
        return std::make_tuple(tile, tile->GetUniqueData<ResourceEntityData>());
    };


    auto tile_coord                      = get_tile_coord();
    auto [resource_layer, resource_data] = get_resource_layer(tile_coord);

    if (resource_data == nullptr) {
        if (!SetupResourceDeduction(world, drill_data, orien))
            return false; // Drill has no resources left to mine

        tile_coord                              = get_tile_coord();
        std::tie(resource_layer, resource_data) = get_resource_layer(tile_coord);
    }

    assert(resource_data != nullptr);

    assert(resource_data->resourceAmount >= amount);
    resource_data->resourceAmount -= amount;


    if (resource_data->resourceAmount == 0) {
        resource_layer->Clear();
        world.SetTexCoordId(tile_coord, game::TileLayer::resource, 0);
    }

    return true;
}

void proto::MiningDrill::RegisterMineCallback(game::DeferralTimer& timer, MiningDrillData* unique_data) const {
    const auto mine_ticks = LossyCast<GameTickT>(unique_data->miningTicks / miningSpeed);
    assert(mine_ticks > 0);

    unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, mine_ticks);
}

void proto::MiningDrill::RegisterOutputCallback(game::DeferralTimer& timer, MiningDrillData* unique_data) const {
    constexpr int output_retry_ticks = 1;
    static_assert(output_retry_ticks > 0);

    unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, output_retry_ticks);
}
