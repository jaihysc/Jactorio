// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/inserter.h"

#include "renderer/rendering/data_renderer.h"

using namespace jactorio;

void data::Inserter::OnRDrawUniqueData(renderer::RendererLayer& layer, const SpriteUvCoordsT& uv_coords,
                                       const core::Position2<float>& pixel_offset,
                                       const UniqueDataBase* unique_data) const {
	DrawInserterArm(layer, uv_coords, pixel_offset,
	                *this, *static_cast<const InserterData*>(unique_data));
}

data::Sprite::SetT data::Inserter::OnRGetSpriteSet(const Orientation orientation,
                                                   game::WorldData&,
                                                   const WorldCoord&) const {
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

void data::Inserter::OnBuild(game::WorldData& world_data, game::LogicData&,
                             const WorldCoord& world_coords, game::ChunkTileLayer& tile_layer,
                             Orientation orientation) const {
	auto* inserter_data = tile_layer.MakeUniqueData<InserterData>(orientation);
	inserter_data->set  = OnRGetSpriteSet(orientation, world_data, world_coords);


	// Dropoff side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(orientation, emit_coords.x, emit_coords.y, this->tileReach);

		world_data.updateDispatcher.Register(world_coords, emit_coords, *this);
		world_data.UpdateDispatch(emit_coords, UpdateType::place);
	}
	// Pickup side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(orientation, emit_coords.x, emit_coords.y, this->tileReach * -1);

		world_data.updateDispatcher.Register(world_coords, emit_coords, *this);
		world_data.UpdateDispatch(emit_coords, UpdateType::place);
	}
}

void data::Inserter::OnTileUpdate(game::WorldData& world_data,
                                  const WorldCoord& emit_coords,
                                  const WorldCoord& receive_coords, UpdateType) const {
	auto& inserter_layer = world_data.GetTile(receive_coords)->GetLayer(game::TileLayer::entity);
	auto& inserter_data  = *inserter_layer.GetUniqueData<InserterData>();

	auto& target_layer = world_data.GetTile(emit_coords)->GetLayer(game::TileLayer::entity);
	auto* target_data  = target_layer.GetUniqueData();

	//

	auto pickup_coords = receive_coords;
	OrientationIncrement(inserter_data.orientation, pickup_coords.x, pickup_coords.y, this->tileReach * -1);

	auto dropoff_coords = receive_coords;
	OrientationIncrement(inserter_data.orientation, dropoff_coords.x, dropoff_coords.y, this->tileReach);

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
		                       game::TileLayer::entity);
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
		                         game::TileLayer::entity);
	}
}

void data::Inserter::OnRemove(game::WorldData& world_data, game::LogicData&,
                              const WorldCoord& world_coords, game::ChunkTileLayer& tile_layer) const {
	world_data.LogicRemove(game::Chunk::LogicGroup::inserter, world_coords,
	                       game::TileLayer::entity);

	const auto* inserter_data = tile_layer.GetUniqueData<InserterData>();

	// Dropoff side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(inserter_data->orientation,
		                     emit_coords.x, emit_coords.y, this->tileReach);

		world_data.updateDispatcher.Unregister({world_coords, emit_coords});
	}
	// Pickup side
	{
		auto emit_coords = world_coords;
		OrientationIncrement(inserter_data->orientation,
		                     emit_coords.x, emit_coords.y, this->tileReach * -1);

		world_data.updateDispatcher.Unregister({world_coords, emit_coords});
	}
}
