// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/mining_drill.h"

#include <tuple>

#include "game/logic/logic_data.h"
#include "game/world/world_data.h"
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
                                               game::WorldData& /*world_data*/,
                                               const WorldCoord& /*world_coords*/) const {
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

proto::Item* proto::MiningDrill::FindOutputItem(const game::WorldData& world_data, WorldCoord world_pair) const {
    world_pair.x -= this->miningRadius;
    world_pair.y -= this->miningRadius;

    for (uint32_t y = 0; y < 2u * this->miningRadius + this->tileHeight; ++y) {
        for (uint32_t x = 0; x < 2u * this->miningRadius + this->tileWidth; ++x) {
            const game::ChunkTile* tile = world_data.GetTile(world_pair.x + x, world_pair.y + y);

            const auto& resource = tile->GetLayer(game::TileLayer::resource);
            if (resource.prototypeData.Get() != nullptr)
                return static_cast<const ResourceEntity*>(resource.prototypeData.Get())->GetItem();
        }
    }

    return nullptr;
}

void proto::MiningDrill::OnDeferTimeElapsed(game::WorldData& world_data,
                                            game::LogicData& logic_data,
                                            UniqueDataBase* unique_data) const {
    // Re-register callback and insert item, remove item from ground for next elapse
    auto* drill_data = static_cast<MiningDrillData*>(unique_data);

    const bool outputted_item = drill_data->output.DropOff(logic_data, {drill_data->outputItem, 1});

    if (outputted_item) {
        if (DeductResource(world_data, *drill_data)) {
            RegisterMineCallback(logic_data.deferralTimer, drill_data);
        }
        else {
            drill_data->deferralEntry.Invalidate();
        }
    }
    else {
        RegisterOutputCallback(logic_data.deferralTimer, drill_data);
    }
}


bool proto::MiningDrill::OnCanBuild(const game::WorldData& world_data, const WorldCoord& world_coords) const {
    auto coords = world_coords;
    /*
     * [ ] [ ] [ ] [ ] [ ]
     * [ ] [X] [x] [x] [ ]
     * [ ] [x] [x] [x] [ ]
     * [ ] [x] [x] [x] [ ]
     * [ ] [ ] [ ] [ ] [ ]
     */
    coords.x -= this->miningRadius;
    coords.y -= this->miningRadius;

    for (uint32_t y = 0; y < 2u * this->miningRadius + this->tileHeight; ++y) {
        for (uint32_t x = 0; x < 2u * this->miningRadius + this->tileWidth; ++x) {
            const game::ChunkTile* tile = world_data.GetTile(coords.x + x, coords.y + y);

            if (tile->GetLayer(game::TileLayer::resource).prototypeData.Get() != nullptr)
                return true;
        }
    }

    return false;
}

void proto::MiningDrill::OnBuild(game::WorldData& world_data,
                                 game::LogicData& logic_data,
                                 const WorldCoord& world_coords,
                                 game::ChunkTileLayer& tile_layer,
                                 const Orientation orientation) const {
    auto* drill_data = tile_layer.MakeUniqueData<MiningDrillData>(orientation);
    assert(drill_data);


    drill_data->resourceCoord.x = world_coords.x - this->miningRadius;
    drill_data->resourceCoord.y = world_coords.y - this->miningRadius;

    const bool success = SetupResourceDeduction(world_data, *drill_data);
    assert(success);
    assert(drill_data->outputItem != nullptr); // Should not have been allowed to be placed on no resources

    const auto output_coords = GetOutputCoord(world_coords, orientation);

    drill_data->set        = OnRGetSpriteSet(orientation, world_data, world_coords);
    drill_data->outputTile = output_coords;

    OnNeighborUpdate(world_data, logic_data, output_coords, world_coords, orientation);
}

void proto::MiningDrill::OnNeighborUpdate(game::WorldData& world_data,
                                          game::LogicData& logic_data,
                                          const WorldCoord& emit_world_coords,
                                          const WorldCoord& receive_world_coords,
                                          Orientation /*emit_orientation*/) const {
    auto& self_layer = world_data.GetTile(receive_world_coords)->GetLayer(game::TileLayer::entity);

    auto* drill_data = self_layer.GetUniqueData<MiningDrillData>();
    assert(drill_data != nullptr);

    // Ignore updates from non output tiles
    if (emit_world_coords != drill_data->outputTile)
        return;


    // Do not register callback to mine items if there is no valid entity to output items to
    if (InitializeOutput(world_data, emit_world_coords, drill_data)) {
        drill_data->miningTicks = core::LossyCast<uint16_t>(core::SafeCast<float>(kGameHertz) *
                                                            drill_data->outputItem->entityPrototype->pickupTime);

        const bool success = DeductResource(world_data, *drill_data);
        assert(success);
        RegisterMineCallback(logic_data.deferralTimer, drill_data);
    }
    else {
        // Un-register callback if one is registered
        logic_data.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
    }
}


void proto::MiningDrill::OnRemove(game::WorldData& /*world_data*/,
                                  game::LogicData& logic_data,
                                  const WorldCoord& /*world_coords*/,
                                  game::ChunkTileLayer& tile_layer) const {
    auto* drill_data = tile_layer.GetUniqueData<MiningDrillData>();
    logic_data.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
}

void proto::MiningDrill::OnDeserialize(game::WorldData& world_data,
                                       const WorldCoord& world_coord,
                                       game::ChunkTileLayer& tile_layer) const {
    auto* drill_data = tile_layer.GetUniqueData<MiningDrillData>();
    assert(drill_data != nullptr);

    InitializeOutput(world_data, GetOutputCoord(world_coord, drill_data->output.GetOrientation()), drill_data);
}

void proto::MiningDrill::PostLoadValidate(const data::PrototypeManager& /*data_manager*/) const {
    J_DATA_ASSERT(sprite != nullptr, "North sprite not provided");
    J_DATA_ASSERT(spriteE != nullptr, "East sprite not provided");
    J_DATA_ASSERT(spriteS != nullptr, "South sprite not provided");
    J_DATA_ASSERT(spriteW != nullptr, "West sprite not provided");
}

void proto::MiningDrill::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteE->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteS->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteW->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}


