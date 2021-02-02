// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/mining_drill.h"

#include <tuple>

#include "game/logic/logic_data.h"
#include "game/world/world.h"
#include "gui/menus.h"
#include "proto/item.h"
#include "proto/resource_entity.h"
#include "proto/sprite.h"

using namespace jactorio;

bool proto::MiningDrill::OnRShowGui(const render::GuiRenderer& g_rendr, game::ChunkTileLayer* tile_layer) const {
    auto* drill_data = static_cast<MiningDrillData*>(tile_layer->GetUniqueData());

    gui::MiningDrill({g_rendr, this, drill_data});
    return true;
}

proto::Sprite* proto::MiningDrill::OnRGetSprite(const SpriteSetT set) const {
    if (set <= 7)
        return sprite;

    if (set <= 15)
        return spriteE;

    if (set <= 23)
        return spriteS;

    return spriteW;
}

SpriteSetT proto::MiningDrill::OnRGetSpriteSet(const Orientation orientation,
                                               game::World& /*world*/,
                                               const WorldCoord& /*coord*/) const {
    switch (orientation) {
    case Orientation::up:
        return 0;
    case Orientation::right:
        return 8;
    case Orientation::down:
        return 16;
    case Orientation::left:
        return 24;

    default:
        assert(false); // Missing switch case
        return 0;
    }
}

SpriteFrameT proto::MiningDrill::OnRGetSpriteFrame(const UniqueDataBase& unique_data, GameTickT game_tick) const {
    const auto& drill_data = static_cast<const MiningDrillData&>(unique_data);

    // Drill is inactive
    if (!drill_data.deferralEntry.Valid())
        game_tick = 0;

    return AllOfSpriteReversing(*sprite, game_tick);
}

// ======================================================================

proto::Item* proto::MiningDrill::FindOutputItem(const game::World& world,
                                                WorldCoord coord,
                                                const Orientation orien) const {
    coord.x -= this->miningRadius;
    coord.y -= this->miningRadius;

    for (uint32_t y = 0; y < 2u * this->miningRadius + this->GetHeight(orien); ++y) {
        for (uint32_t x = 0; x < 2u * this->miningRadius + this->GetWidth(orien); ++x) {
            const game::ChunkTile* tile = world.GetTile(coord.x + x, coord.y + y);

            const auto& resource = tile->GetLayer(game::TileLayer::resource);
            if (resource.GetPrototype() != nullptr)
                return resource.GetPrototype<ResourceEntity>()->GetItem();
        }
    }

    return nullptr;
}

