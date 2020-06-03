// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#include "data/prototype/entity/inserter.h"

using namespace jactorio;

void data::Inserter::OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const {
}

data::Sprite::SetT data::Inserter::MapPlacementOrientation(const Orientation orientation,
                                                           game::WorldData&,
                                                           const game::WorldData::WorldPair&) const {
	switch (orientation) {

	case Orientation::up:
		return 0;
	case Orientation::right:
		return 1;
	case Orientation::down:
		return 2;
	case Orientation::left:
		return 3;

	default: ;
		assert(false);
		break;
	}

	return 0;
}

void data::Inserter::OnBuild(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
                             game::ChunkTileLayer& tile_layer, Orientation orientation) const {
	auto* inserter_data = tile_layer.MakeUniqueData<InserterData>(orientation);
	inserter_data->set = MapPlacementOrientation(orientation, world_data, world_coords);


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
	auto& inserter_data  = *inserter_layer.GetUniqueData<InserterData>();

	auto& target_layer = world_data.GetTile(emit_world_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);
	auto* target_data  = target_layer.GetUniqueData();

	//

	auto pickup_coords = receive_world_coords;
	OrientationIncrement(inserter_data.orientation, pickup_coords.first, pickup_coords.second, -1);

	auto dropoff_coords = receive_world_coords;
	OrientationIncrement(inserter_data.orientation, dropoff_coords.first, dropoff_coords.second);

	// Neighbor was removed, Uninitialize removed item handler
	if (!target_data) {
		if (emit_world_coords == pickup_coords) {
			inserter_data.pickup.Uninitialize();
		}
		else if (emit_world_coords == dropoff_coords) {
			inserter_data.dropoff.Uninitialize();
		}

		world_data.LogicRemove(game::Chunk::LogicGroup::inserter,
		                       receive_world_coords,
		                       game::ChunkTile::ChunkLayer::entity);
		return;
	}


	if (emit_world_coords == pickup_coords) {
		inserter_data.pickup.Initialize(world_data, *target_data, emit_world_coords);
	}
	else if (emit_world_coords == dropoff_coords) {
		inserter_data.dropoff.Initialize(world_data, *target_data, emit_world_coords);
	}


	// Add to logic updates if initialized, remove if not
	if (inserter_data.pickup.IsInitialized() && inserter_data.dropoff.IsInitialized()) {
		world_data.LogicRegister(game::Chunk::LogicGroup::inserter,
		                         receive_world_coords,
		                         game::ChunkTile::ChunkLayer::entity);
	}
}

void data::Inserter::OnRemove(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
                              game::ChunkTileLayer& tile_layer) const {
	world_data.LogicRemove(game::Chunk::LogicGroup::inserter, world_coords,
	                       game::ChunkTile::ChunkLayer::entity);
}
