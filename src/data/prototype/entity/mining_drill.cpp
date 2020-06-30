// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/04/2020

#include "data/prototype/entity/mining_drill.h"

#include <cmath>

#include "data/data_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "game/logic/item_logistics.h"
#include "renderer/gui/gui_menus.h"


bool jactorio::data::MiningDrill::OnRShowGui(game::PlayerData& player_data, const DataManager& data_manager,
											 game::ChunkTileLayer* tile_layer) const {
	auto* drill_data = static_cast<MiningDrillData*>(tile_layer->GetUniqueData());

	renderer::MiningDrill(player_data, data_manager, drill_data);
	return true;
}

jactorio::data::Sprite::FrameT GetMiningDrillFrame(const jactorio::GameTickT game_tick, const uint16_t total_frames) {
	// Double for reversed animation, take 2 frames away such that the last and first frame is not repeated

	int32_t val = game_tick % (static_cast<uint64_t>(total_frames) * 2 - 2);
	val -= total_frames - 1;  // Negative side has all frames, positive side has 1 less without final frame
	return abs(val);
}

std::pair<jactorio::data::Sprite*, jactorio::data::Sprite::FrameT> jactorio::data::MiningDrill::OnRGetSprite(
	const UniqueDataBase* unique_data, GameTickT game_tick) const {
	const auto& drill_data = *static_cast<const MiningDrillData*>(unique_data);

	const auto set = drill_data.set;

	// Drill is inactive
	if (drill_data.deferralEntry.second == 0)
		game_tick = 0;

	if (set <= 7)
		return {this->sprite, GetMiningDrillFrame(game_tick, this->sprite->frames * this->spriteE->sets)};

	if (set <= 15)
		return {this->spriteE, GetMiningDrillFrame(game_tick, this->spriteE->frames * this->spriteE->sets)};

	if (set <= 23)
		return {this->spriteS, GetMiningDrillFrame(game_tick, this->spriteS->frames * this->spriteS->sets)};

	return {this->spriteW, GetMiningDrillFrame(game_tick, this->spriteW->frames * this->spriteW->sets)};
}

jactorio::data::Sprite::SetT jactorio::data::MiningDrill::MapPlacementOrientation(const Orientation orientation,
                                                                                  game::WorldData&,
                                                                                  const game::WorldData::WorldPair&) const {
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

// ======================================================================

void jactorio::data::MiningDrill::RegisterMineCallback(game::WorldData::DeferralTimer& timer, MiningDrillData* unique_data) const {
	unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, unique_data->miningTicks);
}

jactorio::data::Item* jactorio::data::MiningDrill::FindOutputItem(const game::WorldData& world_data,
                                                                  game::WorldData::WorldPair world_pair) const {
	world_pair.first -= this->miningRadius;
	world_pair.second -= this->miningRadius;

	for (int y = 0; y < 2 * this->miningRadius + this->tileHeight; ++y) {
		for (int x = 0; x < 2 * this->miningRadius + this->tileWidth; ++x) {
			const game::ChunkTile* tile =
				world_data.GetTile(world_pair.first + x, world_pair.second + y);

			game::ChunkTileLayer& resource = tile->GetLayer(game::ChunkTile::ChunkLayer::resource);
			if (resource.prototypeData != nullptr)
				return static_cast<const ResourceEntity*>(resource.prototypeData)->GetItem();
		}
	}

	return nullptr;
}

void jactorio::data::MiningDrill::OnDeferTimeElapsed(game::WorldData& world_data, UniqueDataBase* unique_data) const {
	// Re-register callback and insert item
	auto* drill_data = static_cast<MiningDrillData*>(unique_data);

	drill_data->outputTile.DropOff({drill_data->outputItem, 1});
	RegisterMineCallback(world_data.deferralTimer, drill_data);
}


