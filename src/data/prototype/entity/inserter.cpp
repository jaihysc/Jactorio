// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/18/2020

#include "data/prototype/entity/inserter.h"

using namespace jactorio;

data::Sprite::SetT data::Inserter::OnRGetSet(const Orientation orientation,
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
	inserter_data->set  = OnRGetSet(orientation, world_data, world_coords);


	// Dropoff side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(orientation, emit_coords.first, emit_coords.second, this->tileReach);

		world_data.updateDispatcher.Register(world_coords, emit_coords, *this);
		world_data.updateDispatcher.Dispatch(emit_coords, UpdateType::place);
	}
	// Pickup side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(orientation, emit_coords.first, emit_coords.second, this->tileReach * -1);

		world_data.updateDispatcher.Register(world_coords, emit_coords, *this);
		world_data.updateDispatcher.Dispatch(emit_coords, UpdateType::place);
	}
}

void data::Inserter::OnTileUpdate(game::WorldData& world_data,
                                  const game::WorldData::WorldPair& emit_coords,
                                  const game::WorldData::WorldPair& receive_coords, UpdateType type) const {
	auto& inserter_layer = world_data.GetTile(receive_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);
	auto& inserter_data  = *inserter_layer.GetUniqueData<InserterData>();

	auto& target_layer = world_data.GetTile(emit_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);
	auto* target_data  = target_layer.GetUniqueData();

	//

	auto pickup_coords = receive_coords;
	OrientationIncrement(inserter_data.orientation, pickup_coords.first, pickup_coords.second, this->tileReach * -1);

	auto dropoff_coords = receive_coords;
	OrientationIncrement(inserter_data.orientation, dropoff_coords.first, dropoff_coords.second, this->tileReach);

	// Neighbor was removed, Uninitialize removed item handler
	if (!target_data) {
		if (emit_coords == pickup_coords) {
			inserter_data.pickup.Uninitialize();
		}
		else if (emit_coords == dropoff_coords) {
			inserter_data.dropoff.Uninitialize();
		}

		world_data.LogicRemove(game::Chunk::LogicGroup::inserter,
		                       receive_coords,
		                       game::ChunkTile::ChunkLayer::entity);
		return;
	}


	if (emit_coords == pickup_coords) {
		inserter_data.pickup.Initialize(world_data, *target_data, emit_coords);
	}
	else if (emit_coords == dropoff_coords) {
		inserter_data.dropoff.Initialize(world_data, *target_data, emit_coords);
	}


	// Add to logic updates if initialized, remove if not
	if (inserter_data.pickup.IsInitialized() && inserter_data.dropoff.IsInitialized()) {
		world_data.LogicRegister(game::Chunk::LogicGroup::inserter,
		                         receive_coords,
		                         game::ChunkTile::ChunkLayer::entity);
	}
}

void data::Inserter::OnRemove(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
                              game::ChunkTileLayer& tile_layer) const {
	world_data.LogicRemove(game::Chunk::LogicGroup::inserter, world_coords,
	                       game::ChunkTile::ChunkLayer::entity);

	const auto* inserter_data = tile_layer.GetUniqueData<InserterData>();

	// Dropoff side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(inserter_data->orientation,
		                     emit_coords.first, emit_coords.second, this->tileReach);

		world_data.updateDispatcher.Unregister({world_coords, emit_coords});
	}
	// Pickup side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(inserter_data->orientation,
		                     emit_coords.first, emit_coords.second, this->tileReach * -1);

		world_data.updateDispatcher.Unregister({world_coords, emit_coords});
	}
}
