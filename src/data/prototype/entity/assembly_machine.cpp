// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#include "data/prototype/entity/assembly_machine.h"

jactorio::data::Sprite::SetT jactorio::data::AssemblyMachine::MapPlacementOrientation(
	Orientation orientation,
	game::WorldData& world_data,
	const game::WorldData::WorldPair& world_coords) const {
	return 0;
}

void jactorio::data::AssemblyMachine::OnBuild(game::WorldData& world_data,
                                              const game::WorldData::WorldPair& world_coords,
                                              game::ChunkTileLayer& tile_layer, Orientation orientation) const {
}

void jactorio::data::AssemblyMachine::OnRemove(game::WorldData& world_data,
                                               const game::WorldData::WorldPair& world_coords,
                                               game::ChunkTileLayer& tile_layer) const {
}
