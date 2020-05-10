// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/04/2020

#include "data/prototype/entity/mining_drill.h"

#include "data/data_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "game/logic/item_logistics.h"
#include "renderer/gui/gui_menus.h"


void jactorio::data::MiningDrill::OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const {
	auto* drill_data = static_cast<MiningDrillData*>(tile_layer->uniqueData);

	renderer::MiningDrill(player_data, drill_data);
}

std::pair<jactorio::data::Sprite*, jactorio::data::RenderableData::frame_t> jactorio::data::MiningDrill::OnRGetSprite(
	UniqueDataBase* unique_data, const GameTickT game_tick) const {
	const auto set = static_cast<RenderableData*>(unique_data)->set;

	if (set <= 7)
		return {this->sprite, game_tick % this->sprite->frames * this->sprite->sets};

	if (set <= 15)
		return {this->spriteE, game_tick % this->spriteE->frames * this->spriteE->sets};

	if (set <= 23)
		return {this->spriteS, game_tick % this->spriteS->frames * this->spriteS->sets};

	return {this->spriteW, game_tick % this->spriteW->frames * this->spriteW->sets};
}

std::pair<uint16_t, uint16_t> jactorio::data::MiningDrill::MapPlacementOrientation(const Orientation orientation,
                                                                                   game::WorldData&,
                                                                                   const game::WorldData::WorldPair&) const {
	switch (orientation) {
	case Orientation::up:
		return {0, 0};
	case Orientation::right:
		return {8, 0};
	case Orientation::down:
		return {16, 0};
	case Orientation::left:
		return {24, 0};

	default:
		assert(false);  // Missing switch case
		return {0, 0};
	}
}

// ======================================================================

void jactorio::data::MiningDrill::RegisterMineCallback(game::DeferralTimer& timer, MiningDrillData* unique_data) const {
	unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, unique_data->miningTicks);
}

void jactorio::data::MiningDrill::RemoveMineCallback(game::DeferralTimer& timer,
                                                     game::DeferralTimer::DeferralEntry& entry) {
	if (entry.second == 0)
		return;

	timer.RemoveDeferral(entry);
	entry.second = 0;
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

void jactorio::data::MiningDrill::OnDeferTimeElapsed(game::DeferralTimer& timer, UniqueDataBase* unique_data) const {
	// Re-register callback and insert item
	auto* drill_data = static_cast<MiningDrillData*>(unique_data);

	drill_data->outputTile->Insert({drill_data->outputItem, 1});
	RegisterMineCallback(timer, drill_data);
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
	tile_layer.uniqueData = new MiningDrillData();
	auto* drill_data      = static_cast<MiningDrillData*>(tile_layer.uniqueData);

	drill_data->outputItem = FindOutputItem(world_data, world_coords);
	assert(drill_data->outputItem != nullptr);  // Should not have been allowed to be placed on no resources

	drill_data->set = MapPlacementOrientation(orientation, world_data, world_coords).first;

	game::WorldData::WorldPair output_coords = this->resourceOutput.Get(orientation);
	output_coords.first += world_coords.first;
	output_coords.second += world_coords.second;

	drill_data->outputTileCoords = output_coords;

	OnNeighborUpdate(world_data, output_coords, world_coords, orientation);
}

void jactorio::data::MiningDrill::OnNeighborUpdate(game::WorldData& world_data,
                                                   const game::WorldData::WorldPair& emit_world_coords,
                                                   const game::WorldData::WorldPair& receive_world_coords,
                                                   Orientation emit_orientation) const {
	MiningDrillData* drill_data;
	{
		auto& self_layer = world_data.GetTile(receive_world_coords.first,
		                                      receive_world_coords.second)
		                             ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		// Use the top left tile
		if (self_layer.IsMultiTile())
			drill_data = static_cast<MiningDrillData*>(self_layer.GetMultiTileTopLeft()->uniqueData);
		else
			drill_data = static_cast<MiningDrillData*>(self_layer.uniqueData);
	}

	// Ignore updates from non output tiles 
	if (emit_world_coords != drill_data->outputTileCoords)
		return;

	const game::ItemInsertDestination::InsertFunc output_item_func =
		game::item_logistics::CanAcceptItem(world_data,
		                                    emit_world_coords.first,
		                                    emit_world_coords.second);

	// Do not register callback to mine items if there is no valid entity to output items to
	if (output_item_func) {

		auto& output_layer = world_data.GetTile(emit_world_coords.first,
		                                        emit_world_coords.second)
		                               ->GetLayer(game::ChunkTile::ChunkLayer::entity);

		drill_data->outputTile.emplace(*output_layer.uniqueData, output_item_func, emit_orientation);

		drill_data->miningTicks =
			static_cast<uint16_t>(static_cast<double>(JC_GAME_HERTZ) * drill_data->outputItem->entityPrototype->pickupTime);

		RegisterMineCallback(world_data.deferralTimer, drill_data);
	}
		// Un-register callback if one is registered
	else if (drill_data->deferralEntry.second != 0) {
		RemoveMineCallback(world_data.deferralTimer, drill_data->deferralEntry);
	}
}


void jactorio::data::MiningDrill::OnRemove(game::WorldData& world_data,
                                           const game::WorldData::WorldPair /*world_coords*/&,
                                           game::ChunkTileLayer& tile_layer) const {
	UniqueDataBase* drill_data;

	if (tile_layer.IsMultiTile())
		drill_data = tile_layer.GetMultiTileTopLeft()->uniqueData;
	else
		drill_data = tile_layer.uniqueData;

	RemoveMineCallback(world_data.deferralTimer, static_cast<MiningDrillData*>(drill_data)->deferralEntry);
}