// ======================================================================


bool proto::MiningDrill::InitializeOutput(game::WorldData& world_data,
                                          const WorldCoord& output_coord,
                                          MiningDrillData* drill_data) {
    return drill_data->output.Initialize(world_data, output_coord);
}

WorldCoord proto::MiningDrill::GetOutputCoord(const WorldCoord& world_coord, const Orientation orientation) const {
    WorldCoord output_coords = this->resourceOutput.Get(orientation);
    output_coords.x += world_coord.x;
    output_coords.y += world_coord.y;

    return output_coords;
}


int proto::MiningDrill::GetMiningAreaX() const {
    return 2 * this->miningRadius + this->tileWidth;
}

int proto::MiningDrill::GetMiningAreaY() const {
    return 2 * this->miningRadius + this->tileHeight;
}

bool proto::MiningDrill::SetupResourceDeduction(const game::WorldData& world_data, MiningDrillData& drill_data) const {
    const auto x_span = GetMiningAreaX();
    const auto y_span = GetMiningAreaY();

    for (int y = 0; y < y_span; ++y) {
        for (int x = 0; x < x_span; ++x) {
            const auto* tile = world_data.GetTile(drill_data.resourceCoord.x + x, drill_data.resourceCoord.y + y);

            const auto& resource_layer = tile->GetLayer(game::TileLayer::resource);

            if (resource_layer.prototypeData.Get() != nullptr) {
                drill_data.outputItem     = resource_layer.GetPrototypeData<ResourceEntity>()->GetItem();
                drill_data.resourceOffset = core::SafeCast<decltype(drill_data.resourceOffset)>(y * x_span + x);
                return true;
            }
        }
    }

    return false;
}

bool proto::MiningDrill::DeductResource(game::WorldData& world_data,
                                        MiningDrillData& drill_data,
                                        const ResourceEntityData::ResourceCount amount) const {

    auto get_resource_layer = [&]() {
        auto* resource_tile =
            world_data.GetTile(drill_data.resourceCoord.x + drill_data.resourceOffset % GetMiningAreaX(),
                               drill_data.resourceCoord.y + drill_data.resourceOffset / GetMiningAreaX());
        assert(resource_tile != nullptr);

        auto& resource_layer = resource_tile->GetLayer(game::TileLayer::resource);

        return std::make_tuple(&resource_layer, resource_layer.GetUniqueData<ResourceEntityData>());
    };


    auto [resource_layer, resource_data] = get_resource_layer();

    if (resource_data == nullptr) {
        if (!SetupResourceDeduction(world_data, drill_data))
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