void proto::MiningDrill::OnDeferTimeElapsed(game::World& world,
                                            game::LogicData& logic,
                                            UniqueDataBase* unique_data) const {
    // Re-register callback and insert item, remove item from ground for next elapse
    auto* drill_data = static_cast<MiningDrillData*>(unique_data);

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


bool proto::MiningDrill::OnCanBuild(const game::World& world,
                                    const WorldCoord& coord,
                                    const Orientation orien) const {
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

    for (uint32_t y = 0; y < 2u * this->miningRadius + this->GetHeight(orien); ++y) {
        for (uint32_t x = 0; x < 2u * this->miningRadius + this->GetWidth(orien); ++x) {
            const game::ChunkTile* tile = world.GetTile(coords.x + x, coords.y + y);

            if (tile->GetLayer(game::TileLayer::resource).GetPrototype() != nullptr)
                return true;
        }
    }

    return false;
}

void proto::MiningDrill::OnBuild(game::World& world,
                                 game::LogicData& logic,
                                 const WorldCoord& coord,
                                 game::ChunkTileLayer& tile_layer,
                                 const Orientation orientation) const {
    auto& drill_data = tile_layer.MakeUniqueData<MiningDrillData>(orientation);


    drill_data.resourceCoord.x = coord.x - this->miningRadius;
    drill_data.resourceCoord.y = coord.y - this->miningRadius;

    const bool success = SetupResourceDeduction(world, drill_data, orientation);
    assert(success);
    assert(drill_data.outputItem != nullptr); // Should not have been allowed to be placed on no resources

    const auto output_coords = GetOutputCoord(coord, orientation);

    drill_data.set        = OnRGetSpriteSet(orientation, world, coord);
    drill_data.outputTile = output_coords;

    OnNeighborUpdate(world, logic, output_coords, coord, orientation);
}

void proto::MiningDrill::OnNeighborUpdate(game::World& world,
                                          game::LogicData& logic,
                                          const WorldCoord& emit_coord,
                                          const WorldCoord& receive_coord,
                                          Orientation /*emit_orientation*/) const {
    auto& self_layer = world.GetTile(receive_coord)->GetLayer(game::TileLayer::entity);

    auto* drill_data = self_layer.GetUniqueData<MiningDrillData>();
    assert(drill_data != nullptr);

    // Ignore updates from non output tiles
    if (emit_coord != drill_data->outputTile)
        return;


    // Do not register callback to mine items if there is no valid entity to output items to
    if (InitializeOutput(world, emit_coord, drill_data)) {
        drill_data->miningTicks = core::LossyCast<uint16_t>(core::SafeCast<float>(kGameHertz) *
                                                            drill_data->outputItem->entityPrototype->pickupTime);

        const bool success = DeductResource(world, self_layer.GetOrientation(), *drill_data);
        assert(success);
        RegisterMineCallback(logic.deferralTimer, drill_data);
    }
    else {
        // Un-register callback if one is registered
        logic.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
    }
}


void proto::MiningDrill::OnRemove(game::World& /*world*/,
                                  game::LogicData& logic_data,
                                  const WorldCoord& /*coord*/,
                                  game::ChunkTileLayer& tile_layer) const {
    auto* drill_data = tile_layer.GetUniqueData<MiningDrillData>();
    logic_data.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
}

void proto::MiningDrill::OnDeserialize(game::World& world,
                                       const WorldCoord& coord,
                                       game::ChunkTileLayer& tile_layer) const {
    auto* drill_data = tile_layer.GetUniqueData<MiningDrillData>();
    assert(drill_data != nullptr);

    InitializeOutput(world, GetOutputCoord(coord, drill_data->output.GetOrientation()), drill_data);
}

void proto::MiningDrill::PostLoadValidate(const data::PrototypeManager& /*data_manager*/) const {
    J_PROTO_ASSERT(sprite != nullptr, "North sprite not provided");
    J_PROTO_ASSERT(spriteE != nullptr, "East sprite not provided");
    J_PROTO_ASSERT(spriteS != nullptr, "South sprite not provided");
    J_PROTO_ASSERT(spriteW != nullptr, "West sprite not provided");
}

void proto::MiningDrill::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteE->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteS->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteW->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
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
            const auto* tile = world.GetTile(drill_data.resourceCoord.x + x, drill_data.resourceCoord.y + y);

            const auto& resource_layer = tile->GetLayer(game::TileLayer::resource);

            if (resource_layer.GetPrototype() != nullptr) {
                drill_data.outputItem     = resource_layer.GetPrototype<ResourceEntity>()->GetItem();
                drill_data.resourceOffset = core::SafeCast<decltype(drill_data.resourceOffset)>(y * x_span + x);
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

    auto get_resource_layer = [&]() {
        auto* resource_tile =
            world.GetTile(drill_data.resourceCoord.x + drill_data.resourceOffset % GetMiningAreaX(orien),
                          drill_data.resourceCoord.y + drill_data.resourceOffset / GetMiningAreaX(orien));
        assert(resource_tile != nullptr);

        auto& resource_layer = resource_tile->GetLayer(game::TileLayer::resource);

        return std::make_tuple(&resource_layer, resource_layer.GetUniqueData<ResourceEntityData>());
    };


    auto [resource_layer, resource_data] = get_resource_layer();

    if (resource_data == nullptr) {
        if (!SetupResourceDeduction(world, drill_data, orien))
            return false; // Drill has no resources left to mine

        std::tie(resource_layer, resource_data) = get_resource_layer();
    }

    assert(resource_data != nullptr);
    assert(resource_layer != nullptr);

    assert(resource_data->resourceAmount >= amount);
    resource_data->resourceAmount -= amount;


    if (resource_data->resourceAmount == 0) {
        resource_layer->Clear();
    }

    return true;
}

void proto::MiningDrill::RegisterMineCallback(game::DeferralTimer& timer, MiningDrillData* unique_data) const {
    const auto mine_ticks = core::LossyCast<GameTickT>(unique_data->miningTicks / miningSpeed);
    assert(mine_ticks > 0);

    unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, mine_ticks);
}

void proto::MiningDrill::RegisterOutputCallback(game::DeferralTimer& timer, MiningDrillData* unique_data) const {
    constexpr int output_retry_ticks = 1;
    static_assert(output_retry_ticks > 0);

    unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, output_retry_ticks);
}
