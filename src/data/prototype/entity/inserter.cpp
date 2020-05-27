// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/17/2020

#include "data/prototype/entity/inserter.h"

using namespace jactorio;

void data::Inserter::OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const {
}

std::pair<uint16_t, uint16_t> data::Inserter::MapPlacementOrientation(Orientation orientation,
                                                                      game::WorldData& world_data,
                                                                      const game::WorldData::WorldPair& world_coords) const {
	return {0, 0};
}

void data::Inserter::OnBuild(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
                             game::ChunkTileLayer& tile_layer, Orientation orientation) const {
    tile_layer.uniqueData = new InserterData();
    world_data.LogicRegister(game::Chunk::LogicGroup::inserter, world_coords, game::ChunkTile::ChunkLayer::entity);
}

void data::Inserter::OnRemove(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
                              game::ChunkTileLayer& tile_layer) const {

}
