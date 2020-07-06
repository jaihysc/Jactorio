// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#include "data/prototype/entity/assembly_machine.h"

jactorio::data::Sprite::SetT jactorio::data::AssemblyMachine::OnRGetSet(
	Orientation,
	game::WorldData&,
	const game::WorldData::WorldPair&) const {
	return 0;
}

jactorio::data::IRenderable::SpritemapFrame jactorio::data::AssemblyMachine::OnRGetSprite(
	const UniqueDataBase*, const GameTickT game_tick) const {

	return IRenderable::AllOfSprite(*sprite, game_tick, 1.f / 6);
}


bool jactorio::data::AssemblyMachine::OnRShowGui(game::PlayerData& player_data, const DataManager& data_manager,
                                                 game::ChunkTileLayer* tile_layer) const {
	return true;
}

void jactorio::data::AssemblyMachine::OnBuild(game::WorldData& world_data,
                                              const game::WorldData::WorldPair& world_coords,
                                              game::ChunkTileLayer& tile_layer,
                                              const Orientation orientation) const {
	auto* data = tile_layer.MakeUniqueData<AssemblyMachineData>();

	data->set = OnRGetSet(orientation, world_data, world_coords);
}

void jactorio::data::AssemblyMachine::OnRemove(game::WorldData& world_data,
                                               const game::WorldData::WorldPair& world_coords,
                                               game::ChunkTileLayer& tile_layer) const {
}
