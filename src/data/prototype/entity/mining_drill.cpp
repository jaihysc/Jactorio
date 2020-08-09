// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/entity/mining_drill.h"

#include <tuple>

#include "data/prototype_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "game/logic/item_logistics.h"
#include "renderer/gui/gui_menus.h"


bool jactorio::data::MiningDrill::OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
                                             game::ChunkTileLayer* tile_layer) const {
	auto* drill_data = static_cast<MiningDrillData*>(tile_layer->GetUniqueData());

	renderer::MiningDrill(player_data, data_manager, this, drill_data);
	return true;
}

jactorio::data::Sprite* jactorio::data::MiningDrill::OnRGetSprite(const Sprite::SetT set) const {
	if (set <= 7)
		return sprite;

	if (set <= 15)
		return spriteE;

	if (set <= 23)
		return spriteS;

	return spriteW;
}

jactorio::data::Sprite::SetT jactorio::data::MiningDrill::OnRGetSpriteSet(const Orientation orientation,
                                                                          game::WorldData&,
                                                                          const WorldCoord&) const {
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
		assert(false);  // Missing switch case
		return 0;
	}
}

jactorio::data::Sprite::FrameT jactorio::data::MiningDrill::OnRGetSpriteFrame(const UniqueDataBase& unique_data,
                                                                              GameTickT game_tick) const {
	const auto& drill_data = static_cast<const MiningDrillData&>(unique_data);

	// Drill is inactive
	if (!drill_data.deferralEntry.Valid())
		game_tick = 0;

	return AllOfSpriteReversing(*sprite, game_tick);
}

// ======================================================================

jactorio::data::Item* jactorio::data::MiningDrill::FindOutputItem(const game::WorldData& world_data,
                                                                  WorldCoord world_pair) const {
	world_pair.x -= this->miningRadius;
	world_pair.y -= this->miningRadius;

	for (int y = 0; y < 2 * this->miningRadius + this->tileHeight; ++y) {
		for (int x = 0; x < 2 * this->miningRadius + this->tileWidth; ++x) {
			const game::ChunkTile* tile =
				world_data.GetTile(world_pair.x + x, world_pair.y + y);

			game::ChunkTileLayer& resource = tile->GetLayer(game::ChunkTile::ChunkLayer::resource);
			if (resource.prototypeData != nullptr)
				return static_cast<const ResourceEntity*>(resource.prototypeData)->GetItem();
		}
	}

	return nullptr;
}

