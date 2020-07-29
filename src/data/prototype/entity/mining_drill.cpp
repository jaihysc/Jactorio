// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/entity/mining_drill.h"

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
	if (drill_data.deferralEntry.second == 0)
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

void jactorio::data::MiningDrill::OnDeferTimeElapsed(game::WorldData&,
                                                     game::LogicData& logic_data, UniqueDataBase* unique_data) const {
	// Re-register callback and insert item
	auto* drill_data = static_cast<MiningDrillData*>(unique_data);

	drill_data->outputTile.DropOff(logic_data, {drill_data->outputItem, 1});
	RegisterMineCallback(logic_data.deferralTimer, drill_data);
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
                                          jactorio::game::LogicData& logic_data,
                                          const WorldCoord& world_coords,
                                          game::ChunkTileLayer& tile_layer, const Orientation orientation) const {
	WorldCoord output_coords = this->resourceOutput.Get(orientation);
	output_coords.x += world_coords.x;
	output_coords.y += world_coords.y;

	auto* drill_data = tile_layer.MakeUniqueData<MiningDrillData>(game::ItemDropOff(orientation));

	drill_data->outputItem = FindOutputItem(world_data, world_coords);
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

void jactorio::data::MiningDrill::RegisterMineCallback(game::LogicData::DeferralTimer& timer,
                                                       MiningDrillData* unique_data) const {
	unique_data->deferralEntry = timer.RegisterFromTick(*this, unique_data, unique_data->miningTicks);
}
