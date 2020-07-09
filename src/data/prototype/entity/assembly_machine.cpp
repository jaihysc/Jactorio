// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#include "data/prototype/entity/assembly_machine.h"

#include "renderer/gui/gui_menus.h"

using namespace jactorio;

void data::AssemblyMachineData::ChangeRecipe(game::WorldData& world_data,
                                             const AssemblyMachine& assembly_proto,
                                             const Recipe* new_recipe) {
	if (new_recipe) {
		deferralEntry = world_data.deferralTimer.RegisterFromTick(
			assembly_proto,
			this,
			new_recipe->GetCraftingTime(1.f / assembly_proto.assemblySpeed));
		ingredientInv.resize(new_recipe->ingredients.size());
		productInv.resize(1);
	}
	else {
		// Remove recipe
		world_data.deferralTimer.RemoveDeferralEntry(deferralEntry);
	}

	recipe_ = new_recipe;
}

// ======================================================================

data::Sprite::SetT data::AssemblyMachine::OnRGetSet(
	Orientation,
	game::WorldData&,
	const game::WorldData::WorldPair&) const {
	return 0;
}

data::IRenderable::SpritemapFrame data::AssemblyMachine::OnRGetSprite(
	const UniqueDataBase*, const GameTickT game_tick) const {

	return AllOfSprite(*sprite, game_tick, 1.f / 6);
}


bool data::AssemblyMachine::OnRShowGui(game::PlayerData& player_data, const DataManager& data_manager,
                                       game::ChunkTileLayer* tile_layer) const {
	renderer::AssemblyMachine(player_data, data_manager, this, tile_layer->GetUniqueData());
	return true;
}

// ======================================================================

void data::AssemblyMachine::OnDeferTimeElapsed(game::WorldData& world_data, UniqueDataBase* unique_data) const {
	LOG_MESSAGE(debug, "Ding!");
}

void data::AssemblyMachine::OnBuild(game::WorldData& world_data,
                                    const game::WorldData::WorldPair& world_coords,
                                    game::ChunkTileLayer& tile_layer,
                                    const Orientation orientation) const {
	auto* data = tile_layer.MakeUniqueData<AssemblyMachineData>();

	data->set = OnRGetSet(orientation, world_data, world_coords);
}

void data::AssemblyMachine::OnRemove(game::WorldData& world_data,
                                     const game::WorldData::WorldPair& world_coords,
                                     game::ChunkTileLayer& tile_layer) const {
	auto& machine_data = *tile_layer.GetMultiTileTopLeft()->GetUniqueData<AssemblyMachineData>();

	world_data.deferralTimer.RemoveDeferralEntry(machine_data.deferralEntry);
}
