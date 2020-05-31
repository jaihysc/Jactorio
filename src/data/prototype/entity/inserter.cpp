// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#include "data/prototype/entity/inserter.h"

using namespace jactorio;

void data::Inserter::OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const {
}

std::pair<uint16_t, uint16_t> data::Inserter::MapPlacementOrientation(Orientation orientation,
                                                                      game::WorldData& world_data,
                                                                      const game::WorldData::WorldPair& world_coords) const {
	return {0, 0};
}

void data::Inserter::OnDeferTimeElapsed(game::DeferralTimer& timer, UniqueDataBase* unique_data) const {
	auto* inserter_data = static_cast<InserterData*>(unique_data);
	inserter_data->dropoff.DropOff(inserter_data->heldItem);
}

void data::Inserter::OnBuild(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
                             game::ChunkTileLayer& tile_layer, Orientation orientation) const {
	tile_layer.MakeUniqueData<InserterData>(orientation);
	world_data.LogicRegister(game::Chunk::LogicGroup::inserter, world_coords, game::ChunkTile::ChunkLayer::entity);

    // Dropoff side
	{
	    auto emit_coords = world_coords;
		OrientationIncrement(orientation, emit_coords.first, emit_coords.second);
		
		OnNeighborUpdate(world_data, emit_coords, world_coords, orientation);
	}
	// Pickup side
	{
	    auto emit_coords = world_coords;
		OrientationIncrement(orientation, emit_coords.first, emit_coords.second, -1);
		
		OnNeighborUpdate(world_data, emit_coords, world_coords, orientation);
	}
}

void data::Inserter::OnNeighborUpdate(game::WorldData& world_data, 
                                      const game::WorldData::WorldPair& emit_world_coords,
                                      const game::WorldData::WorldPair& receive_world_coords,
                                      const Orientation) const {
    auto& inserter_layer = world_data.GetTile(receive_world_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);
    auto& inserter_data = *inserter_layer.GetUniqueData<InserterData>();

	auto& target_layer = world_data.GetTile(emit_world_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);
	auto* target_data = target_layer.GetUniqueData();

	//
	
    auto pickup_coords = receive_world_coords;
    OrientationIncrement(inserter_data.orientation, pickup_coords.first, pickup_coords.second, -1);

    auto dropoff_coords = receive_world_coords;
    OrientationIncrement(inserter_data.orientation, dropoff_coords.first, dropoff_coords.second);

	// Uninitialize removed item handler
	if (!target_data) {
		if (emit_world_coords == pickup_coords) {
	        inserter_data.pickup.Uninitialize();
		}
		else if (emit_world_coords == dropoff_coords) {
	        inserter_data.dropoff.Uninitialize();
		}
		return;
	}


	if (emit_world_coords == pickup_coords) {
        inserter_data.pickup.Initialize(world_data, *target_data, emit_world_coords);
	}
	else if (emit_world_coords == dropoff_coords) {
        inserter_data.dropoff.Initialize(world_data, *target_data, emit_world_coords);
	}
}

void data::Inserter::OnRemove(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
                              game::ChunkTileLayer& tile_layer) const {
	world_data.LogicRemove(game::Chunk::LogicGroup::inserter, world_coords, 
						   game::ChunkTile::ChunkLayer::entity);
}