void jactorio::data::MiningDrill::OnDeferTimeElapsed(game::WorldData& world_data, game::LogicData& logic_data,
                                                     UniqueDataBase* unique_data) const {
	// Re-register callback and insert item, remove item from ground for next elapse
	auto* drill_data = static_cast<MiningDrillData*>(unique_data);

	const bool outputted_item = drill_data->outputTile.DropOff(logic_data, {drill_data->outputItem, 1});

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


bool jactorio::data::MiningDrill::OnCanBuild(const game::WorldData& world_data,
                                             const WorldCoord& world_coords) const {
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

	for (int y = 0; y < 2 * this->miningRadius + this->tileHeight; ++y) {
		for (int x = 0; x < 2 * this->miningRadius + this->tileWidth; ++x) {
			const game::ChunkTile* tile =
				world_data.GetTile(coords.x + x, coords.y + y);

			if (tile->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData != nullptr)
				return true;
		}
	}

	return false;
}

void jactorio::data::MiningDrill::OnBuild(game::WorldData& world_data,
                                          game::LogicData& logic_data,
                                          const WorldCoord& world_coords,
                                          game::ChunkTileLayer& tile_layer, const Orientation orientation) const {
	WorldCoord output_coords = this->resourceOutput.Get(orientation);
	output_coords.x += world_coords.x;
	output_coords.y += world_coords.y;

	auto* drill_data = tile_layer.MakeUniqueData<MiningDrillData>(game::ItemDropOff(orientation));
	assert(drill_data);


	drill_data->resourceCoord.x = world_coords.x - this->miningRadius;
	drill_data->resourceCoord.y = world_coords.y - this->miningRadius;

	const bool success = SetupResourceDeduction(world_data, *drill_data);
	assert(success);
	assert(drill_data->outputItem != nullptr);  // Should not have been allowed to be placed on no resources

	drill_data->set              = OnRGetSpriteSet(orientation, world_data, world_coords);
	drill_data->outputTileCoords = output_coords;

	OnNeighborUpdate(world_data, logic_data, output_coords, world_coords, orientation);
}

void jactorio::data::MiningDrill::OnNeighborUpdate(game::WorldData& world_data,
                                                   game::LogicData& logic_data,
                                                   const WorldCoord& emit_world_coords,
                                                   const WorldCoord& receive_world_coords, Orientation) const {
	auto& self_layer = world_data.GetTile(receive_world_coords)
	                             ->GetLayer(game::ChunkTile::ChunkLayer::entity).GetMultiTileTopLeft();

	auto* drill_data = static_cast<MiningDrillData*>(self_layer.GetUniqueData());
	assert(drill_data);

	// Ignore updates from non output tiles 
	if (emit_world_coords != drill_data->outputTileCoords)
		return;

	auto& output_layer = world_data.GetTile(emit_world_coords)
	                               ->GetLayer(game::ChunkTile::ChunkLayer::entity).GetMultiTileTopLeft();

	const bool initialized =
		drill_data->outputTile.Initialize(world_data,
		                                  *output_layer.GetUniqueData(), emit_world_coords);

	// Do not register callback to mine items if there is no valid entity to output items to
	if (initialized) {
		drill_data->miningTicks =
			static_cast<uint16_t>(static_cast<double>(kGameHertz) * drill_data->outputItem->entityPrototype->pickupTime);

		const bool success = DeductResource(world_data, *drill_data);
		assert(success);
		RegisterMineCallback(logic_data.deferralTimer, drill_data);
	}
	else {
		// Un-register callback if one is registered
		logic_data.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
	}
}


void jactorio::data::MiningDrill::OnRemove(game::WorldData&,
                                           game::LogicData& logic_data,
                                           const WorldCoord&, game::ChunkTileLayer& tile_layer) const {
	UniqueDataBase* drill_data;

	if (tile_layer.IsMultiTile())
		drill_data = tile_layer.GetMultiTileTopLeft().GetUniqueData();
	else
		drill_data = tile_layer.GetUniqueData();

	logic_data.deferralTimer.RemoveDeferralEntry(static_cast<MiningDrillData*>(drill_data)->deferralEntry);
}

// ======================================================================

int jactorio::data::MiningDrill::GetMiningAreaX() const {
	return 2 * this->miningRadius + this->tileWidth;
}

int jactorio::data::MiningDrill::GetMiningAreaY() const {
	return 2 * this->miningRadius + this->tileHeight;
}

bool jactorio::data::MiningDrill::SetupResourceDeduction(const game::WorldData& world_data,
                                                         MiningDrillData& drill_data) const {
	const auto x_span = GetMiningAreaX();
	const auto y_span = GetMiningAreaY();

	for (int y = 0; y < y_span; ++y) {
		for (int x = 0; x < x_span; ++x) {
			const auto* tile = world_data.GetTile(drill_data.resourceCoord.x + x,
			                                      drill_data.resourceCoord.y + y);

			auto& resource_layer = tile->GetLayer(game::ChunkTile::ChunkLayer::resource);

			if (resource_layer.prototypeData != nullptr) {
				drill_data.outputItem     = static_cast<const ResourceEntity*>(resource_layer.prototypeData)->GetItem();
				drill_data.resourceOffset = y * x_span + x;
				return true;
			}

		}
	}

	return false;
}

bool jactorio::data::MiningDrill::DeductResource(game::WorldData& world_data, MiningDrillData& drill_data,
                                                 const ResourceEntityData::ResourceCount amount) const {

	auto get_resource_layer = [this](game::WorldData& world_data, const MiningDrillData& drill_data) {
		auto* resource_tile =
			world_data.GetTile(drill_data.resourceCoord.x + drill_data.resourceOffset % GetMiningAreaX(),
			                   drill_data.resourceCoord.y + drill_data.resourceOffset / GetMiningAreaX());
		assert(resource_tile != nullptr);

		auto& resource_layer = resource_tile->GetLayer(game::ChunkTile::ChunkLayer::resource);

		return std::make_tuple(&resource_layer, resource_layer.GetUniqueData<ResourceEntityData>());
	};


	auto [resource_layer, resource_data] = get_resource_layer(world_data, drill_data);

	if (resource_data == nullptr) {
		if (!SetupResourceDeduction(world_data, drill_data))
			return false;  // Drill has no resources left to mine

		std::tie(resource_layer, resource_data) = get_resource_layer(world_data, drill_data);
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

void jactorio::data::MiningDrill::RegisterMineCallback(game::LogicData::DeferralTimer& timer,
                                                       MiningDrillData* unique_data) const {
	const auto mine_ticks = static_cast<GameTickT>(unique_data->miningTicks / miningSpeed);
	assert(mine_ticks > 0);

	unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, mine_ticks);
}

void jactorio::data::MiningDrill::RegisterOutputCallback(game::LogicData::DeferralTimer& timer,
                                                         MiningDrillData* unique_data) const {
	constexpr int output_retry_ticks = 1;
	static_assert(output_retry_ticks > 0);

	unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, output_retry_ticks);
}