bool jactorio::data::MiningDrill::OnCanBuild(const game::WorldData& world_data,
                                             const game::WorldData::WorldPair& world_coords) const {
	auto coords = world_coords;
	/*
	 * [ ] [ ] [ ] [ ] [ ]
	 * [ ] [X] [x] [x] [ ]
	 * [ ] [x] [x] [x] [ ]
	 * [ ] [x] [x] [x] [ ]
	 * [ ] [ ] [ ] [ ] [ ]
	 */
	coords.first -= this->miningRadius;
	coords.second -= this->miningRadius;

	for (int y = 0; y < 2 * this->miningRadius + this->tileHeight; ++y) {
		for (int x = 0; x < 2 * this->miningRadius + this->tileWidth; ++x) {
			const game::ChunkTile* tile =
				world_data.GetTile(coords.first + x, coords.second + y);

			if (tile->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData != nullptr)
				return true;
		}
	}

	return false;
}

void jactorio::data::MiningDrill::OnBuild(game::WorldData& world_data,
                                          const game::WorldData::WorldPair& world_coords,
                                          game::ChunkTileLayer& tile_layer,
                                          const Orientation orientation) const {
	game::WorldData::WorldPair output_coords = this->resourceOutput.Get(orientation);
	output_coords.first += world_coords.first;
	output_coords.second += world_coords.second;

	auto* drill_data = tile_layer.MakeUniqueData<MiningDrillData>(game::ItemDropOff(orientation));

	drill_data->outputItem = FindOutputItem(world_data, world_coords);
	assert(drill_data->outputItem != nullptr);  // Should not have been allowed to be placed on no resources

	drill_data->set              = MapPlacementOrientation(orientation, world_data, world_coords);
	drill_data->outputTileCoords = output_coords;

	OnNeighborUpdate(world_data, output_coords, world_coords, orientation);
}

void jactorio::data::MiningDrill::OnNeighborUpdate(game::WorldData& world_data,
                                                   const game::WorldData::WorldPair& emit_world_coords,
                                                   const game::WorldData::WorldPair& receive_world_coords,
                                                   Orientation) const {
	MiningDrillData* drill_data;
	{
		auto& self_layer = world_data.GetTile(receive_world_coords.first,
		                                      receive_world_coords.second)
		                             ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		// Use the top left tile
		if (self_layer.IsMultiTile())
			drill_data = static_cast<MiningDrillData*>(self_layer.GetMultiTileTopLeft()->GetUniqueData());
		else
			drill_data = static_cast<MiningDrillData*>(self_layer.GetUniqueData());
	}

	// Ignore updates from non output tiles 
	if (emit_world_coords != drill_data->outputTileCoords)
		return;

	auto& output_layer = world_data.GetTile(emit_world_coords)
	                               ->GetLayer(game::ChunkTile::ChunkLayer::entity);

	const bool initialized =
		drill_data->outputTile.Initialize(world_data,
		                                  *output_layer.GetUniqueData(), emit_world_coords);

	// Do not register callback to mine items if there is no valid entity to output items to
	if (initialized) {
		drill_data->miningTicks =
			static_cast<uint16_t>(static_cast<double>(JC_GAME_HERTZ) * drill_data->outputItem->entityPrototype->pickupTime);

		RegisterMineCallback(world_data.deferralTimer, drill_data);
	}
	else {
		// Un-register callback if one is registered
		world_data.deferralTimer.RemoveDeferralEntry(drill_data->deferralEntry);
	}
}


void jactorio::data::MiningDrill::OnRemove(game::WorldData& world_data,
                                           const game::WorldData::WorldPair&,
                                           game::ChunkTileLayer& tile_layer) const {
	UniqueDataBase* drill_data;

	if (tile_layer.IsMultiTile())
		drill_data = tile_layer.GetMultiTileTopLeft()->GetUniqueData();
	else
		drill_data = tile_layer.GetUniqueData();

	world_data.deferralTimer.RemoveDeferralEntry(static_cast<MiningDrillData*>(drill_data)->deferralEntry);
}
